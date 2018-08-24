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
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>

#include <sys/reboot.h>
#include <linux/reboot.h>

#include "util.h"

#define FL_FORCE 0x01
#define FL_NOSYNC 0x02

static const struct option options[] = {
	{ "help", no_argument, NULL, 'h' },
	{ "version", no_argument, NULL, 'V' },
	{ "poweroff", no_argument, NULL, 'p' },
	{ "reboot", no_argument, NULL, 'r' },
	{ "force", no_argument, NULL, 'f' },
	{ "no-sync", no_argument, NULL, 'n' },
	{ NULL, 0, NULL, 0 },
};

static const char *shortopt = "hVprfn";

static const char *defact_str = "power-off";
static int defact = RB_POWER_OFF;

static NORETURN void usage(const char *progname, int status)
{
	fprintf(status == EXIT_SUCCESS ? stdout : stderr,
"%s [OPTIONS...]\n\n"
"Perform a system shutdown or reboot.\n\n"
"   -h, --help      Display this help text and exit.\n"
"   -V, --version   Display version information and exit.\n"
"   -p, --poweroff  Power-off the machine.\n"
"   -r, --reboot    Reboot the machine.\n"
"   -f, --force     Force immediate power-off or reboot. Do not contact the\n"
"                   init system.\n"
"   -n, --no-sync   Don't sync storage media before power-off or reboot.\n\n"
"If no option is specified, the default action is %s.\n",
	progname, defact_str);
	exit(status);
}

static NORETURN void version(const char *progname)
{
	fprintf(stdout, 
"%s (Pygos init) %s\n"
"Copyright (C) 2018 David Oberhollenzer\n"
"License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n"
"This is free software: you are free to change and redistribute it.\n"
"There is NO WARRANTY, to the extent permitted by law.\n",
	progname, PACKAGE_VERSION);

	exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
	int c, ret, flags = 0;
	char *ptr;

	ptr = strrchr(argv[0], '/');
	ptr = (ptr == NULL) ? argv[0] : (ptr + 1);

	if (strcmp(ptr, "reboot") == 0) {
		defact_str = "reboot";
		defact = RB_AUTOBOOT;
	}

	while (1) {
		c = getopt_long(argc, argv, shortopt, options, NULL);
		if (c == -1)
			break;

		switch (c) {
		case 'f':
			flags |= FL_FORCE;
			break;
		case 'n':
			flags |= FL_NOSYNC;
			break;
		case 'p':
			defact = RB_POWER_OFF;
			break;
		case 'r':
			defact = RB_AUTOBOOT;
			break;
		case 'V':
			version(ptr);
		case 'h':
			usage(ptr, EXIT_SUCCESS);
		default:
			usage(ptr, EXIT_FAILURE);
		}
	}

	if (flags & FL_FORCE) {
		if (!(flags & FL_NOSYNC))
			sync();
		reboot(defact);
		perror("reboot system call");
		return EXIT_FAILURE;
	}

	switch (defact) {
	case RB_AUTOBOOT:
		ret = kill(1, SIGINT);
		break;
	case RB_POWER_OFF:
		ret = kill(1, SIGTERM);
		break;
	default:
		return EXIT_SUCCESS;
	}

	if (ret) {
		perror("sending signal to init");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
