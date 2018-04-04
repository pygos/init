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
#include <errno.h>
#include <ctype.h>

#include "util.h"

enum {
	STATE_INITIAL = 0,
	STATE_STRING = 1,
	STATE_STRING_ESC = 2,
	STATE_COMMENT = 3,
	STATE_ARG = 4,
};

char *rdline(int fd, int argc, const char *const *argv)
{
	size_t i = 0, bufsiz = 0, newsz;
	int ret, state = STATE_INITIAL;
	char c, *new, *buffer = NULL;
	const char *argstr = NULL;

	for (;;) {
		if (argstr == NULL) {
			switch (read(fd, &c, 1)) {
			case 0:
				if (i == 0) {
					errno = 0;
					return NULL;
				}
				c = '\0';
				break;
			case 1:
				break;
			default:
				if (errno == EINTR)
					continue;
				goto fail;
			}
		} else {
			c = *(argstr++);

			if (c == '\0') {
				argstr = NULL;
				continue;
			}
		}

		if (c == '\n')
			c = '\0';

		switch (state) {
		case STATE_STRING:
			if (c == '\\')
				state = STATE_STRING_ESC;
			if (c == '"')
				state = STATE_INITIAL;
			break;
		case STATE_STRING_ESC:
			state = STATE_STRING;
			break;
		case STATE_COMMENT:
			if (c != '\0')
				continue;
			break;
		case STATE_ARG:
			state = STATE_INITIAL;
			if (c == '%')
				break;
			if (!isdigit(c) || (c - '0') >= argc) {
				errno = EINVAL;
				goto fail;
			}
			if (argstr != NULL) {
				errno = ELOOP;
				goto fail;
			}
			argstr = argv[c - '0'];
			continue;
		default:
			if (isspace(c))
				c = ' ';
			if (c == ' ' && (i == 0 || buffer[i - 1] == ' '))
				continue;
			if (c == '#') {
				state = STATE_COMMENT;
				continue;
			}
			if (c == '%') {
				state = STATE_ARG;
				continue;
			}
			if (c == '"')
				state = STATE_STRING;
			break;
		}

		if (c == '\0') {
			while (i > 0 && buffer[i - 1] == ' ')
				--i;
		}

		if (i == bufsiz) {
			newsz = bufsiz ? bufsiz * 2 : 16;
			new = realloc(buffer, newsz);

			if (new == NULL)
				goto fail;

			buffer = new;
			bufsiz = newsz;
		}

		buffer[i++] = c;
		if (c == '\0')
			break;
	}

	if (state == STATE_STRING || state == STATE_STRING_ESC) {
		errno = EILSEQ;
		goto fail;
	}
	return buffer;
fail:
	ret = errno;
	free(buffer);
	errno = ret;
	return NULL;
}
