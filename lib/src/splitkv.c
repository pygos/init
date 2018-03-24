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
#include <ctype.h>

#include "util.h"

static char *skpspc(char *ptr)
{
	while (*ptr == ' ' || *ptr == '\t')
		++ptr;
	return ptr;
}

static int xdigit(int x)
{
	if (isupper(x))
		return x - 'A' + 0x0A;
	if (islower(x))
		return x - 'a' + 0x0A;
	return x - '0';
}

static char *parse_str(char *src)
{
	char *dst = src;
	int c;

	for (;;) {
		c = *(src++);

		switch (c) {
		case '\\':
			c = *(src++);

			switch (c) {
			case 'a': c = '\a'; break;
			case 'b': c = '\b'; break;
			case 'f': c = '\f'; break;
			case 'n': c = '\n'; break;
			case 't': c = '\t'; break;
			case '\\': break;
			case '"': break;
			case 'x':
				c = 0;
				if (isxdigit(*src))
					c = (c << 4) | xdigit(*(src++));
				if (isxdigit(*src))
					c = (c << 4) | xdigit(*(src++));
				break;
			case '0':
				c = 0;
				if (isdigit(*src) && *src < '8')
					c = (c << 3) | (*(src++) - '0');
				if (isdigit(*src) && *src < '8')
					c = (c << 3) | (*(src++) - '0');
				if (isdigit(*src) && *src < '8')
					c = (c << 3) | (*(src++) - '0');
				break;
			default:
				return NULL;
			}
			break;
		case '"':
			*(dst++) = '\0';
			goto out;
		}

		*(dst++) = c;
	}
out:
	return src;
}

int splitkv(char *line, char **key, char **value)
{
	*key = NULL;
	*value = NULL;

	line = skpspc(line);

	if (*line == '#' || *line == '\0')
		return 0;

	if (!isalpha(*line))
		return -1;

	*key = line;

	while (isalnum(*line))
		++line;

	if (*line == ' ' || *line == '\t') {
		*(line++) = '\0';
		line = skpspc(line);
	}

	if (*line != '=')
		return -1;

	*(line++) = '\0';
	line = skpspc(line);

	if (*line == '"') {
		++line;
		*value = line;

		line = parse_str(line);
	} else if (isalnum(*line)) {
		*value = line;

		while (isalnum(*line) || *line == '.' || *line == '_')
			++line;

		if (*line != '\0')
			*(line++) = '\0';
	} else {
		return -1;
	}

	line = skpspc(line);

	if (*line != '\0' && *line != '#')
		return -1;

	return 0;
}
