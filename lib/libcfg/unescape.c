/* SPDX-License-Identifier: ISC */
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
