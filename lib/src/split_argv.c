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

#include "util.h"

char **split_argv(char *str)
{
	size_t i = 0, cap = 0, new_cap;
	char **argv = NULL, **new;
	char *ptr;

	ptr = str;

	for (;;) {
		if (*ptr == ' ') {
			++ptr;
			continue;
		}

		if (i == cap) {
			new_cap = cap ? cap * 2 : 16;
			new = realloc(argv, sizeof(argv[0]) * new_cap);

			if (new == NULL) {
				free(argv);
				errno = ENOMEM;
				return NULL;
			}

			cap = new_cap;
			argv = new;
		}

		if (*ptr == '\0') {
			argv[i++] = NULL;
			break;
		}

		argv[i++] = ptr;

		if (*ptr == '"') {
			++ptr;
			while (*ptr != '\0' && *ptr != '"') {
				if (ptr[0] == '\\' && ptr[1] != '\0')
					++ptr;

				++ptr;
			}

			if (*ptr == '"')
				++ptr;

			if (*ptr == ' ') {
				*(ptr++) = '\0';
			} else if (*ptr != '\0') {
				goto fail_str;
			}

			if (unescape(argv[i - 1]))
				goto fail_str;
		} else {
			while (*ptr != '\0' && *ptr != ' ')
				++ptr;

			if (*ptr == ' ')
				*(ptr++) = '\0';
		}
	}

	return argv;
fail_str:
	free(argv);
	errno = EINVAL;
	return NULL;
}
