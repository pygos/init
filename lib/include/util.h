/* SPDX-License-Identifier: ISC */
#ifndef UTIL_H
#define UTIL_H

#include <sys/types.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "config.h"

#ifdef __GNUC__
	#define NORETURN __attribute__((noreturn))
#endif

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

typedef struct exec_t {
	struct exec_t *next;
	int argc;		/* number of elements in argument vector */
	char args[];		/* argument vectot string blob */
} exec_t;

enum {
	/* only allow root to connect */
	SOCK_FLAG_ROOT_ONLY = 0x01,

	/* allow everyone to connect */
	SOCK_FLAG_EVERYONE = 0x02,

	/* create a datagram socket, otherwise use a stream socket */
	SOCK_FLAG_DGRAM = 0x04,
};

int setup_tty(const char *tty, bool truncate);

NORETURN void argv_exec(exec_t *e);

#endif /* UTIL_H */
