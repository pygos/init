#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "telinit.h"

int opensock(void)
{
	struct sockaddr_un un;
	int fd;

	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("socket");
		return -1;
	}

	memset(&un, 0, sizeof(un));
	un.sun_family = AF_UNIX;

	strcpy(un.sun_path, INITSOCK);

	if (connect(fd, (struct sockaddr *)&un, sizeof(un))) {
		perror("connect: " INITSOCK);
		close(fd);
		return -1;
	}

	return fd;
}
