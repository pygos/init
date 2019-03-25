/* SPDX-License-Identifier: ISC */
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>

#include "servicecmd.h"

static int cmd_disable(int argc, char **argv)
{
	int ret = EXIT_FAILURE;
	char *linkname, *ptr;
	struct stat sb;

	if (check_arguments(argv[0], argc, 2, 3))
		return EXIT_FAILURE;

	for (ptr = argv[1]; isalnum(*ptr) || *ptr == '_'; ++ptr)
		;

	if (*ptr != '\0') {
		fprintf(stderr, "Invalid service name '%s'\n", argv[1]);
		tell_read_help(argv[0]);
		return EXIT_FAILURE;
	}

	if (argc == 3) {
		ret = asprintf(&linkname, "%s/%s@%s",
				SVCDIR, argv[1], argv[2]);
	} else {
		ret = asprintf(&linkname, "%s/%s", SVCDIR, argv[1]);
	}

	if (ret < 0) {
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

	kill(1, SIGHUP);

	ret = EXIT_SUCCESS;
out:
	free(linkname);
	return ret;
}

static command_t disable = {
	.cmd = "disable",
	.usage = "<name> [argument]",
	.s_desc = "disable a service",
	.l_desc = "This disables a service by removing the coresponding "
		  "symlink in " SVCDIR ".",
	.run_cmd = cmd_disable,
};

REGISTER_COMMAND(disable)
