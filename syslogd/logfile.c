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
#include <stdarg.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#include "logfile.h"


logfile_t *logfile_create(const char *ident, const char *name, int facility)
{
	int dfd = AT_FDCWD;
	logfile_t *file;
	size_t size;

	size = sizeof(*file) + 1;
	if (ident != NULL)
		size += strlen(ident);

	file = calloc(1, size);
	if (file == NULL) {
		perror("calloc");
		return NULL;
	}

	if (ident != NULL) {
		strcpy(file->ident, ident);
		if (mkdir(file->ident, 0750) != 0 && errno != EEXIST) {
			perror(file->ident);
			goto fail;
		}

		dfd = open(file->ident, O_DIRECTORY | O_RDONLY);
		if (dfd < 0) {
			perror(file->ident);
			goto fail;
		}
	}

	file->facility = facility;

	file->fd = openat(dfd, name, O_WRONLY | O_CREAT, 0640);
	if (file->fd < 0) {
		perror(name);
		goto fail;
	}

	if (lseek(file->fd, 0, SEEK_END))
		goto fail;

	if (dfd != AT_FDCWD)
		close(dfd);
	return file;
fail:
	if (dfd != AT_FDCWD)
		close(dfd);
	free(file);
	return NULL;
}

void logfile_destroy(logfile_t *file)
{
	close(file->fd);
	free(file);
}

void logfile_write(logfile_t *file, const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	vdprintf(file->fd, format, ap);
	va_end(ap);

	fsync(file->fd);
}
