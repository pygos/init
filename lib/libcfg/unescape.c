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
#include <ctype.h>
#include <errno.h>

#include "libcfg.h"

static int xdigit(int x)
{
	if (isupper(x))
		return x - 'A' + 0x0A;
	if (islower(x))
		return x - 'a' + 0x0A;
	return x - '0';
}

int unescape(char *src)
{
	char *dst = src;
	int c;

	for (;;) {
		while (*src != '"' && *src != '\0')
			*(dst++) = *(src++);

		if (*src == '\0')
			break;

		++src;

		while ((c = *(src++)) != '"') {
			if (c == '\0')
				return -1;

			if (c == '\\') {
				c = *(src++);

				switch (c) {
				case 'a': c = '\a'; break;
				case 'b': c = '\b'; break;
				case 'f': c = '\f'; break;
				case 'n': c = '\n'; break;
				case 't': c = '\t'; break;
				case '\\':
				case '"':
				case '%':
					break;
				case 'x':
					c = 0;
					if (isxdigit(*src))
						c = (c<<4) | xdigit(*(src++));
					if (isxdigit(*src))
						c = (c<<4) | xdigit(*(src++));
					break;
				case '0':
					c = 0;
					if (isdigit(*src) && *src < '8')
						c = (c<<3) | (*(src++) - '0');
					if (isdigit(*src) && *src < '8')
						c = (c<<3) | (*(src++) - '0');
					if (isdigit(*src) && *src < '8')
						c = (c<<3) | (*(src++) - '0');
					break;
				default:
					return -1;
				}

				if (c == 0)
					return -1;
			}

			*(dst++) = c;
		}
	}

	*(dst++) = '\0';
	return 0;
}
