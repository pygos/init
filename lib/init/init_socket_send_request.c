/* SPDX-License-Identifier: ISC */
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <endian.h>
#include <stdio.h>
#include <errno.h>

#include "initsock.h"

int init_socket_send_request(int fd, E_INIT_REQUEST rq, ...)
{
	init_request_t request;
	ssize_t ret;
	va_list ap;

	memset(&request, 0, sizeof(request));
	request.rq = rq;

	va_start(ap, rq);
	switch (rq) {
	case EIR_STATUS:
		request.arg.status.filter = va_arg(ap, E_SERVICE_STATE);
		break;
	case EIR_START:
	case EIR_STOP:
		request.arg.startstop.id = htobe32(va_arg(ap, int));
		break;
	default:
		break;
	}
	va_end(ap);

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
