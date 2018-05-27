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
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

#include "util.h"

int mksock(const char *path, int flags)
{
	struct sockaddr_un un;
	const char *errmsg;
	int fd, type;

	type = (flags & SOCK_FLAG_DGRAM) ? SOCK_DGRAM : SOCK_STREAM;

	fd = socket(AF_UNIX, type | SOCK_CLOEXEC, 0);
	if (fd < 0) {
		perror("socket");
		return -1;
	}

	memset(&un, 0, sizeof(un));
	un.sun_family = AF_UNIX;

	strcpy(un.sun_path, path);

	if (bind(fd, (struct sockaddr *)&un, sizeof(un))) {
		errmsg ="bind";
		goto fail_errno;
	}

	if (flags & SOCK_FLAG_ROOT_ONLY) {
		if (chown(path, 0, 0)) {
			errmsg = "chown";
			goto fail_errno;
		}

		if (chmod(path, 0770)) {
			errmsg = "chmod";
			goto fail_errno;
		}
	} else if (flags & SOCK_FLAG_EVERYONE) {
		if (chmod(path, 0777)) {
			errmsg = "chmod";
			goto fail_errno;
		}
	}

	if (!(flags & SOCK_FLAG_DGRAM)) {
		if (listen(fd, 10)) {
			errmsg = "listen";
			goto fail_errno;
		}
	}

	return fd;
fail_errno:
	fprintf(stderr, "%s: %s: %s\n", path, errmsg, strerror(errno));
	close(fd);
	unlink(path);
	return -1;
}
