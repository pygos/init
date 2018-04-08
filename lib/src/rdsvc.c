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

static int try_unescape(char *arg, const char *filename, size_t lineno)
{
	if (unescape(arg)) {
		fprintf(stderr, "%s: %zu: malformed string constant\n",
			filename, lineno);
		return -1;
	}
	return 0;
}

static char **try_split_argv(char *str, const char *filename, size_t lineno)
{
	char **argv = split_argv(str);

	if (argv == NULL) {
		switch (errno) {
		case EINVAL:
			fprintf(stderr, "%s: %zu: malformed string constant\n",
				filename, lineno);
			break;
		default:
			fprintf(stderr, "%s: %zu: %s\n", filename, lineno,
				strerror(errno));
			break;
		}
	}

	return argv;
}

static int svc_desc(service_t *svc, char *arg,
		    const char *filename, size_t lineno)
{
	if (try_unescape(arg, filename, lineno))
		return -1;
	svc->desc = arg;
	return 0;
}

static int svc_tty(service_t *svc, char *arg,
		   const char *filename, size_t lineno)
{
	if (try_unescape(arg, filename, lineno))
		return -1;
	svc->ctty = arg;
	return 0;
}

static int svc_exec(service_t *svc, char *arg,
		    const char *filename, size_t lineno)
{
	exec_t *e, *end;
	char **argv;

	argv = try_split_argv(arg, filename, lineno);
	if (argv == NULL)
		return -1;

	e = calloc(1, sizeof(*e));
	if (e == NULL) {
		fprintf(stderr, "%s: %zu: out of memory\n", filename, lineno);
		free(argv);
		return -1;
	}

	e->argv = argv;
	e->raw_argv = arg;

	if (svc->exec == NULL) {
		svc->exec = e;
	} else {
		for (end = svc->exec; end->next != NULL; end = end->next)
			;
		end->next = e;
	}
	return 0;
}

static int svc_before(service_t *svc, char *arg,
		      const char *filename, size_t lineno)
{
	if (svc->before != NULL) {
		fprintf(stderr, "%s: %zu: 'before' dependencies respecified\n",
			filename, lineno);
		return -1;
	}

	svc->before = try_split_argv(arg, filename, lineno);
	if (svc->before == NULL)
		return -1;

	svc->raw_before = arg;
	return 0;
}

static int svc_after(service_t *svc, char *arg,
		     const char *filename, size_t lineno)
{
	if (svc->after != NULL) {
		fprintf(stderr, "%s: %zu: 'after' dependencies respecified\n",
			filename, lineno);
		return -1;
	}

	svc->after = try_split_argv(arg, filename, lineno);
	if (svc->after == NULL)
		return -1;

	svc->raw_after = arg;
	return 0;
}

static int svc_type(service_t *svc, char *arg,
		    const char *filename, size_t lineno)
{
	char **args;
	int i, type;

	args = try_split_argv(arg, filename, lineno);

	if (args == NULL)
		return -1;

	type = svc_type_from_string(args[0]);

	if (type == -1) {
		fprintf(stderr, "%s: %zu: unknown service type '%s'\n",
			filename, lineno, args[0]);
		free(args);
		return -1;
	}

	if (args[1] != NULL) {
		switch (type) {
		case SVC_RESPAWN:
			if (strcmp(args[1], "limit") != 0)
				goto fail_limit;

			svc->rspwn_limit = 0;

			if (!isdigit(args[2][0]))
				goto fail_limit;

			for (i = 0; isdigit(args[2][i]); ++i) {
				svc->rspwn_limit *= 10;
				svc->rspwn_limit += args[2][i] - '0';
			}

			if (args[2][i] != '\0')
				goto fail_limit;
			if (args[3] == NULL)
				break;
			/* fall-through */
		default:
			fprintf(stderr, "%s: %zu: unexpected extra arguments "
				"for type '%s'\n", filename, lineno, arg);
			return -1;
		}
	}

	svc->type = type;
	free(args);
	free(arg);
	return 0;
fail_limit:
	fprintf(stderr, "%s: %zu: expected 'limit <value>' after 'respawn'\n",
		filename, lineno);
	free(args);
	return -1;
}

