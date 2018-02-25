#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

#include "service.h"
#include "util.h"

static const enum_map_t type_map[] = {
	{ "once", SVC_ONCE },
	{ "wait", SVC_WAIT },
	{ "respawn", SVC_RESPAWN },
	{ NULL, 0 },
};

static const enum_map_t target_map[] = {
	{ "boot", TGT_BOOT },
	{ "shutdown", TGT_SHUTDOWN },
	{ "reboot", TGT_REBOOT },
	{ "ctrlaltdel", TGT_CAD },
	{ NULL, 0 },
};

static int srv_name(service_t *srv, char *arg,
		    const char *filename, size_t lineno)
{
	(void)filename; (void)lineno;
	srv->name = arg;
	return 0;
}

static int srv_desc(service_t *srv, char *arg,
		    const char *filename, size_t lineno)
{
	(void)filename; (void)lineno;
	srv->desc = arg;
	return 0;
}

static int srv_tty(service_t *srv, char *arg,
		   const char *filename, size_t lineno)
{
	(void)filename; (void)lineno;
	srv->ctty = arg;
	return 0;
}

static int srv_exec(service_t *srv, char *arg,
		    const char *filename, size_t lineno)
{
	char **new = realloc(srv->exec, sizeof(char*) * (srv->num_exec + 1));

	if (new == NULL) {
		fprintf(stderr, "%s: %zu: out of memory\n", filename, lineno);
		free(arg);
		return -1;
	}

	srv->exec = new;
	srv->exec[srv->num_exec++] = arg;
	return 0;
}

static int srv_before(service_t *srv, char *arg,
		      const char *filename, size_t lineno)
{
	char **new = realloc(srv->before,
			     sizeof(char*) * (srv->num_before + 1));

	if (new == NULL) {
		fprintf(stderr, "%s: %zu: out of memory\n", filename, lineno);
		free(arg);
		return -1;
	}

	srv->before = new;
	srv->before[srv->num_before++] = arg;
	return 0;
}

static int srv_after(service_t *srv, char *arg,
		     const char *filename, size_t lineno)
{
	char **new = realloc(srv->after, sizeof(char*) * (srv->num_after + 1));

	if (new == NULL) {
		fprintf(stderr, "%s: %zu: out of memory\n", filename, lineno);
		free(arg);
		return -1;
	}

	srv->after = new;
	srv->after[srv->num_after++] = arg;
	return 0;
}

static int srv_type(service_t *srv, char *arg,
		    const char *filename, size_t lineno)
{
	const enum_map_t *ent = enum_by_name(type_map, arg);

	if (ent == NULL) {
		fprintf(stderr, "%s: %zu: unknown service type '%s'\n",
			filename, lineno, arg);
		return -1;
	}

	srv->type = ent->value;
	free(arg);
	return 0;
}

static int srv_target(service_t *srv, char *arg,
		      const char *filename, size_t lineno)
{
	const enum_map_t *ent = enum_by_name(target_map, arg);

	if (ent == NULL) {
		fprintf(stderr, "%s: %zu: unknown service target '%s'\n",
			filename, lineno, arg);
		return -1;
	}

	srv->target = ent->value;
	free(arg);
	return 0;
}


static const struct {
	const char *key;

	int (*handle)(service_t *srv, char *arg,
		      const char *filename, size_t lineno);
} srv_params[] = {
	{ "name", srv_name },
	{ "description", srv_desc },
	{ "exec", srv_exec },
	{ "type", srv_type },
	{ "target", srv_target },
	{ "tty", srv_tty },
	{ "before", srv_before },
	{ "after", srv_after },
};


service_t *rdsrv(int dirfd, const char *filename)
{
	const char *arg, *args[1];
	char *line, *key, *value;
	size_t i, argc, lineno;
	service_t *srv;
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

	srv = calloc(1, sizeof(*srv));
	if (srv == NULL) {
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

		for (i = 0; i < ARRAY_SIZE(srv_params); ++i) {
			if (!strcmp(srv_params[i].key, key))
				break;
		}

		if (i >= ARRAY_SIZE(srv_params)) {
			fprintf(stderr, "%s: %zu: unknown parameter '%s'\n",
				filename, lineno, key);
			goto fail_line;
		}

		value = strexpand(value, argc, args);
		if (value == NULL) {
			fputs("out of memory", stderr);
			goto fail_line;
		}

		if (srv_params[i].handle(srv, value, filename, lineno)) {
			free(value);
			goto fail_line;
		}

		free(line);
	}

	close(fd);
	return srv;
fail_line:
	free(line);
fail:
	close(fd);
	delsrv(srv);
	return NULL;
}
