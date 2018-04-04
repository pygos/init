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

	The following transformations are applied:
	 - Space characters are replaced with regular white space characters.
	 - Sequences of space characters are truncated to a single space.
	 - A '#' sign is interpreted as the start of a comment and removed,
	   together with everything that follows.
	 - Padding spaces are removed from the line.
	 - If a '"' is encounterd, the above rules are disabled, until a
	   after the matching '"' is read. A '"' can be escaped by preceeding
	   it with a backslash.
	 - If a second, coresponding '"' is not found, processing fails with
	   errno set to EILSEQ.
	 - If a '%' character is encountered, the next character is expected
	   to be a single digit index into argv. If it is not a digit or
	   outside the bounds set by argc, processing fails and sets errno
	   to EINVAL. On success, the argv value is inserted and processed
	   as described above.
	 - A '%' character can be escaped by writing '%%' or, if inside
	   a double quite string, by writing \%.
	 - An attempt to use such an indexed argument inside an argument
	   expansion, results in failure with errno set to ELOOP.
*/
char *rdline(int fd, int argc, const char *const *argv);

/*
	Remove double quotes ('"') from a string and substitute escape
	sequences in between double quotes.
*/
int unescape(char *src);

/*
	Split a space seperated string into a sequence of null-terminated
	strings. Return a NULL terminated array of strings pointing to the
	start of each sub string.

	If a double quote is encountered, the entire string up to to the next,
	unescaped double quite is interpreted as a single sub string and
	fed through the unescape function.

	The returned array must be freed with free().
*/
char **split_argv(char *str);

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

#endif /* UTIL_H */

