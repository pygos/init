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
#include <fcntl.h>
#include <errno.h>

#include "servicecmd.h"
#include "service.h"

static service_t *try_load(const char *directory, const char *filename)
{
	int dirfd, type;
	struct stat sb;
	service_t *svc;

	dirfd = open(directory, O_RDONLY | O_DIRECTORY);

	if (dirfd < 0) {
		perror(directory);
		return NULL;
	}

	if (fstatat(dirfd, filename, &sb, AT_SYMLINK_NOFOLLOW)) {
		fprintf(stderr, "stat %s/%s: %s\n",
			directory, filename, strerror(errno));
		close(dirfd);
		return NULL;
	}

	type = (sb.st_mode & S_IFMT);

	if (type != S_IFREG && type != S_IFLNK)
		return NULL;

	svc = rdsvc(dirfd, filename);
	close(dirfd);
	return svc;
}

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

	svc = try_load(SVCDIR, filename);

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