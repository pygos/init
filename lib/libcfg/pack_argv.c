/* SPDX-License-Identifier: ISC */
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>

#include "libcfg.h"

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
