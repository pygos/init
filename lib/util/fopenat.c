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
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

#include "util.h"

FILE *fopenat(int dirfd, const char *filename, const char *mode)
{
	const char *ptr = mode;
	int fd, flags = 0;
	FILE *fp;

	switch (*(ptr++)) {
	case 'r':
		flags = O_RDONLY;
		break;
	case 'w':
		flags = O_WRONLY | O_CREAT | O_TRUNC;
		break;
	case 'a':
		flags = O_WRONLY | O_CREAT | O_APPEND;
		break;
	default:
		errno = EINVAL;
		return NULL;
	}

	if (*ptr == '+') {
		flags = (flags & ~(O_RDONLY | O_WRONLY)) | O_RDWR;
		++ptr;
	}

	if (*ptr == 'b')
		++ptr;

	if (*ptr != '\0') {
		errno = EINVAL;
		return NULL;
	}

	fd = openat(dirfd, filename, flags, 0644);
	if (fd == -1)
		return NULL;

	fp = fdopen(fd, mode);
	if (fp == NULL)
		close(fd);

	return fp;
}
