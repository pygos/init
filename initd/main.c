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
#include "init.h"

static int ti_sock = -1;
static int sigfd = -1;

static void handle_signal(void)
{
	struct signalfd_siginfo info;
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

			supervisor_handle_exited(pid, status);
		}
		break;
	case SIGINT:
		/* TODO: ctrl-alt-del */
		break;
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

static void handle_tellinit(void)
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
		supervisor_set_target(TGT_SHUTDOWN);
		break;
	case TI_REBOOT:
		supervisor_set_target(TGT_REBOOT);
		break;
	}

	close(fd);
}

void target_completed(int target)
{
	switch (target) {
	case TGT_BOOT:
		if (ti_sock == -1)
			ti_sock = mksock(INITSOCK, SOCK_FLAG_ROOT_ONLY);
		break;
	default:
		break;
	}
}

int main(void)
{
	int ret, count;
	struct pollfd pfd[2];

	if (getpid() != 1) {
		fputs("init does not have pid 1, terminating!\n", stderr);
		return EXIT_FAILURE;
	}

	supervisor_init();

	sigfd = sigsetup();
	if (sigfd < 0)
		return -1;

	for (;;) {
		while (supervisor_process_queues())
			;

		memset(pfd, 0, sizeof(pfd));
		pfd[0].fd = sigfd;
		pfd[0].events = POLLIN;
		count = 1;

		if (ti_sock != -1) {
			pfd[1].fd = ti_sock;
			pfd[1].events = POLLIN;
			count = 2;
		}

		ret = poll(pfd, count, -1);

		if (ret > 0) {
			if (pfd[0].revents & POLLIN)
				handle_signal();

			if (ti_sock != -1 && pfd[1].revents & POLLIN)
				handle_tellinit();
		}
	}

	return EXIT_SUCCESS;
}