static int svc_target(service_t *svc, char *arg,
		      const char *filename, size_t lineno)
{
	int target;

	if (try_unescape(arg, filename, lineno))
		return -1;

	target = svc_target_from_string(arg);

	if (target == -1) {
		fprintf(stderr, "%s: %zu: unknown service target '%s'\n",
			filename, lineno, arg);
		return -1;
	}

	svc->target = target;
	free(arg);
	return 0;
}

static const struct svc_param {
	const char *key;

	int (*handle)(service_t *svc, char *arg,
		      const char *filename, size_t lineno);
} svc_params[] = {
	{ "description", svc_desc },
	{ "exec", svc_exec },
	{ "type", svc_type },
	{ "target", svc_target },
	{ "tty", svc_tty },
	{ "before", svc_before },
	{ "after", svc_after },
};


static char *get_line(int fd, const char *filename, size_t *lineno,
		      int argc, const char *const *args)
{
	const char *error;
	char *line;
	int ret;
retry:
	errno = 0;
	line = rdline(fd, argc, args);
	ret = errno;

	if (line == NULL && errno != 0) {
		switch (errno) {
		case EINVAL: error = "error in argument expansion";  break;
		case ELOOP:  error = "recursive argument expansion"; break;
		case EILSEQ: error = "missing \"";                   break;
		default:     error = strerror(errno);                break;
		}

		fprintf(stderr, "%s: %zu: %s\n", filename, *lineno, error);
	}

	if (line != NULL && strlen(line) == 0) {
		free(line);
		(*lineno) += 1;
		goto retry;
	}

	errno = ret;
	return line;
}

static int splitkv(const char *filename, size_t lineno,
		   char *line, char **k, char **v)
{
	char *key = line, *value = line;

	while (*value != ' ' && *value != '\0') {
		if (!isalpha(*value)) {
			fprintf(stderr,
				"%s: %zu: unexpected '%c' in keyword\n",
				filename, lineno, *value);
			return -1;
		}
		++value;
	}

	if (*value != ' ') {
		fprintf(stderr, "%s: %zu: expected argument after '%s'\n",
			filename, lineno, key);
		return -1;
	}

	*(value++) = '\0';

	*k = key;
	*v = value;
	return 0;
}

static const struct svc_param *find_param(const char *filename, size_t lineno,
					  const char *name)
{
	size_t i;

	for (i = 0; i < ARRAY_SIZE(svc_params); ++i) {
		if (!strcmp(svc_params[i].key, name))
			return svc_params + i;
	}

	fprintf(stderr, "%s: %zu: unknown keyword '%s'\n",
		filename, lineno, name);
	return NULL;
}


service_t *rdsvc(int dirfd, const char *filename)
{
	char *line = NULL, *key, *value;
	const struct svc_param *p;
	const char *arg, *args[1];
	service_t *svc = NULL;
	size_t argc, lineno;
	int fd;

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

	svc = calloc(1, sizeof(*svc));
	if (svc == NULL)
		goto fail_oom;

	if (arg != NULL) {
		svc->name = strndup(filename, arg - filename);
	} else {
		svc->name = strdup(filename);
	}

	if (svc->name == NULL)
		goto fail_oom;

	for (lineno = 1; ; ++lineno) {
		line = get_line(fd, filename, &lineno, argc, args);
		if (line == NULL) {
			if (errno == 0)
				break;
			goto fail;
		}

		if (splitkv(filename, lineno, line, &key, &value))
			goto fail;

		p = find_param(filename, lineno, key);
		if (p == NULL)
			goto fail;

		memmove(line, value, strlen(value) + 1);
		if (p->handle(svc, line, filename, lineno))
			goto fail;
	}

	close(fd);
	return svc;
fail_oom:
	fputs("out of memory\n", stderr);
fail:
	free(line);
	delsvc(svc);
	close(fd);
	return NULL;
}
