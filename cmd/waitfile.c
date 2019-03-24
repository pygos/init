/* SPDX-License-Identifier: ISC */
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <ctype.h>
#include <poll.h>

static int strtoui(const char *str)
{
	int i = 0;

	if (!isdigit(*str))
		return -1;

	while (isdigit(*str)) {
		if (i > (INT_MAX / 10))
			return -1;

		i = i * 10 + (*(str++)) - '0';
	}

	if (*str != '\0')
		return -1;

	return i;
}

static void sigproc(int signo)
{
	if (signo == SIGALRM) {
		fputs("waitfile timeout\n", stderr);
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char **argv)
{
	int i, found, timeout, probetime;
	struct stat sb;

	if (argc < 4)
		goto fail_usage;

	timeout = strtoui(argv[1]);
	probetime = strtoui(argv[2]);
	if (timeout < 0 || probetime < 0)
		goto fail_timeout;

	signal(SIGALRM, sigproc);
	alarm(timeout);

	for (;;) {
		found = 1;

		for (i = 3; i < argc; ++i) {
			if (stat(argv[i], &sb) != 0) {
				found = 0;
				break;
			}
		}

		if (found) {
			alarm(0);
			break;
		}

		poll(NULL, 0, probetime);
	}

	return EXIT_SUCCESS;
fail_timeout:
	fputs("Timeout values must be integers!\n", stderr);
	goto fail_usage;
fail_usage:
	fputs("Usage: waitfile <timeout secs> <probe time ms> FILES...\n",
	      stderr);
	return EXIT_FAILURE;
}
