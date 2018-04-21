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

#include "init.h"

static service_list_t cfg;

static int target = TGT_BOOT;		/* runlevel we are targetting */
static int runlevel = -1;		/* runlevel we are currently on */

static void handle_exited(service_t *svc)
{
	switch (svc->type) {
	case SVC_RESPAWN:
		if (target == TGT_REBOOT || target == TGT_SHUTDOWN)
			break;

		if (svc->rspwn_limit > 0) {
			svc->rspwn_limit -= 1;

			if (svc->rspwn_limit == 0) {
				print_status(svc->desc, STATUS_FAIL, false);
				break;
			}
		}

		svc->pid = runsvc(svc);
		if (svc->pid == -1) {
			print_status(svc->desc, STATUS_FAIL, false);
			break;
		}

		svclist_add(svc);
		return;
	case SVC_ONCE:
		print_status(svc->desc,
			     svc->status == EXIT_SUCCESS ?
			     STATUS_OK : STATUS_FAIL, false);
		break;
	}
	delsvc(svc);
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
		while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
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

		if (svc->type == SVC_WAIT) {
			print_status(svc->desc, STATUS_WAIT, false);

			status = runsvc_wait(svc);

			print_status(svc->desc,
				     status == EXIT_SUCCESS ?
				     STATUS_OK : STATUS_FAIL,
				     true);
			delsvc(svc);
		} else {
			svc->pid = runsvc(svc);
			if (svc->pid == -1) {
				print_status(svc->desc, STATUS_FAIL, false);
				delsvc(svc);
				continue;
			}

			if (svc->type == SVC_RESPAWN)
				print_status(svc->desc, STATUS_STARTED, false);

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
	int ti_sock = -1, sfd, ret, count;
	struct pollfd pfd[2];

	if (getpid() != 1) {
		fputs("init does not have pid 1, terminating!\n", stderr);
		return EXIT_FAILURE;
	}

	if (svcscan(SVCDIR, &cfg, RDSVC_NO_EXEC | RDSVC_NO_CTTY)) {
		fputs("Error reading service list from " SVCDIR "\n"
			"Trying to continue anyway\n", stderr);
	}

	sfd = sigsetup();
	if (sfd < 0)
		return -1;

	memset(pfd, 0, sizeof(pfd));
	pfd[0].fd = sfd;
	pfd[0].events = POLLIN;
	count = 1;

	for (;;) {
		if (!svclist_have_singleshot() && target != runlevel) {
			start_runlevel(target);
			runlevel = target;

			if (target == TGT_BOOT && ti_sock == -1) {
				ti_sock = mksock();
				if (ti_sock != -1) {
					pfd[1].fd = ti_sock;
					pfd[1].events = POLLIN;
					count = 2;
				}
			}
			continue;
		}

		ret = poll(pfd, count, -1);

		if (ret > 0) {
			if (pfd[0].revents & POLLIN)
				handle_signal(sfd);

			if (ti_sock != -1 && pfd[1].revents & POLLIN)
				handle_tellinit(ti_sock);
		}
	}

	return EXIT_SUCCESS;
}
