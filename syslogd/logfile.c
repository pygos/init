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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#include "syslogd.h"
#include "util.h"


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

static const enum_map_t facilities[] = {
	{ "kernel", 0 },
	{ "user", 1 },
	{ "mail", 2 },
	{ "daemon", 3 },
	{ "auth", 4 },
	{ "syslog", 5 },
	{ "lpr", 6 },
	{ "news", 7 },
	{ "uucp", 8 },
	{ "clock", 9 },
	{ "authpriv", 10 },
	{ "ftp", 11 },
	{ "ntp", 12 },
	{ "audit", 13 },
	{ "alert", 14 },
	{ "cron", 15 },
	{ "local0", 16 },
	{ "local1", 17 },
	{ "local2", 18 },
	{ "local3", 19 },
	{ "local4", 20 },
	{ "local5", 21 },
	{ "local6", 22 },
	{ "local7", 23 },
	{ NULL, 0 },
};


typedef struct logfile_t {
	struct logfile_t *next;
	size_t size;
	int fd;
	char filename[];
} logfile_t;


typedef struct {
	log_backend_t base;
	logfile_t *list;
	size_t maxsize;
	int flags;
} log_backend_file_t;


static int logfile_open(logfile_t *file)
{
	struct stat sb;

	file->fd = open(file->filename, O_WRONLY | O_CREAT, 0640);
	if (file->fd < 0) {
		perror(file->filename);
		return -1;
	}

	if (lseek(file->fd, 0, SEEK_END))
		goto fail;

	if (fstat(file->fd, &sb))
		goto fail;

	file->size = sb.st_size;
	return 0;
fail:
	perror(file->filename);
	close(file->fd);
	file->fd = -1;
	return -1;
}

static logfile_t *logfile_create(const char *filename)
{
	logfile_t *file = calloc(1, sizeof(*file) + strlen(filename) + 1);

	if (file == NULL) {
		perror("calloc");
		return NULL;
	}

	strcpy(file->filename, filename);

	if (logfile_open(file)) {
		free(file);
		return NULL;
	}

	return file;
}

static int logfile_write(logfile_t *file, const syslog_msg_t *msg)
{
	const char *lvl_str, *fac_name;
	char timebuf[32];
	struct tm tm;
	int ret;

	if (file->fd < 0 && logfile_open(file) != 0)
		return -1;

	lvl_str = enum_to_name(levels, msg->level);
	if (lvl_str == NULL)
		return -1;

	gmtime_r(&msg->timestamp, &tm);
	strftime(timebuf, sizeof(timebuf), "%FT%T", &tm);

	if (msg->ident != NULL) {
		fac_name = enum_to_name(facilities, msg->facility);
		if (fac_name == NULL)
			return -1;

		ret = dprintf(file->fd, "[%s][%s][%s][%u] %s", timebuf,
			      fac_name, lvl_str, msg->pid, msg->message);
	} else {
		ret = dprintf(file->fd, "[%s][%s][%u] %s", timebuf, lvl_str,
			      msg->pid, msg->message);
	}

	fsync(file->fd);

	if (ret > 0)
		file->size += ret;
	return 0;
}

static int logfile_rotate(logfile_t *f, int flags)
{
	char timebuf[32];
	char *filename;
	struct tm tm;
	time_t now;

	if (flags & LOG_ROTATE_OVERWRITE) {
		strcpy(timebuf, "1");
	} else {
		now = time(NULL);
		gmtime_r(&now, &tm);
		strftime(timebuf, sizeof(timebuf), "%FT%T", &tm);
	}

	filename = alloca(strlen(f->filename) + strlen(timebuf) + 2);
	sprintf(filename, "%s.%s", f->filename, timebuf);

	if (rename(f->filename, filename)) {
		perror(filename);
		return -1;
	}

	close(f->fd);
	logfile_open(f);
	return 0;
}

/*****************************************************************************/

static int file_backend_init(log_backend_t *backend, int flags,
			     size_t sizelimit)
{
	log_backend_file_t *log = (log_backend_file_t *)backend;

	log->flags = flags;
	log->maxsize = sizelimit;
	return 0;
}

static void file_backend_cleanup(log_backend_t *backend)
{
	log_backend_file_t *log = (log_backend_file_t *)backend;
	logfile_t *f;

	while (log->list != NULL) {
		f = log->list;
		log->list = f->next;

		close(f->fd);
		free(f);
	}
}

static int file_backend_write(log_backend_t *backend, const syslog_msg_t *msg)
{
	log_backend_file_t *log = (log_backend_file_t *)backend;
	const char *ident;
	char *filename;
	logfile_t *f;
	size_t len;

	if (msg->ident != NULL) {
		ident = msg->ident;
	} else {
		ident = enum_to_name(facilities, msg->facility);
		if (ident == NULL)
			return -1;
	}

	len = strlen(ident) + strlen(".log") + 1;
	filename = alloca(len);
	strcpy(filename, ident);
	strcat(filename, ".log");

	for (f = log->list; f != NULL; f = f->next) {
		if (strcmp(filename, f->filename) == 0)
			break;
	}

	if (f == NULL) {
		f = logfile_create(filename);
		if (f == NULL)
			return -1;
		f->next = log->list;
		log->list = f;
	}

	if (logfile_write(f, msg))
		return -1;

	if ((log->flags & LOG_ROTATE_SIZE_LIMIT) && f->size >= log->maxsize)
		logfile_rotate(f, log->flags);

	return 0;
}

static void file_backend_rotate(log_backend_t *backend)
{
	log_backend_file_t *log = (log_backend_file_t *)backend;
	logfile_t *f;

	for (f = log->list; f != NULL; f = f->next)
		logfile_rotate(f, log->flags);
}

log_backend_file_t filebackend = {
	.base = {
		.init = file_backend_init,
		.cleanup = file_backend_cleanup,
		.write = file_backend_write,
		.rotate = file_backend_rotate,
	},
	.list = NULL,
};

log_backend_t *logmgr = (log_backend_t *)&filebackend;
