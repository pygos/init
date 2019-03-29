/* SPDX-License-Identifier: ISC */
#ifndef INITSOCK_H
#define INITSOCK_H

#include <stdint.h>

#include "config.h"
#include "service.h"

#define INIT_SOCK_PATH SOCKDIR "/init.sock"

typedef enum {
	EIR_STATUS = 0x00,
	EIR_START = 0x01,
	EIR_STOP = 0x02,
} E_INIT_REQUEST;

typedef enum {
	ESS_NONE = 0x00,
	ESS_RUNNING = 0x01,
	ESS_ENQUEUED = 0x02,
	ESS_DONE = 0x03,
	ESS_FAILED = 0x04
} E_SERVICE_STATE;

typedef struct {
	uint8_t rq;
	uint8_t padd[3];

	union {
		struct {
			uint8_t filter;
			uint8_t padd[3];
		} status;

		struct {
			uint32_t id;
		} startstop;
	} arg;
} init_request_t;

typedef struct {
	uint8_t state;
	uint8_t exit_status;
	uint8_t padd[2];
	int32_t id;
} init_response_status_t;

typedef struct {
	E_SERVICE_STATE state;
	int exit_status;
	int id;
	char *filename;
	char *service_name;
} init_status_t;

int init_socket_open(const char *tmppath);

int init_socket_send_request(int fd, E_INIT_REQUEST rq, ...);

int init_socket_recv_status(int fd, init_status_t *resp);

void free_init_status(init_status_t *resp);

#endif /* INITSOCK_H */
