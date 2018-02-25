#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#include <sys/reboot.h>
#include <linux/reboot.h>

#include "telinit.h"
#include "util.h"

#define STRINIFY(x) #x
#define STRINIFY_VALUE(x) STRINIFY(x)
#define PROGRAM_NAME STRINIFY_VALUE(PROGNAME)

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
static int defact = TI_SHUTDOWN;

static NORETURN void usage(int status)
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
	PROGRAM_NAME, defact_str);
	exit(status);
}

static NORETURN void version(void)
{
	fputs(
PROGRAM_NAME " (Pygos init) " PACKAGE_VERSION "\n"
"Copyright (C) 2018 David Oberhollenzer\n"
"License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n"
"This is free software: you are free to change and redistribute it.\n"
"There is NO WARRANTY, to the extent permitted by law.\n",
	stdout);

	exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
	int c, fd, flags = 0;
	ti_msg_t msg;
	ssize_t ret;

	if (!strcmp(PROGRAM_NAME, "reboot")) {
		defact_str = "reboot";
		defact = TI_REBOOT;
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
			defact = TI_SHUTDOWN;
			break;
		case 'r':
			defact = TI_REBOOT;
			break;
		case 'V':
			version();
		case 'h':
			usage(EXIT_SUCCESS);
		default:
			exit(EXIT_FAILURE);
		}
	}

	if (flags & FL_FORCE) {
		if (!(flags & FL_NOSYNC))
			sync();

		switch (defact) {
		case TI_REBOOT:
			reboot(RB_AUTOBOOT);
			break;
		case TI_SHUTDOWN:
			reboot(RB_POWER_OFF);
			break;
		}

		perror("reboot system call");
		return EXIT_FAILURE;
	}

	fd = opensock();
	if (fd < 0)
		return EXIT_FAILURE;

	msg.type = defact;
retry:
	ret = write(fd, &msg, sizeof(msg));

	if (ret < 0) {
		if (errno == EINTR)
			goto retry;
		perror("write on init socket");
		close(fd);
		return EXIT_FAILURE;
	}

	close(fd);
	return EXIT_SUCCESS;
}
