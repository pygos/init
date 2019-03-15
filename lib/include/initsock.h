/* SPDX-License-Identifier: ISC */
#ifndef INITSOCK_H
#define INITSOCK_H

#include "config.h"

#define INIT_SOCK_PATH SOCKDIR "/init.sock"

typedef enum {
	EIR_STATUS = 0x00,
} E_INIT_REQUEST;

typedef struct {
	E_INIT_REQUEST rq;
} init_request_t;

int init_socket_create(void);

int init_socket_open(const char *tmppath);

int init_socket_send_request(int fd, E_INIT_REQUEST rq);

#endif /* INITSOCK_H */
