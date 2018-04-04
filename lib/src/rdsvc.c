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

static const struct {
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


service_t *rdsvc(int dirfd, const char *filename)
{
	const char *arg, *args[1], *error;
	char *line, *key, *value;
	size_t i, argc, lineno;
	service_t *svc;
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
	if (svc == NULL) {
		fputs("out of memory\n", stderr);
		close(fd);
		return NULL;
	}

	if (arg != NULL) {
		svc->name = strndup(filename, arg - filename);
	} else {
		svc->name = strdup(filename);
	}

	if (svc->name == NULL) {
		free(svc);
		fputs("out of memory\n", stderr);
		close(fd);
		return NULL;
	}

	for (lineno = 1; ; ++lineno) {
		errno = 0;
		line = rdline(fd, argc, args);

		if (line == NULL) {
			if (errno == 0)
				break;

			switch (errno) {
			case EINVAL:
				error = "error in argument expansion";
				break;
			case ELOOP:
				error = "recursive argument expansion";
				break;
			case EILSEQ:
				error = "missing \"";
				break;
			default:
				error = strerror(errno);
				break;
			}

			fprintf(stderr, "%s: %zu: %s\n",
				filename, lineno, error);
			goto fail;
		}

		if (!strlen(line)) {
			free(line);
			continue;
		}

		key = value = line;

		while (*value != ' ' && *value != '\0') {
			if (!isalpha(*value)) {
				fprintf(stderr, "%s: %zu: unexpected '%c' in "
					"keyword\n", filename, lineno, *value);
				goto fail_line;
			}
			++value;
		}

		if (*value == ' ') {
			*(value++) = '\0';
		} else {
			value = NULL;
		}

		for (i = 0; i < ARRAY_SIZE(svc_params); ++i) {
			if (!strcmp(svc_params[i].key, key))
				break;
		}

		if (i >= ARRAY_SIZE(svc_params)) {
			fprintf(stderr, "%s: %zu: unknown keyword '%s'\n",
				filename, lineno, key);
			goto fail_line;
		}

		if (value == NULL) {
			fprintf(stderr,
				"%s: %zu: expected argument after '%s'\n",
				filename, lineno, key);
			goto fail_line;
		}

		value = strdup(value);
		if (value == NULL) {
			fputs("out of memory", stderr);
			goto fail_line;
		}

		if (svc_params[i].handle(svc, value, filename, lineno)) {
			free(value);
			goto fail_line;
		}

		free(line);
	}

	close(fd);
	return svc;
fail_line:
	free(line);
fail:
	close(fd);
	delsvc(svc);
	return NULL;
}
