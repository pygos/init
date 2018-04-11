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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>

#include "service.h"
#include "util.h"

static int try_unescape(char *arg, rdline_t *rd)
{
	if (unescape(arg)) {
		fprintf(stderr, "%s: %zu: malformed string constant\n",
			rd->filename, rd->lineno);
		return -1;
	}
	return 0;
}

static char **try_split_argv(char *str, rdline_t *rd)
{
	char **argv = split_argv(str);

	if (argv == NULL) {
		switch (errno) {
		case EINVAL:
			fprintf(stderr, "%s: %zu: malformed string constant\n",
				rd->filename, rd->lineno);
			break;
		default:
			fprintf(stderr, "%s: %zu: %s\n",
				rd->filename, rd->lineno, strerror(errno));
			break;
		}
	}

	return argv;
}

static char *try_strdup(const char *str, rdline_t *rd)
{
	char *out = strdup(str);

	if (out == NULL) {
		fprintf(stderr, "%s: %zu: out of memory\n",
			rd->filename, rd->lineno);
	}
	return out;
}

static int svc_desc(service_t *svc, char *arg, rdline_t *rd)
{
	if (try_unescape(arg, rd))
		return -1;
	svc->desc = try_strdup(arg, rd);
	return svc->desc == NULL ? -1 : 0;
}

static int svc_tty(service_t *svc, char *arg, rdline_t *rd)
{
	if (try_unescape(arg, rd))
		return -1;
	svc->ctty = try_strdup(arg, rd);
	return svc->ctty == NULL ? -1 : 0;
}

static int svc_exec(service_t *svc, char *arg, rdline_t *rd)
{
	exec_t *e, *end;

	e = calloc(1, sizeof(*e) + strlen(arg) + 1);
	if (e == NULL) {
		fprintf(stderr, "%s: %zu: out of memory\n",
			rd->filename, rd->lineno);
		return -1;
	}

	strcpy(e->buffer, arg);

	e->argv = try_split_argv(e->buffer, rd);
	if (e->argv == NULL) {
		free(e);
		return -1;
	}

	if (svc->exec == NULL) {
		svc->exec = e;
	} else {
		for (end = svc->exec; end->next != NULL; end = end->next)
			;
		end->next = e;
	}
	return 0;
}

static int svc_before(service_t *svc, char *arg, rdline_t *rd)
{
	if (svc->before != NULL) {
		fprintf(stderr, "%s: %zu: 'before' dependencies respecified\n",
			rd->filename, rd->lineno);
		return -1;
	}

	svc->before = try_strdup(arg, rd);
	if (svc->before == NULL)
		return -1;

	svc->num_before = pack_argv(svc->before);
	if (svc->num_before < 0) {
		fprintf(stderr, "%s: %zu: malformed string constant\n",
				rd->filename, rd->lineno);
		return -1;
	}

	return 0;
}

static int svc_after(service_t *svc, char *arg, rdline_t *rd)
{
	if (svc->after != NULL) {
		fprintf(stderr, "%s: %zu: 'after' dependencies respecified\n",
			rd->filename, rd->lineno);
		return -1;
	}

	svc->after = try_strdup(arg, rd);
	if (svc->after == NULL)
		return -1;

	svc->num_after = pack_argv(svc->after);
	if (svc->num_after < 0) {
		fprintf(stderr, "%s: %zu: malformed string constant\n",
				rd->filename, rd->lineno);
		return -1;
	}

	return 0;
}

