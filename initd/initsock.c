/* SPDX-License-Identifier: ISC */
#include "init.h"

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
	uint16_t raw;

	if (len > 0xFFFF)
		return -1;

	raw = htobe16(len);
	if (send_retry(fd, dst, addrlen, &raw, 2))
		return -1;

	return len > 0 ? send_retry(fd, dst, addrlen, str, len) : 0;
}

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

int init_socket_send_status(int fd, const void *dest_addr, size_t addrlen,
			    E_SERVICE_STATE state, service_t *svc)
{
	init_response_status_t info;

	memset(&info, 0, sizeof(info));

	if (svc == NULL || state == ESS_NONE) {
		info.state = ESS_NONE;
		info.id = -1;
	} else {
		info.state = state;
		info.exit_status = svc->status & 0xFF;
		info.id = htobe32(svc->id);
	}

	if (send_retry(fd, dest_addr, addrlen, &info, sizeof(info)))
		return -1;

	if (svc != NULL && state != ESS_NONE) {
		if (send_string(fd, dest_addr, addrlen, svc->fname))
			return -1;
		if (send_string(fd, dest_addr, addrlen, svc->name))
			return -1;
	}
	return 0;
}
