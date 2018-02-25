#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>

#include "telinit.h"
#include "init.h"

int mksock(void)
{
	struct sockaddr_un un;
	int fd, flags;

	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("socket");
		return -1;
	}

	flags = fcntl(fd, F_GETFD);
	if (flags == -1) {
		perror("socket F_GETFD");
		goto fail;
	}

	if (fcntl(fd, F_SETFD, flags | O_CLOEXEC)) {
		perror("socket F_SETFD");
		goto fail;
	}

	memset(&un, 0, sizeof(un));
	un.sun_family = AF_UNIX;

	strcpy(un.sun_path, INITSOCK);

	if (bind(fd, (struct sockaddr *)&un, sizeof(un))) {
		perror("bind: " INITSOCK);
		goto fail;
	}

	if (chown(INITSOCK, 0, 0)) {
		perror("chown: " INITSOCK);
		goto fail;
	}

	if (chmod(INITSOCK, 0770)) {
		perror("chmod: " INITSOCK);
		goto fail;
	}

	if (listen(fd, 10)) {
		perror("listen");
		goto fail;
	}

	return fd;
fail:
	close(fd);
	unlink(INITSOCK);
	return -1;
}
