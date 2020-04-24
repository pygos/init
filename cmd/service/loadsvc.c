/* SPDX-License-Identifier: ISC */
#include "servicecmd.h"

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

service_t *loadsvc(const char *directory, const char *filename)
{
	service_t *svc;
	int dirfd;

	dirfd = open(directory, O_RDONLY | O_DIRECTORY);

	if (dirfd < 0) {
		perror(directory);
		return NULL;
	}

	svc = rdsvc(dirfd, filename);
	close(dirfd);
	return svc;
}
