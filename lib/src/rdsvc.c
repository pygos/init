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

static int svc_name(service_t *svc, char *arg,
		    const char *filename, size_t lineno)
{
	(void)filename; (void)lineno;
	svc->name = arg;
	return 0;
}

static int svc_desc(service_t *svc, char *arg,
		    const char *filename, size_t lineno)
{
	(void)filename; (void)lineno;
	svc->desc = arg;
	return 0;
}

static int svc_tty(service_t *svc, char *arg,
		   const char *filename, size_t lineno)
{
	(void)filename; (void)lineno;
	svc->ctty = arg;
	return 0;
}

static int svc_exec(service_t *svc, char *arg,
		    const char *filename, size_t lineno)
{
	char **new = realloc(svc->exec, sizeof(char*) * (svc->num_exec + 1));

	if (new == NULL) {
		fprintf(stderr, "%s: %zu: out of memory\n", filename, lineno);
		free(arg);
		return -1;
	}

	svc->exec = new;
	svc->exec[svc->num_exec++] = arg;
	return 0;
}

static int svc_before(service_t *svc, char *arg,
		      const char *filename, size_t lineno)
{
	char **new = realloc(svc->before,
			     sizeof(char*) * (svc->num_before + 1));

	if (new == NULL) {
		fprintf(stderr, "%s: %zu: out of memory\n", filename, lineno);
		free(arg);
		return -1;
	}

	svc->before = new;
	svc->before[svc->num_before++] = arg;
	return 0;
}

static int svc_after(service_t *svc, char *arg,
		     const char *filename, size_t lineno)
{
	char **new = realloc(svc->after, sizeof(char*) * (svc->num_after + 1));

	if (new == NULL) {
		fprintf(stderr, "%s: %zu: out of memory\n", filename, lineno);
		free(arg);
		return -1;
	}

	svc->after = new;
	svc->after[svc->num_after++] = arg;
	return 0;
}

static int svc_type(service_t *svc, char *arg,
		    const char *filename, size_t lineno)
{
	int type = svc_type_from_string(arg);

	if (type == -1) {
		fprintf(stderr, "%s: %zu: unknown service type '%s'\n",
			filename, lineno, arg);
		return -1;
	}

	svc->type = type;
	free(arg);
	return 0;
}

static int svc_target(service_t *svc, char *arg,
		      const char *filename, size_t lineno)
{
	int target = svc_target_from_string(arg);

	if (target == -1) {
		fprintf(stderr, "%s: %zu: unknown service target '%s'\n",
			filename, lineno, arg);
		return -1;
	}

	svc->target = target;
	free(arg);
	return 0;
}

static int svc_rspwn_limit(service_t *svc, char *arg,
			   const char *filename, size_t lineno)
{
	const char *ptr;

	svc->rspwn_limit = 0;

	if (!isdigit(*arg))
		goto fail;

	for (ptr = arg; isdigit(*ptr); ++ptr)
		svc->rspwn_limit = svc->rspwn_limit * 10 + (*ptr - '0');

	if (*ptr != '\0')
		goto fail;

	free(arg);
	return 0;
fail:
	fprintf(stderr,
		"%s: %zu: expected numeric argument for respawn limit\n",
		filename, lineno);
	free(arg);
	return -1;
}


static const struct {
	const char *key;

	int (*handle)(service_t *svc, char *arg,
		      const char *filename, size_t lineno);
} svc_params[] = {
	{ "name", svc_name },
	{ "description", svc_desc },
	{ "exec", svc_exec },
	{ "type", svc_type },
	{ "target", svc_target },
	{ "tty", svc_tty },
	{ "before", svc_before },
	{ "after", svc_after },
	{ "respawn_limit", svc_rspwn_limit },
};


service_t *rdsvc(int dirfd, const char *filename)
{
	const char *arg, *args[1];
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

	for (lineno = 1; ; ++lineno) {
		errno = 0;
		line = rdline(fd);

		if (line == NULL) {
			if (errno != 0) {
				fprintf(stderr, "read: %s: %zu: %s\n",
					filename, lineno, strerror(errno));
				goto fail;
			}
			break;
		}

		if (splitkv(line, &key, &value)) {
			if (key == NULL) {
				fprintf(stderr,
					"%s: %zu: expected <key> = <value>\n",
					filename, lineno);
			} else if (value == NULL) {
				fprintf(stderr,
					"%s: %zu: expected value after %s\n",
					filename, lineno, key);
			} else {
				fprintf(stderr,
					"%s: %zu: unexpected arguments "
					"after key-value pair\n",
					filename, lineno);
			}
			goto fail;
		}

		if (key == NULL) {
			free(line);
			continue;
		}

		for (i = 0; i < ARRAY_SIZE(svc_params); ++i) {
			if (!strcmp(svc_params[i].key, key))
				break;
		}

		if (i >= ARRAY_SIZE(svc_params)) {
			fprintf(stderr, "%s: %zu: unknown parameter '%s'\n",
				filename, lineno, key);
			goto fail_line;
		}

		value = strexpand(value, argc, args);
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
