/* SPDX-License-Identifier: ISC */
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "initsock.h"

int init_socket_send_request(int fd, E_INIT_REQUEST rq)
{
	init_request_t request;
	ssize_t ret;

	memset(&request, 0, sizeof(request));
	request.rq = rq;

retry:
	ret = write(fd, &request, sizeof(request));

	if (ret < 0) {
		if (errno == EINTR)
			goto retry;
		perror(INIT_SOCK_PATH);
		return -1;
	}

	return 0;
}
