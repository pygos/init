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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <fcntl.h>

#include "logfile.h"


logfile_t *logfile_create(const char *name)
{
	logfile_t *file;

	file = calloc(1, sizeof(*file) + strlen(name) + 1);
	if (file == NULL) {
		perror("calloc");
		return NULL;
	}

	strcpy(file->name, name);

	file->fd = open(file->name, O_WRONLY | O_CREAT, 0640);
	if (file->fd < 0)
		goto fail;

	if (lseek(file->fd, 0, SEEK_END))
		goto fail;

	return file;
fail:
	perror(file->name);
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
