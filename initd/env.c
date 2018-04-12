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
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#include "init.h"
#include "util.h"

struct entry {
	struct entry *next;
	char data[];
};

extern char **environ;

static void free_list(struct entry *list)
{
	struct entry *e;

	while (list != NULL) {
		e = list;
		list = list->next;
		free(e);
	}
}

static struct entry *parse_list(rdline_t *rd)
{
	struct entry *e, *list = NULL;
	char *ptr;

	while (rdline(rd) == 0) {
		ptr = rd->buffer;

		while (*ptr != '\0' && *ptr != ' ' && *ptr != '=')
			++ptr;

		if (*ptr == ' ')
			memmove(ptr, ptr + 1, strlen(ptr + 1) + 1);

		if (*(ptr++) != '=') {
			fprintf(stderr, "%s: %zu: line is not of the shape "
					"'key = value', skipping\n",
				rd->filename, rd->lineno);
			continue;
		}

		if (*ptr == ' ')
			memmove(ptr, ptr + 1, strlen(ptr + 1) + 1);

		if (unescape(ptr)) {
			fprintf(stderr, "%s: %zu: malformed string constant, "
					"skipping\n",
				rd->filename, rd->lineno);
			continue;
		}

		e = calloc(1, sizeof(*e) + strlen(rd->buffer) + 1);
		if (e == NULL)
			goto fail_oom;

		strcpy(e->data, rd->buffer);
		e->next = list;
		list = e;
	}

	return list;
fail_oom:
	fputs("out of memory\n", stderr);
	free_list(list);
	return NULL;
}

static struct entry *list_from_file(void)
{
	struct entry *list;
	rdline_t rd;
	int fd;

	fd = open(ENVFILE, O_RDONLY);
	if (fd < 0) {
		perror(ENVFILE);
		return NULL;
	}

	rdline_init(&rd, fd, ENVFILE, 0, NULL);
	list = parse_list(&rd);
	close(fd);
	return list;
}

int initenv(void)
{
	struct entry *list, *e;
	int i, count;
	char **envp;

	list = list_from_file();
	if (list == NULL)
		return -1;

	for (count = 0, e = list; e != NULL; e = e->next)
		++count;

	envp = malloc((count + 1) * sizeof(char *));
	if (envp == NULL) {
		fputs("out of memory\n", stderr);
		free_list(list);
		return -1;
	}

	for (i = 0, e = list; e != NULL; e = e->next)
		envp[i] = e->data;

	envp[i] = NULL;

	environ = envp;
	return 0;
}
