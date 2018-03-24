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
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "util.h"

char *strexpand(const char *inp, size_t argc, const char *const *argv)
{
	char *out, *dst;
	const char *ptr;
	size_t i, len;

	ptr = inp;
	len = 0;

	while (*ptr != '\0') {
		if (ptr[0] == '%' && isdigit(ptr[1])) {
			i = ptr[1] - '0';
			if (i < argc)
				len += strlen(argv[i]);
			ptr += 2;
		} else if (ptr[0] == '%' && ptr[1] == '%') {
			ptr += 2;
			len += 1;
		} else {
			++ptr;
			++len;
		}
	}

	out = calloc(1, len + 1);
	if (out == NULL)
		return NULL;

	dst = out;

	while (*inp != '\0') {
		if (inp[0] == '%' && isdigit(inp[1])) {
			i = inp[1] - '0';
			if (i < argc) {
				len = strlen(argv[i]);
				memcpy(dst, argv[i], len);
				dst += len;
			}
			inp += 2;
		} else if (inp[0] == '%' && inp[1] == '%') {
			*(dst++) = '%';
			inp += 2;
		} else {
			*(dst++) = *(inp++);
		}
	}

	return out;
}
