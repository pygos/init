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

static int cmd_schedule(int argc, char **argv)
{
	char *target, *linkname, *ptr;
	int ret = EXIT_FAILURE;
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

	if (asprintf(&target, "%s/%s.gcron", TEMPLATEDIR, argv[1]) < 0) {
		perror("asprintf");
		return EXIT_FAILURE;
	}

	if (stat(target, &sb)) {
		fprintf(stderr, "%s: %s\n", target, strerror(errno));
		goto out_tgt;
	}

	if ((sb.st_mode & S_IFMT) != S_IFREG) {
		fprintf(stderr, "%s: must be a regular file\n", target);
		goto out_tgt;
	}

	if (asprintf(&linkname, "%s/%s", GCRONDIR, argv[1]) < 0) {
		perror("asprintf");
		goto out_tgt;
	}

	if (symlink(target, linkname)) {
		fprintf(stderr, "creating symlink '%s' -> '%s: %s\n",
			linkname, target, strerror(errno));
		goto out;
	}

	ret = EXIT_SUCCESS;
out:
	free(linkname);
out_tgt:
	free(target);
	return ret;
}

static command_t schedule = {
	.cmd = "schedule",
	.usage = "<name>",
	.s_desc = "enable a gcrond service",
	.l_desc = "This marks a gcrond service as enabled by creating a "
		  "symlink in " GCRONDIR " pointing to a template file in "
		  TEMPLATEDIR " with a .gcron extension.",
	.run_cmd = cmd_schedule,
};

REGISTER_COMMAND(schedule)