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

#include "config.h"

#ifdef __GNUC__
	#define NORETURN __attribute__((noreturn))
#endif

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

typedef struct {
	const char *name;
	int value;
} enum_map_t;


/*
	Read from fd until end-of-file or a line feed is encountered.

	Returns NULL with errno set on failure. Returns NULL with errno
	cleared if end-of-file is reached.

	The line must be deallocated with free().
*/
char *rdline(int fd);

/*
	Split a line of the shape "key = value" into key and value part.

	The key can contain alphanumeric characters and can be padded with
	spaces or tabs.

	The value can be either a sequence of alphanumeric characters, period
	or underscore OR a string in quotation marks. For strings, the
	quotation marks are removed and escape sequences are processed.

	The value may also be padded with spaces or tabs but the line may not
	contain anything else after the value, except for spaces, tabs or
	the '#' symbol which is interpreted as start of a comment.
*/
int splitkv(char *line, char **key, char **value);

/*
	Search through an array of enum_map_t entries to resolve a string to
	a numeric value. The end of the map is indicated by a sentinel entry
	with the name set to NULL.
*/
const enum_map_t *enum_by_name(const enum_map_t *map, const char *name);

/*
	Create a copy of the input string inp, but replace all occourances
	of %<number> with argv[number] if the number is within the bounds
	specified by argc.
*/
char *strexpand(const char *inp, size_t argc, const char *const *argv);

#endif /* UTIL_H */

