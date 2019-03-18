/* SPDX-License-Identifier: ISC */
#ifndef INITSOCK_H
#define INITSOCK_H

#include "config.h"
#include "service.h"

#define INIT_SOCK_PATH SOCKDIR "/init.sock"

typedef enum {
	EIR_STATUS = 0x00,
} E_INIT_REQUEST;

typedef enum {
	ESS_NONE = 0x00,
	ESS_RUNNING = 0x01,
	ESS_ENQUEUED = 0x02,
	ESS_EXITED = 0x03,
} E_SERVICE_STATE;

typedef struct {
	E_INIT_REQUEST rq;
} init_request_t;

typedef struct {
	E_SERVICE_STATE state;
	int exit_status;
	char *filename;
	char *service_name;
} init_status_response_t;

int init_socket_create(void);

int init_socket_open(const char *tmppath);

int init_socket_send_request(int fd, E_INIT_REQUEST rq);

int init_socket_send_status(int fd, const void *dest_addr, size_t addrlen,
			    E_SERVICE_STATE state, service_t *svc);

int init_socket_recv_status(int fd, init_status_response_t *resp);

#endif /* INITSOCK_H */
