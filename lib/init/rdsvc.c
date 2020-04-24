/* SPDX-License-Identifier: ISC */
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
#include "libcfg.h"

static int try_unescape(char *arg, rdline_t *rd)
{
	if (unescape(arg)) {
		fprintf(stderr, "%s: %zu: malformed string constant\n",
			rd->filename, rd->lineno);
		return -1;
	}
	return 0;
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

static int try_pack_argv(char *str, rdline_t *rd)
{
	int count = pack_argv(str);
	if (count < 0) {
		fprintf(stderr, "%s: %zu: malformed string constant\n",
			rd->filename, rd->lineno);
	}
	return count;
}

static int svc_desc(void *user, char *arg, rdline_t *rd, int flags)
{
	service_t *svc = user;
	(void)flags;

	if (try_unescape(arg, rd))
		return -1;
	svc->desc = try_strdup(arg, rd);
	return svc->desc == NULL ? -1 : 0;
}

static int svc_tty(void *user, char *arg, rdline_t *rd, int flags)
{
	service_t *svc = user;
	(void)flags;

	if (strncmp(arg, "truncate", 8) == 0 && isspace(arg[8])) {
		svc->flags |= SVC_FLAG_TRUNCATE_OUT;
		arg += 8;
		while (isspace(*arg))
			++arg;
	}

	if (try_unescape(arg, rd))
		return -1;

	svc->ctty = try_strdup(arg, rd);
	return svc->ctty == NULL ? -1 : 0;
}

static int svc_exec(void *user, char *arg, rdline_t *rd, int flags)
{
	service_t *svc = user;
	exec_t *e, *end;
	(void)flags;

	svc->flags |= SVC_FLAG_HAS_EXEC;

	e = calloc(1, sizeof(*e) + strlen(arg) + 1);
	if (e == NULL) {
		fprintf(stderr, "%s: %zu: out of memory\n",
			rd->filename, rd->lineno);
		return -1;
	}

	strcpy(e->args, arg);

	e->argc = try_pack_argv(e->args, rd);
	if (e->argc < 0)
		return -1;

	if (svc->exec == NULL) {
		svc->exec = e;
	} else {
		for (end = svc->exec; end->next != NULL; end = end->next)
			;
		end->next = e;
	}
	return 0;
}

static int svc_before(void *user, char *arg, rdline_t *rd, int flags)
{
	service_t *svc = user;
	(void)flags;

	if (svc->before != NULL) {
		fprintf(stderr, "%s: %zu: 'before' dependencies respecified\n",
			rd->filename, rd->lineno);
		return -1;
	}

	svc->before = try_strdup(arg, rd);
	if (svc->before == NULL)
		return -1;

	svc->num_before = try_pack_argv(svc->before, rd);
	return (svc->num_before < 0) ? -1 : 0;
}

static int svc_after(void *user, char *arg, rdline_t *rd, int flags)
{
	service_t *svc = user;
	(void)flags;

	if (svc->after != NULL) {
		fprintf(stderr, "%s: %zu: 'after' dependencies respecified\n",
			rd->filename, rd->lineno);
		return -1;
	}

	svc->after = try_strdup(arg, rd);
	if (svc->after == NULL)
		return -1;

	svc->num_after = try_pack_argv(svc->after, rd);
	return (svc->num_after < 0) ? -1 : 0;
}

static int svc_type(void *user, char *arg, rdline_t *rd, int flags)
{
	service_t *svc = user;
	int count = try_pack_argv(arg, rd);
	(void)flags;

	if (count < 1)
		return -1;

	svc->type = svc_type_from_string(arg);

	if (svc->type == -1) {
		fprintf(stderr, "%s: %zu: unknown service type '%s'\n",
			rd->filename, rd->lineno, arg);
		return -1;
	}

	if (count > 1) {
		arg += strlen(arg) + 1;

		switch (svc->type) {
		case SVC_RESPAWN:
			if (strcmp(arg, "limit") != 0)
				goto fail_limit;
			arg += strlen(arg) + 1;

			if (count > 3)
				goto fail_args;
			if (!isdigit(*arg))
				goto fail_limit;
			svc->rspwn_limit = atoi(arg);
			break;
		default:
			goto fail_args;
		}
	}

	return 0;
fail_args:
	fprintf(stderr, "%s: %zu: unexpected extra arguments\n",
		rd->filename, rd->lineno);
	return -1;
fail_limit:
	fprintf(stderr, "%s: %zu: expected 'limit <value>' after 'respawn'\n",
		rd->filename, rd->lineno);
	return -1;
}

static int svc_target(void *user, char *arg, rdline_t *rd, int flags)
{
	service_t *svc = user;
	int target;
	(void)flags;

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

static const cfg_param_t svc_params[] = {
	{ "description", 0, svc_desc },
	{ "exec", 1, svc_exec },
	{ "type", 0, svc_type },
	{ "target", 0, svc_target },
	{ "tty", 0, svc_tty },
	{ "before", 0, svc_before },
	{ "after", 0, svc_after },
};

service_t *rdsvc(int dirfd, const char *filename)
{
	const char *arg, *args[1];
	service_t *svc = NULL;
	size_t argc, nlen;
	rdline_t rd;

	arg = strchr(filename, '@');
	if (arg != NULL) {
		args[0] = arg + 1;
		argc = 1;
	} else {
		argc = 0;
	}

	if (rdline_init(&rd, dirfd, filename, argc, args))
		return NULL;

	nlen = (arg != NULL) ? (size_t)(arg - filename) : strlen(filename);

	svc = calloc(1, sizeof(*svc) + nlen + 1);
	if (svc == NULL)
		goto fail_oom;

	svc->fname = strdup(filename);
	if (svc->fname == NULL)
		goto fail_oom;

	memcpy(svc->name, filename, nlen);
	svc->id = -1;

	if (rdcfg(svc, &rd, svc_params,
		  sizeof(svc_params) / sizeof(svc_params[0]), 0)) {
		goto fail;
	}

out:
	rdline_cleanup(&rd);
	return svc;
fail_oom:
	fputs("out of memory\n", stderr);
fail:
	delsvc(svc);
	svc = NULL;
	goto out;
}
