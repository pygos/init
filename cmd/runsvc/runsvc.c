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
#include "runsvc.h"

static int runlst_wait(exec_t *list)
{
	pid_t ret, pid;
	int status;

	for (; list != NULL; list = list->next) {
		pid = fork();

		if (pid == 0)
			argv_exec(list);

		if (pid == -1) {
			perror("fork");
			return EXIT_FAILURE;
		}

		do {
			ret = waitpid(pid, &status, 0);
		} while (ret != pid);

		if (!WIFEXITED(status))
			return EXIT_FAILURE;

		if (WEXITSTATUS(status) != EXIT_SUCCESS)
			return WEXITSTATUS(status);
	}

	return EXIT_SUCCESS;
}

/*****************************************************************************/

int main(int argc, char **argv)
{
	int dirfd, ret = EXIT_FAILURE;
	service_t *svc = NULL;

	if (argc != 3) {
		fputs("usage: runsvc <directory> <filename>\n", stderr);
		goto out;
	}

	if (getppid() != 1) {
		fputs("must be run by init!\n", stderr);
		goto out;
	}

	dirfd = open(argv[1], O_RDONLY | O_DIRECTORY);
	if (dirfd < 0) {
		perror(argv[1]);
		goto out;
	}

	svc = rdsvc(dirfd, argv[2], RDSVC_NO_FNAME | RDSVC_NO_DEPS);
	close(dirfd);
	if (svc == NULL)
		goto out;

	if (svc->exec == NULL) {
		ret = EXIT_SUCCESS;
		goto out;
	}

	if (initenv())
		goto out;

	if (setup_tty(svc->ctty, (svc->flags & SVC_FLAG_TRUNCATE_OUT) != 0))
		goto out;

	if (svc->exec->next == NULL)
		argv_exec(svc->exec);

	ret = runlst_wait(svc->exec);
out:
	delsvc(svc);
	return ret;
}
