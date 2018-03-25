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
#include <dirent.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>

#include "util.h"

static NORETURN void usage_and_exit(void)
{
	fputs("Usage: killall5 SIGNAL\n", stderr);
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	int signo, ret = EXIT_SUCCESS;
	pid_t pid, mypid, mysid;
	struct dirent *ent;
	const char *ptr;
	DIR *dir;

	if (argc != 2)
		usage_and_exit();

	ptr = argv[1];
	if (*ptr == '-')
		++ptr;

	if (!isdigit(*ptr))
		usage_and_exit();

	for (signo = 0; isdigit(*ptr); ++ptr)
		signo = signo * 10 + ((*ptr) - '0');

	if (*ptr != '\0' || signo < 1 || signo > 31)
		usage_and_exit();

	kill(-1, SIGSTOP);

	dir = opendir("/proc");
	if (dir == NULL) {
		perror("opendir /proc");
		ret = EXIT_FAILURE;
		goto out;
	}

	mypid = getpid();
	mysid = getsid(0);

	for (;;) {
		errno = 0;
		ent = readdir(dir);

		if (ent == NULL) {
			if (errno) {
				perror("readdir");
				ret = EXIT_FAILURE;
			}
			break;
		}

		if (!isdigit(ent->d_name[0]))
			continue;

		for (pid = 0, ptr = ent->d_name; isdigit(*ptr); ++ptr)
			pid = pid * 10 + ((*ptr) - '0');

		if (*ptr != '\0' || pid == mypid || getsid(pid) == mysid)
			continue;

		if (kill(pid, signo)) {
			ret = EXIT_FAILURE;
			fprintf(stderr, "kill %d: %s\n",
					(int)pid, strerror(errno));
		}
	}
out:
	kill(-1, SIGCONT);
	return ret;
}
