/* SPDX-License-Identifier: ISC */
#include <sys/types.h>
#include <sys/socket.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "initsock.h"


static int send_retry(int fd, const void *dst, size_t addrlen,
		      const void *buffer, size_t size)
{
	ssize_t ret;
retry:
	ret = sendto(fd, buffer, size, MSG_NOSIGNAL, dst, addrlen);
	if (ret < 0 && errno == EINTR)
		goto retry;

	if (ret < 0 || (size_t)ret < size)
		return -1;

	return 0;
}

static int send_string(int fd, const void *dst, size_t addrlen,
		       const char *str)
{
	size_t len = strlen(str);
	uint8_t raw_len[2];

	raw_len[0] = (len >> 8) & 0xFF;
	raw_len[1] = len & 0xFF;

	if (send_retry(fd, dst, addrlen, raw_len, 2))
		return -1;

	return len > 0 ? send_retry(fd, dst, addrlen, str, len) : 0;
}

int init_socket_send_status(int fd, const void *dest_addr, size_t addrlen,
			    E_SERVICE_STATE state, service_t *svc)
{
	uint8_t info[2];

	if (svc == NULL || state == ESS_NONE) {
		info[0] = ESS_NONE;
		info[1] = 0;
	} else {
		info[0] = state;
		info[1] = svc->status & 0xFF;
	}

	if (send_retry(fd, dest_addr, addrlen, info, 2))
		return -1;

	if (svc != NULL && state != ESS_NONE) {
		if (send_string(fd, dest_addr, addrlen, svc->fname))
			return -1;
		if (send_string(fd, dest_addr, addrlen, svc->name))
			return -1;
	}
	return 0;
}
