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

char *strexpand(const char *inp, size_t argc, const char *const *argv);

#endif /* UTIL_H */

