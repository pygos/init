#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#include "init.h"

int setup_tty(void)
{
	int fd;

	fd = open("/dev/console", O_WRONLY | O_NOCTTY);
	if (fd < 0) {
		perror("/dev/console");
		return -1;
	}

	close(STDIN_FILENO);
	dup2(fd, STDOUT_FILENO);
	dup2(fd, STDERR_FILENO);

	close(fd);
	return 0;
}
