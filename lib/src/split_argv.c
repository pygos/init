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
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>

#include "util.h"

int pack_argv(char *str)
{
	char *dst, *start;
	int count = 0;

	dst = str;

	for (;;) {
		while (*str == ' ')
			++str;

		if (*str == '\0')
			break;

		if (*str == '"') {
			start = dst;
			*(dst++) = *(str++);

			while (*str != '"') {
				if (*str == '\0')
					goto fail_str;
				if (str[0] == '\\' && str[1] != '\0')
					*(dst++) = *(str++);
				*(dst++) = *(str++);
			}

			*(dst++) = *(str++);

			if (*str != ' ' && *str != '\0')
				goto fail_str;
			if (*str == ' ')
				++str;

			*(dst++) = '\0';

			if (unescape(start))
				goto fail_str;

			dst = start + strlen(start) + 1;
		} else {
			while (*str != '\0' && *str != ' ')
				*(dst++) = *(str++);
			if (*str == ' ') {
				++str;
				*(dst++) = '\0';
			}
		}

		++count;
	}

	*dst = '\0';
	return count;
fail_str:
	errno = EINVAL;
	return -1;
}

char **split_argv(char *str)
{
	char **argv = NULL;
	int i, count;

	count = pack_argv(str);
	if (count <= 0)
		return NULL;

	argv = malloc(sizeof(argv[0]) * (count + 1));
	if (argv == NULL)
		return NULL;

	for (i = 0; i < count; ++i) {
		argv[i] = str;
		str += strlen(str) + 1;
	}

	argv[i] = NULL;
	return argv;
}
