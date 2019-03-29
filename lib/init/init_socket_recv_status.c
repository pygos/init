/* SPDX-License-Identifier: ISC */
#include <sys/types.h>
#include <sys/socket.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "initsock.h"

static int read_retry(int fd, void *buffer, size_t size)
{
	ssize_t ret;
retry:
	ret = read(fd, buffer, size);

	if (ret < 0) {
		if (errno == EINTR)
			goto retry;
		return -1;
	}

	if ((size_t)ret < size) {
		errno = EPROTO;
		return 0;
	}

	return 1;
}

static char *read_string(int fd)
{
	uint16_t len;
	char *buffer;
	int ret;

	ret = read_retry(fd, &len, sizeof(len));
	if (ret <= 0)
		return NULL;

	len = be16toh(len);

	buffer = calloc(1, len + 1);
	if (buffer == NULL)
		return NULL;

	if (len > 0) {
		ret = read_retry(fd, buffer, len);

		if (ret <= 0) {
			ret = errno;
			free(buffer);
			errno = ret;
			return NULL;
		}
	}

	return buffer;
}

int init_socket_recv_status(int fd, init_status_t *resp)
{
	init_response_status_t info;

	memset(resp, 0, sizeof(*resp));

	if (read_retry(fd, &info, sizeof(info)) <= 0)
		return -1;

	resp->state = info.state;
	resp->exit_status = info.exit_status;
	resp->id = be32toh(info.id);

	if (resp->state == ESS_NONE)
		return 0;

	resp->filename = read_string(fd);
	if (resp->filename == NULL)
		return -1;

	resp->service_name = read_string(fd);
	if (resp->service_name == NULL)
		return -1;

	return 0;
}
