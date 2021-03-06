/* SPDX-License-Identifier: ISC */
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <fcntl.h>

#include "libcfg.h"

int rdline_init(rdline_t *t, int dirfd, const char *filename,
		int argc, const char *const *argv)
{
	int fd = openat(dirfd, filename, O_RDONLY);

	if (fd == -1)
		goto fail_open;

	memset(t, 0, sizeof(*t));

	t->fp = fdopen(fd, "r");
	if (t->fp == NULL) {
		close(fd);
		goto fail_open;
	}

	t->filename = filename;
	t->argc = argc;
	t->argv = argv;
	return 0;
fail_open:
	perror(filename);
	return -1;
}

void rdline_cleanup(rdline_t *t)
{
	free(t->line);
	fclose(t->fp);
}

static int read_raw_line(rdline_t *t)
{
	size_t len = 0;

	free(t->line);
	t->line = NULL;

	errno = 0;

	if (getline(&t->line, &len, t->fp) < 0) {
		if (errno) {
			fprintf(stderr, "%s: %zu: %s\n", t->filename,
				t->lineno, strerror(errno));
			return -1;
		}
		return 1;
	}

	t->lineno += 1;
	return 0;
}

static int normalize_line(rdline_t *t)
{
	char *dst = t->line, *src = t->line;
	bool string = false;
	const char *errstr;
	int c, ret = 0;

	while (isspace(*src))
		++src;

	do {
		c = *(src++);

		if (c == '"') {
			string = !string;
		} else if (!string && c == '#') {
			c = '\0';
		} else if (!string && isspace(c)) {
			if (*src == '#' || *src == '\0' || isspace(*src))
				continue;
			c = ' ';
		} else if (c == '%') {
			*(dst++) = c;
			c = *(src++);
			if (isdigit(c)) {
				if ((c - '0') >= t->argc) {
					errstr = "argument out of range";
					goto fail;
				}
				ret += strlen(t->argv[c - '0']);
			} else if (c != '%') {
				errstr = "expected digit after '%%'";
				goto fail;
			}
		} else if (string && c == '\\' && *src != '\0') {
			*(dst++) = c;
			c = *(src++);
		}

		*(dst++) = c;
	} while (c != '\0');

	if (string) {
		errstr = "missing \"";
		goto fail;
	}
	return ret;
fail:
	fprintf(stderr, "%s: %zu: %s\n", t->filename, t->lineno, errstr);
	return -1;
}

static void substitute(rdline_t *t, char *dst, char *src)
{
	bool string = false;

	while (*src != '\0') {
		if (src[0] == '%' && isdigit(src[1])) {
			strcpy(dst, t->argv[src[1] - '0']);
			dst += strlen(dst);
			src += 2;
		} else if (src[0] == '%' && src[1] == '%') {
			*(dst++) = '%';
			src += 2;
		} else {
			if (*src == '"')
				string = !string;
			if (string && *src == '\\')
				*(dst++) = *(src++);
			*(dst++) = *(src++);
		}
	}

	*(dst++) = '\0';
}

int rdline(rdline_t *t)
{
	char *buffer = NULL;
	int ret;

	do {
		if ((ret = read_raw_line(t)))
			goto out;
		if ((ret = normalize_line(t)) < 0)
			goto out;
	} while (t->line[0] == '\0');

	if (ret == 0) {
		substitute(t, t->line, t->line);
		return 0;
	}

	buffer = calloc(1, strlen(t->line) + ret + 1);
	if (buffer == NULL) {
		fprintf(stderr, "%s: %zu: out of memory\n",
			t->filename, t->lineno);
		ret = -1;
		goto out;
	}

	substitute(t, buffer, t->line);
	ret = 0;
out:
	free(t->line);
	t->line = buffer;
	return ret;
}
