/* SPDX-License-Identifier: ISC */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "servicecmd.h"

enum {
	NEED_QUOTES = 0x01,
	NEED_ESCAPE = 0x02,
};

static int check_str(const char *str)
{
	int ret = 0;

	while (*str != '\0') {
		if (isspace(*str))
			ret |= NEED_QUOTES;

		if (!isascii(*str) || !isprint(*str))
			ret |= NEED_ESCAPE | NEED_QUOTES;

		if (*str == '\\' || *str == '"' || *str == '\n')
			ret |= NEED_ESCAPE | NEED_QUOTES;

		++str;
	}

	return ret;
}

static void print_str(const char *str)
{
	int flags = check_str(str);

	if (flags & NEED_QUOTES)
		fputc('"', stdout);

	if (flags & NEED_ESCAPE) {
		while (*str != '\0') {
			switch (*str) {
			case '\a': fputs("\\a", stdout); break;
			case '\b': fputs("\\b", stdout); break;
			case '\f': fputs("\\f", stdout); break;
			case '\r': fputs("\\r", stdout); break;
			case '\t': fputs("\\t", stdout); break;
			case '\n': fputs("\\n", stdout); break;
			case '\\':
			case '"':
				fprintf(stdout, "\\%c", *str);
				break;
			default:
				if (!isascii(*str) || !isprint(*str)) {
					fprintf(stdout, "\\x%02X", *str);
				} else {
					fputc(*str, stdout);
				}
				break;
			}
			++str;
		}
	} else {
		fputs(str, stdout);
	}

	if (flags & NEED_QUOTES)
		fputc('"', stdout);
}

static int cmd_dumpscript(int argc, char **argv)
{
	char *filename, *ptr;
	service_t *svc;
	size_t len;
	exec_t *e;
	int i;

	if (check_arguments(argv[0], argc, 2, 3))
		return EXIT_FAILURE;

	for (len = 1, i = 1; i < argc; ++i)
		len += strlen(argv[i]) + 1;

	filename = alloca(len);
	filename[0] = '\0';

	for (i = 1; i < argc; ++i) {
		if (i > 1)
			strcat(filename, "@");
		strcat(filename, argv[i]);
	}

	svc = loadsvc(SVCDIR, filename);

	if (svc == NULL) {
		fprintf(stderr, "Could not load service '%s'\n", filename);
		return EXIT_FAILURE;
	}

	fprintf(stdout, "#\n# commands executed for serice '%s'\n#\n",
		filename);

	for (e = svc->exec; e != NULL; e = e->next) {
		ptr = e->args;

		for (i = 0; i < e->argc; ++i) {
			if (i)
				fputc(' ', stdout);

			print_str(ptr);
			ptr += strlen(ptr) + 1;
		}

		putchar('\n');
	}

	delsvc(svc);
	return EXIT_SUCCESS;
}

static command_t dumpscript = {
	.cmd = "dumpscript",
	.usage = "<name> [arguments]",
	.s_desc = "print commands executed for a service",
	.l_desc = "This parses a service file from " SVCDIR " and "
		  "produces a pseudo-shell-script containing the "
		  "exact commands after argument expansion that init "
		  "runs when starting the service.",
	.run_cmd = cmd_dumpscript,
};

REGISTER_COMMAND(dumpscript)
