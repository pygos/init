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
