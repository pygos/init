#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "servicecmd.h"

static int cmd_enable(int argc, char **argv)
{
	char *target, *linkname, *ptr;
	int ret = EXIT_FAILURE;
	struct stat sb;

	if (argc < 2 || argc > 3) {
		fputs("Wrong number of arguments for `enable'.\n"
			"Try `service help enable' for more information.\n",
			stderr);
		return EXIT_FAILURE;
	}

	for (ptr = argv[1]; isalnum(*ptr) || *ptr == '_'; ++ptr)
		;

	if (*ptr != '\0') {
		fprintf(stderr, "Invalid service name '%s'\n", argv[1]);
		return EXIT_FAILURE;
	}

	if (asprintf(&target, "%s/%s", TEMPLATEDIR, argv[1]) < 0) {
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

	if (argc == 3) {
		ret = asprintf(&linkname, "%s/%s@%s",
				SVCDIR, argv[1], argv[2]);
	} else {
		ret = asprintf(&linkname, "%s/%s", SVCDIR, argv[1]);
	}

	if (ret < 0) {
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

static command_t enable = {
	.cmd = "enable",
	.usage = "<name> [argument]",
	.s_desc = "enable a service",
	.l_desc = "This marks a service as enabled by creating a symlink in "
		  SVCDIR " pointing to the template file in " TEMPLATEDIR ". "
		  "An optional argument can be supplied to parameterize the "
		  "template.",
	.run_cmd = cmd_enable,
};

REGISTER_COMMAND(enable)
