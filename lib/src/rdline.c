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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "util.h"

typedef struct {
	int fd;			/* input file descriptor */
	const char *argstr;	/* if not NULL, read from this instead */

	size_t i;		/* buffer offset */
	size_t bufsiz;		/* buffer size */
	char *buffer;

	bool string;		/* inside a string? */
	bool escape;		/* reading an escape sequence? */
	bool comment;		/* inside a comment */
} rdline_t;

static int rdline_getc(rdline_t *t)
{
	int ret;
	char c;

	if (t->argstr != NULL) {
		c = *(t->argstr++);
		if (c != '\0')
			goto out;

		t->argstr = NULL;
	}

	do {
		ret = read(t->fd, &c, 1);
	} while (ret < 0 && errno == EINTR);

	if (ret < 0)
		return -1;

	if (ret == 0) {
		if (t->i == 0) {
			errno = 0;
			return -1;
		}
		c = '\0';
	}
out:
	return (c == '\n') ? '\0' : c;
}

static int rdline_append(rdline_t *t, int c)
{
	size_t newsz;
	char *new;

	if (t->comment) {
		if (c != '\0')
			return 0;
	} else if (t->string) {
		if (t->escape) {
			t->escape = false;
		} else {
			if (c == '\\')
				t->escape = true;
			if (c == '"')
				t->string = false;
		}
	} else {
		if (isspace(c))
			c = ' ';
		if (c == ' ' && (t->i == 0 || t->buffer[t->i - 1] == ' '))
			return 0;
		if (c == '#') {
			t->comment = true;
			return 0;
		}
		if (c == '"')
			t->string = true;
	}

	if (c == '\0') {
		while (t->i > 0 && t->buffer[t->i - 1] == ' ')
			t->i -= 1;
	}

	if (t->i == t->bufsiz) {
		newsz = t->bufsiz ? t->bufsiz * 2 : 16;
		new = realloc(t->buffer, newsz);

		if (new == NULL)
			return -1;

		t->buffer = new;
		t->bufsiz = newsz;
	}

	t->buffer[t->i++] = c;
	return 0;
}

char *rdline(int fd, int argc, const char *const *argv)
{
	rdline_t rd;
	int ret;
	char c;

	memset(&rd, 0, sizeof(rd));
	rd.fd = fd;

	do {
		c = rdline_getc(&rd);
		if (c < 0)
			goto fail;
		if (c == 0 && rd.string) {
			errno = EILSEQ;
			goto fail;
		}

		if (c == '%') {
			c = rdline_getc(&rd);
			if (c == 0)
				errno = EILSEQ;
			if (c <= 0)
				goto fail;

			if (c != '%') {
				if (!isdigit(c) || (c - '0') >= argc) {
					errno = EINVAL;
					goto fail;
				}
				if (rd.argstr != NULL) {
					errno = ELOOP;
					goto fail;
				}
				rd.argstr = argv[c - '0'];
				continue;
			}
		}

		if (rdline_append(&rd, c))
			goto fail;
	} while (c != '\0');

	return rd.buffer;
fail:
	ret = errno;
	free(rd.buffer);
	errno = ret;
	return NULL;
}
