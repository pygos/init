/* SPDX-License-Identifier: ISC */
#ifndef LIBCONFIG_H
#define LIBCONFIG_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

typedef struct {
	const char *filename;	/* input file name */
	size_t lineno;		/* current line number */
	FILE *fp;
	char *line;

	int argc;
	const char *const *argv;
} rdline_t;

typedef struct {
	/* keyword to map the callback to */
	const char *key;

	/*
		If set, allow grouping repetitions of the keyword in a single
		multi line '{' ... '}' block. The callback is called for each
		line.
	 */
	unsigned int allow_block : 1;

	int (*handle)(void *obj, char *arg, rdline_t *rd, int flags);
} cfg_param_t;

/*
	Initialize the config line scanner.

	The scanner opens the filename relative to the passed dirfd. An
	argument count and vector can be set for argument substitution
	in rdline.

	Returns 0 on success.
*/
int rdline_init(rdline_t *t, int dirfd, const char *filename,
		int argc, const char *const *argv);

void rdline_cleanup(rdline_t *t);

/*
	Read from file until end-of-file or a line feed is encountered.

	Returns -1 on failure, +1 if end of file was reached,
	0 if data was read successfully.

	The following transformations are applied:
	 - Space characters are replaced with regular white space characters.
	 - Sequences of space characters are truncated to a single space.
	 - A '#' sign is interpreted as the start of a comment and removed,
	   together with everything that follows.
	 - Padding spaces are removed from the line.
	 - If a '"' is encounterd, the above rules are disabled, until a
	   after the matching '"' is read. A '"' can be escaped by preceeding
	   it with a backslash.
	 - If a second, coresponding '"' is not found, processing fails.
	 - If a '%' character is encountered, the next character is expected
	   to be a single digit index into argv. If it is not a digit or
	   outside the bounds set by argc, processing fails. On success,
	   the argv value is inserted and processed as described above.
	 - A '%' character can be escaped by writing '%%' or, if inside
	   a double quoted string, by writing \%.
	 - Arguments are pasted as is. Substitution is not recursive.
	 - If the resulting line is empty, processing is restarted.
*/
int rdline(rdline_t *t);

/*
	Remove double quotes ('"') from a string and substitute escape
	sequences in between double quotes.
*/
int unescape(char *src);

/*
	Replace spaces in 'str' with null bytes. Tread strings (started and
	terminated with double-quotes which can be escaped) as a single block.
	Such strings are run through unescap(). All elements are tightly
	packed together and the function returns the number of consecutive
	argument strings that are now inside 'str'.

	Returns a negative value if unescape() fails, a string is not
	termianted or two such strings touch each other without a white
	space in between.
*/
int pack_argv(char *str);

/*
	Parse a configuration file containing '<keyword> [arguments...]' lines.
	The cfgobj and flags are passed to the callback in the params array.

	Returns zero on success.
 */
int rdcfg(void *cfgobj, rdline_t *rd, const cfg_param_t *params, size_t count,
	  int flags);

#endif /* LIBCONFIG_H */
