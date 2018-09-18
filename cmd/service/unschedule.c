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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "servicecmd.h"

static int cmd_unschedule(int argc, char **argv)
{
	int ret = EXIT_FAILURE;
	char *linkname, *ptr;
	struct stat sb;

	if (check_arguments(argv[0], argc, 2, 2))
		return EXIT_FAILURE;

	for (ptr = argv[1]; isalnum(*ptr) || *ptr == '_'; ++ptr)
		;

	if (*ptr != '\0') {
		fprintf(stderr, "Invalid service name '%s'\n", argv[1]);
		tell_read_help(argv[0]);
		return EXIT_FAILURE;
	}

	if (asprintf(&linkname, "%s/%s.gcron", GCRONDIR, argv[1]) < 0) {
		perror("asprintf");
		return EXIT_FAILURE;
	}

	if (lstat(linkname, &sb)) {
		fprintf(stderr, "lstat %s: %s\n", linkname, strerror(errno));
		goto out;
	}

	if ((sb.st_mode & S_IFMT) != S_IFLNK) {
		fprintf(stderr, "error: '%s' is not a symlink!", linkname);
		goto out;
	}

	if (unlink(linkname)) {
		fprintf(stderr, "removing %s: %s\n",
			linkname, strerror(errno));
		goto out;
	}

	ret = EXIT_SUCCESS;
out:
	free(linkname);
	return ret;
}

static command_t unschedule = {
	.cmd = "unschedule",
	.usage = "<name>",
	.s_desc = "disable a gcrond service",
	.l_desc = "This disables a gcrond service by removing the coresponding "
		  "symlink in " GCRONDIR ".",
	.run_cmd = cmd_unschedule,
};

REGISTER_COMMAND(unschedule)
