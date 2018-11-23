/* SPDX-License-Identifier: ISC */
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
