/* SPDX-License-Identifier: ISC */
#include <stdlib.h>
#include <stdio.h>

#include "config.h"
#include "util.h"

static const char *version_string =
"%s (pygos init) " PACKAGE_VERSION "\n"
"Copyright (C) 2018 David Oberhollenzer\n\n"
"This is free software: you are free to change and redistribute it.\n"
"There is NO WARRANTY, to the extent permitted by law.\n";

void print_version(const char *program)
{
	fprintf(stdout, version_string, program);
	exit(EXIT_SUCCESS);
}
