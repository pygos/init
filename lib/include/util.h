/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Copyright (C) 2018 - David Oberhollenzer
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
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

typedef struct {
	const char *name;
	int value;
} enum_map_t;

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

/*
	Search through an array of enum_map_t entries to resolve a string to
	a numeric value. The end of the map is indicated by a sentinel entry
	with the name set to NULL.
*/
const enum_map_t *enum_by_name(const enum_map_t *map, const char *name);

/*
	Search through an array of enum_map_t entries to resolve a numeric
	value to a string name. The end of the map is indicated by a sentinel
	entry with the name set to NULL.
*/
const char *enum_to_name(const enum_map_t *map, int value);

/*
	Create a UNIX stream socket at the given path.

	Returns the socket fd, -1 on failure. The function takes care of
	printing error messages on failure.

	The socket has the CLOEXEC flag set.
*/
int mksock(const char *path, int flags);

/* print a default version info and license string */
NORETURN void print_version(const char *program);

int setup_tty(const char *tty, bool truncate);

NORETURN void argv_exec(exec_t *e);

/*
	Similar to openat: opens a file relative to a dirfd, but returns
	a FILE pointer instead of an fd.
 */
FILE *fopenat(int fd, const char *filename, const char *mode);

#endif /* UTIL_H */

