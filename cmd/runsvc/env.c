/* SPDX-License-Identifier: ISC */
#include "runsvc.h"

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
		ptr = rd->line;

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

		e = calloc(1, sizeof(*e) + strlen(rd->line) + 1);
		if (e == NULL)
			goto fail_oom;

		strcpy(e->data, rd->line);
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

	if (rdline_init(&rd, AT_FDCWD, ENVFILE, 0, NULL))
		return NULL;

	list = parse_list(&rd);
	rdline_cleanup(&rd);
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
