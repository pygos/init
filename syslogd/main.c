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

#include "logfile.h"
#include "proto.h"
#include "util.h"


#define SYSLOG_SOCKET "/dev/log"
#define SYSLOG_PATH "/var/log"


static volatile sig_atomic_t syslog_run = 1;
static logfile_t *logfiles = NULL;

static const enum_map_t facilities[] = {
	{ "kernel.log", 0 },
	{ "user.log", 1 },
	{ "mail.log", 2 },
	{ "daemon.log", 3 },
	{ "auth.log", 4 },
	{ "syslog.log", 5 },
	{ "lpr.log", 6 },
	{ "news.log", 7 },
	{ "uucp.log", 8 },
	{ "clock.log", 9 },
	{ "authpriv.log", 10 },
	{ "ftp.log", 11 },
	{ "ntp.log", 12 },
	{ "audit.log", 13 },
	{ "alert.log", 14 },
	{ "cron.log", 15 },
	{ "local0.log", 16 },
	{ "local1.log", 17 },
	{ "local2.log", 18 },
	{ "local3.log", 19 },
	{ "local4.log", 20 },
	{ "local5.log", 21 },
	{ "local6.log", 22 },
	{ "local7.log", 23 },
	{ NULL, 0 },
};

static const enum_map_t levels[] = {
	{ "emergency", 0 },
	{ "alert", 1 },
	{ "critical", 2 },
	{ "error", 3 },
	{ "warning", 4 },
	{ "notice", 5 },
	{ "info", 6 },
	{ "debug", 7 },
	{ NULL, 0 },
};


static int directory_setup(void)
{
	if (mkdir(SYSLOG_PATH, 0755)) {
		if (errno != EEXIST) {
			perror("mkdir " SYSLOG_PATH);
			return -1;
		}
	}

	if (chdir(SYSLOG_PATH)) {
		perror("cd " SYSLOG_PATH);
		return -1;
	}

	return 0;
}

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

static int print_to_log(const syslog_msg_t *msg)
{
	const char *fac_name, *lvl_str;
	char timebuf[32];
	logfile_t *log;
	struct tm tm;

	fac_name = enum_to_name(facilities, msg->facility);
	if (fac_name == NULL)
		return -1;

	lvl_str = enum_to_name(levels, msg->level);
	if (lvl_str == NULL)
		return -1;

	for (log = logfiles; log != NULL; log = log->next) {
		if (!strcmp(log->name, fac_name))
			break;
	}

	if (log == NULL) {
		log = logfile_create(fac_name);
		if (log == NULL)
			return -1;
		log->next = logfiles;
		logfiles = log;
	}

	gmtime_r(&msg->timestamp, &tm);
	strftime(timebuf, sizeof(timebuf), "%FT%T", &tm);

	logfile_write(log, "[%s][%s][%s][%u] %s", timebuf, lvl_str,
		      msg->ident ? msg->ident : "", msg->pid,
		      msg->message);
	return 0;
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

	return print_to_log(&msg);
}

int main(void)
{
	int sfd, status = EXIT_FAILURE;
	logfile_t *log;

	signal_setup();

	sfd = mksock(SYSLOG_SOCKET, SOCK_FLAG_EVERYONE | SOCK_FLAG_DGRAM);
	if (sfd < 0)
		return EXIT_FAILURE;

	if (directory_setup())
		goto out;

	while (syslog_run) {
		handle_data(sfd);
	}

	status = EXIT_SUCCESS;
out:
	while (logfiles != NULL) {
		log = logfiles;
		logfiles = logfiles->next;

		logfile_destroy(log);
	}
	if (sfd > 0)
		close(sfd);
	unlink(SYSLOG_SOCKET);
	return status;
}