static int svc_type(service_t *svc, char *arg, rdline_t *rd)
{
	char *ptr;

	for (ptr = arg; *ptr != ' ' && *ptr != '\0'; ++ptr)
		;
	if (*ptr == ' ')
		*(ptr++) = '\0';

	svc->type = svc_type_from_string(arg);

	if (svc->type == -1) {
		fprintf(stderr, "%s: %zu: unknown service type '%s'\n",
			rd->filename, rd->lineno, arg);
		return -1;
	}

	if (*ptr != '\0') {
		switch (svc->type) {
		case SVC_RESPAWN:
			for (arg = ptr; *ptr != ' ' && *ptr != '\0'; ++ptr)
				;
			if (*ptr == ' ')
				*(ptr++) = '\0';

			if (strcmp(arg, "limit") != 0)
				goto fail_limit;

			svc->rspwn_limit = 0;

			if (!isdigit(*ptr))
				goto fail_limit;

			while (isdigit(*ptr)) {
				svc->rspwn_limit *= 10;
				svc->rspwn_limit += *(ptr++) - '0';
			}

			if (*ptr == '\0')
				break;
			if (*ptr != ' ')
				goto fail_limit;
			/* fall-through */
		default:
			fprintf(stderr,
				"%s: %zu: unexpected extra arguments\n",
				rd->filename, rd->lineno);
			return -1;
		}
	}

	return 0;
fail_limit:
	fprintf(stderr, "%s: %zu: expected 'limit <value>' after 'respawn'\n",
		rd->filename, rd->lineno);
	return -1;
}

static int svc_target(service_t *svc, char *arg, rdline_t *rd)
{
	int target;

	if (try_unescape(arg, rd))
		return -1;

	target = svc_target_from_string(arg);

	if (target == -1) {
		fprintf(stderr, "%s: %zu: unknown service target '%s'\n",
			rd->filename, rd->lineno, arg);
		return -1;
	}

	svc->target = target;
	return 0;
}

static const struct svc_param {
	const char *key;

	int (*handle)(service_t *svc, char *arg, rdline_t *rd);
} svc_params[] = {
	{ "description", svc_desc },
	{ "exec", svc_exec },
	{ "type", svc_type },
	{ "target", svc_target },
	{ "tty", svc_tty },
	{ "before", svc_before },
	{ "after", svc_after },
};

static int splitkv(rdline_t *rd, char **k, char **v)
{
	char *key = rd->buffer, *value = rd->buffer;

	while (*value != ' ' && *value != '\0') {
		if (!isalpha(*value)) {
			fprintf(stderr,
				"%s: %zu: unexpected '%c' in keyword\n",
				rd->filename, rd->lineno, *value);
			return -1;
		}
		++value;
	}

	if (*value != ' ') {
		fprintf(stderr, "%s: %zu: expected argument after '%s'\n",
			rd->filename, rd->lineno, key);
		return -1;
	}

	*(value++) = '\0';

	*k = key;
	*v = value;
	return 0;
}

static const struct svc_param *find_param(rdline_t *rd, const char *name)
{
	size_t i;

	for (i = 0; i < ARRAY_SIZE(svc_params); ++i) {
		if (!strcmp(svc_params[i].key, name))
			return svc_params + i;
	}

	fprintf(stderr, "%s: %zu: unknown keyword '%s'\n",
		rd->filename, rd->lineno, name);
	return NULL;
}


service_t *rdsvc(int dirfd, const char *filename)
{
	const struct svc_param *p;
	const char *arg, *args[1];
	service_t *svc = NULL;
	char *key, *value;
	size_t argc, nlen;
	rdline_t rd;
	int fd, ret;

	fd = openat(dirfd, filename, O_RDONLY);
	if (fd < 0) {
		perror(filename);
		return NULL;
	}

	arg = strchr(filename, '@');
	if (arg != NULL) {
		args[0] = arg + 1;
		argc = 1;
	} else {
		argc = 0;
	}

	rdline_init(&rd, fd, filename, argc, args);

	nlen = (arg != NULL) ? (size_t)(arg - filename) : strlen(filename);

	svc = calloc(1, sizeof(*svc) + nlen + 1);
	if (svc == NULL)
		goto fail_oom;

	memcpy(svc->name, filename, nlen);

	while ((ret = rdline(&rd)) == 0) {
		if (splitkv(&rd, &key, &value))
			goto fail;

		p = find_param(&rd, key);
		if (p == NULL)
			goto fail;

		if (p->handle(svc, value, &rd))
			goto fail;
	}

	if (ret < 0)
		goto fail;

	close(fd);
	return svc;
fail_oom:
	fputs("out of memory\n", stderr);
fail:
	delsvc(svc);
	close(fd);
	return NULL;
}
