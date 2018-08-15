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
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "backend.h"
#include "proto.h"
#include "util.h"


#define SYSLOG_SOCKET "/dev/log"


static volatile sig_atomic_t syslog_run = 1;


static void sighandler(int signo)
{
	switch (signo) {
	case SIGINT:
	case SIGTERM:
		syslog_run = 0;
		break;
	default:
		break;
	}
}

static void signal_setup(void)
{
	struct sigaction act;

	memset(&act, 0, sizeof(act));
	act.sa_handler = sighandler;

	sigaction(SIGINT, &act, NULL);
	sigaction(SIGTERM, &act, NULL);
}

static int handle_data(int fd)
{
	char buffer[2048];
	syslog_msg_t msg;
	ssize_t ret;

	memset(buffer, 0, sizeof(buffer));

	ret = read(fd, buffer, sizeof(buffer));
	if (ret <= 0)
		return -1;

	if (syslog_msg_parse(&msg, buffer))
		return -1;

	return logmgr->write(logmgr, &msg);
}

int main(void)
{
	int sfd, status = EXIT_FAILURE;

	signal_setup();

	sfd = mksock(SYSLOG_SOCKET, SOCK_FLAG_EVERYONE | SOCK_FLAG_DGRAM);
	if (sfd < 0)
		return EXIT_FAILURE;

	if (logmgr->init(logmgr))
		goto out;

	while (syslog_run) {
		handle_data(sfd);
	}

	status = EXIT_SUCCESS;
out:
	logmgr->cleanup(logmgr);
	if (sfd > 0)
		close(sfd);
	unlink(SYSLOG_SOCKET);
	return status;
}
