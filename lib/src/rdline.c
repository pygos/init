#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>

#include "util.h"

char *rdline(int fd)
{
	size_t i = 0, bufsiz = 0, newsz;
	char c, *new, *buffer = NULL;
	int ret;

	for (;;) {
		switch (read(fd, &c, 1)) {
		case 0:
			if (i == 0) {
				errno = 0;
				return NULL;
			}
			c = '\0';
			break;
		case 1:
			if (c == '\n')
				c = '\0';
			break;
		default:
			if (errno == EINTR)
				continue;
			goto fail;
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
	return buffer;
fail:
	ret = errno;
	free(buffer);
	errno = ret;
	return NULL;
}
