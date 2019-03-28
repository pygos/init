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
	uint8_t size_raw[2];
	char *buffer;
	size_t len;
	int ret;

	ret = read_retry(fd, size_raw, 2);
	if (ret <= 0)
		return NULL;

	len = (((size_t)size_raw[0]) << 8) | (size_t)size_raw[1];

	buffer = malloc(len + 1);
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

	buffer[len] = '\0';
	return buffer;
}

int init_socket_recv_status(int fd, init_status_response_t *resp)
{
	uint8_t info[8];

	memset(resp, 0, sizeof(*resp));

	if (read_retry(fd, info, sizeof(info)) <= 0)
		return -1;

	resp->state = info[0];
	resp->exit_status = info[1];

	resp->id = ((int)info[4] << 24) | ((int)info[5] << 16) |
		((int)info[6] << 8) | (int)info[7];

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
