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
#include <stdio.h>

#include "libcfg.h"

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

	if (t->i == sizeof(t->buffer))
		return -1;

	t->buffer[t->i++] = c;
	return 0;
}

void rdline_init(rdline_t *t, int fd, const char *filename,
		 int argc, const char *const *argv)
{
	memset(t, 0, sizeof(*t));
	t->fd = fd;
	t->filename = filename;
	t->argc = argc;
	t->argv = argv;
}

int rdline(rdline_t *t)
{
	const char *errstr;
	int c;
retry:
	t->i = 0;
	t->argstr = NULL;
	t->string = t->escape = t->comment = false;
	t->lineno += 1;

	do {
		errno = 0;
		c = rdline_getc(t);
		if (c < 0) {
			if (errno == 0)
				return 1;
			errstr = strerror(errno);
			goto fail;
		}
		if (c == 0 && t->string) {
			errstr = "missing \"";
			goto fail;
		}

		if (c == '%') {
			c = rdline_getc(t);
			if (c == 0) {
				errstr = "unexpected end of line after '%%'";
				goto fail;
			}
			if (c < 0) {
				errstr = strerror(errno);
				goto fail;
			}

			if (c != '%') {
				if (!isdigit(c)) {
					errstr = "exptected digit after '%%'";
					goto fail;
				}
				if ((c - '0') >= t->argc) {
					errstr = "argument out of range";
					goto fail;
				}
				if (t->argstr != NULL) {
					errstr = "recursive argument "
						 "expansion";
					goto fail;
				}
				t->argstr = t->argv[c - '0'];
				continue;
			}
		}

		if (rdline_append(t, c)) {
			errstr = "line too long";
			goto fail;
		}
	} while (c != '\0');

	if (t->buffer[0] == '\0')
		goto retry;

	return 0;
fail:
	fprintf(stderr, "%s: %zu: %s\n", t->filename, t->lineno, errstr);
	return -1;
}
