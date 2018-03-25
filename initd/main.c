/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Copyright (C) 2018 - David Oberhollenzer
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <poll.h>

#include <sys/signalfd.h>

#include "init.h"

static service_list_t cfg;

static int target = TGT_BOOT;		/* runlevel we are targetting */
static int runlevel = -1;		/* runlevel we are currently on */

static void handle_exited(service_t *svc)
{
	switch (svc->type) {
	case SVC_RESPAWN:
		if (target == TGT_REBOOT || target == TGT_SHUTDOWN) {
			delsvc(svc);
			break;
		}

		if (svc->rspwn_limit > 0) {
			svc->rspwn_limit -= 1;

			if (svc->rspwn_limit == 0) {
				print_status(svc->desc, STATUS_FAIL, false);
				delsvc(svc);
				break;
			}
		}

		svc->pid = runlst(svc->exec, svc->num_exec, svc->ctty);
		if (svc->pid == -1) {
			print_status(svc->desc, STATUS_FAIL, false);
			delsvc(svc);
		}

		svclist_add(svc);
		break;
	case SVC_ONCE:
		print_status(svc->desc,
			     svc->status == EXIT_SUCCESS ?
			     STATUS_OK : STATUS_FAIL, false);
		/* fall-through */
	default:
		delsvc(svc);
		break;
	}
}

static void handle_signal(int sigfd)
{
	struct signalfd_siginfo info;
	service_t *svc;
	int status;
	pid_t pid;

	if (read(sigfd, &info, sizeof(info)) != sizeof(info)) {
		perror("read on signal fd");
		return;
	}

	switch (info.ssi_signo) {
	case SIGCHLD:
		for (;;) {
			pid = waitpid(-1, &status, WNOHANG);
			if (pid <= 0)
				break;

			status = WIFEXITED(status) ? WEXITSTATUS(status) :
						     EXIT_FAILURE;

			svc = svclist_remove(pid);

			if (svc != NULL)
				handle_exited(svc);
		}
		break;
	case SIGINT:
		/* TODO: ctrl-alt-del */
		break;
	}
}

static void start_runlevel(int level)
{
	service_t *svc;
	int status;

	while (cfg.targets[level] != NULL) {
		svc = cfg.targets[level];
		cfg.targets[level] = svc->next;

		if (!svc->num_exec) {
			print_status(svc->desc, STATUS_OK, false);
			delsvc(svc);
			continue;
		}

		if (svc->type == SVC_WAIT) {
			print_status(svc->desc, STATUS_WAIT, false);

			status = runlst_wait(svc->exec, svc->num_exec,
					     svc->ctty);

			print_status(svc->desc,
				     status == EXIT_SUCCESS ?
				     STATUS_OK : STATUS_FAIL,
				     true);
			delsvc(svc);
		} else {
			if (svc->type == SVC_RESPAWN)
				print_status(svc->desc, STATUS_STARTED, false);

			svc->pid = runlst(svc->exec, svc->num_exec, svc->ctty);
			if (svc->pid == -1) {
				print_status(svc->desc, STATUS_FAIL, false);
				delsvc(svc);
				continue;
			}

			svclist_add(svc);
		}
	}
}

static int read_msg(int fd, ti_msg_t *msg)
{
	ssize_t ret;
retry:
	ret = read(fd, msg, sizeof(*msg));

	if (ret < 0) {
		if (errno == EINTR)
			goto retry;
		perror("read on telinit socket");
		return -1;
	}

	if ((size_t)ret < sizeof(*msg)) {
		fputs("short read on telinit socket", stderr);
		return -1;
	}

	return 0;
}

static void handle_tellinit(int ti_sock)
{
	ti_msg_t msg;
	int fd;

	fd = accept(ti_sock, NULL, NULL);
	if (fd == -1)
		return;

	if (read_msg(fd, &msg)) {
		close(fd);
		return;
	}

	switch (msg.type) {
	case TI_SHUTDOWN:
		target = TGT_SHUTDOWN;
		break;
	case TI_REBOOT:
		target = TGT_REBOOT;
		break;
	}

	close(fd);
}

int main(void)
{
	int ti_sock, sfd, ret;
	struct pollfd pfd[2];
	sigset_t mask;

	if (getpid() != 1) {
		fputs("init does not have pid 1, terminating!\n", stderr);
		return EXIT_FAILURE;
	}

	if (reboot(LINUX_REBOOT_CMD_CAD_OFF))
		perror("cannot disable CTRL+ALT+DEL");

	if (svcscan(SVCDIR, &cfg)) {
		fputs("Error reading service list from " SVCDIR "\n"
			"Trying to continue anyway\n", stderr);
	}

	sigfillset(&mask);
	if (sigprocmask(SIG_SETMASK, &mask, NULL) == -1) {
		perror("sigprocmask");
		return EXIT_FAILURE;
	}

	sfd = signalfd(-1, &mask, SFD_CLOEXEC);
	if (sfd == -1) {
		perror("signalfd");
		return EXIT_FAILURE;
	}

	ti_sock = mksock();
	if (ti_sock == -1)
		return EXIT_FAILURE;

	if (setup_tty())
		return EXIT_FAILURE;

	memset(pfd, 0, sizeof(pfd));
	pfd[0].fd = sfd;
	pfd[1].fd = ti_sock;
	pfd[0].events = pfd[1].events = POLLIN;

	for (;;) {
		if (!svclist_have_singleshot() && target != runlevel) {
			start_runlevel(target);
			runlevel = target;
			continue;
		}

		ret = poll(pfd, sizeof(pfd) / sizeof(pfd[0]), -1);

		if (ret > 0) {
			if (pfd[0].revents & POLLIN)
				handle_signal(sfd);

			if (pfd[1].revents & POLLIN)
				handle_tellinit(ti_sock);
		}
	}

	return EXIT_SUCCESS;
}
