/* SPDX-License-Identifier: ISC */
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "initsock.h"

int init_socket_open(const char *tmppath)
{
	struct sockaddr_un un;
	int fd;

	fd = socket(AF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (fd < 0) {
		perror("socket");
		return -1;
	}

	memset(&un, 0, sizeof(un));
	un.sun_family = AF_UNIX;

	strcpy(un.sun_path, tmppath);

	if (bind(fd, (struct sockaddr *)&un, sizeof(un))) {
		fprintf(stderr, "bind: %s: %s", tmppath, strerror(errno));
		close(fd);
		unlink(tmppath);
		return -1;
	}

	memset(&un, 0, sizeof(un));
	un.sun_family = AF_UNIX;

	strcpy(un.sun_path, INIT_SOCK_PATH);

	if (connect(fd, (struct sockaddr *)&un, sizeof(un))) {
		perror("connect: " INIT_SOCK_PATH);
		close(fd);
		return -1;
	}

	return fd;
}
