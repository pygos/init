/* SPDX-License-Identifier: ISC */
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>

#include "initsock.h"

int init_socket_create(void)
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

	strcpy(un.sun_path, INIT_SOCK_PATH);
	unlink(INIT_SOCK_PATH);

	if (bind(fd, (struct sockaddr *)&un, sizeof(un))) {
		perror("bind: " INIT_SOCK_PATH);
		close(fd);
		unlink(INIT_SOCK_PATH);
		return -1;
	}

	return fd;
}
