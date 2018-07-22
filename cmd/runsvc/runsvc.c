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

static int setup_tty(service_t *svc)
{
	int fd;

	if (svc->ctty != NULL) {
		fd = open(svc->ctty, O_RDWR);
		if (fd < 0) {
			perror(svc->ctty);
			return -1;
		}

		if (svc->flags & SVC_FLAG_TRUNCATE_OUT)
			ftruncate(fd, 0);

		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);

		setsid();

		dup2(fd, STDIN_FILENO);
		dup2(fd, STDOUT_FILENO);
		dup2(fd, STDERR_FILENO);
		close(fd);
	}

	return 0;
}

/*****************************************************************************/

static NORETURN void argv_exec(exec_t *e)
{
	char **argv = alloca(e->argc + 1), *ptr;
	int i;

	for (ptr = e->args, i = 0; i < e->argc; ++i, ptr += strlen(ptr) + 1)
		argv[i] = ptr;

	argv[i] = NULL;
	execvp(argv[0], argv);
	perror(argv[0]);
	exit(EXIT_FAILURE);
}

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

	if (setup_tty(svc))
		goto out;

	if (svc->exec->next == NULL)
		argv_exec(svc->exec);

	ret = runlst_wait(svc->exec);
out:
	delsvc(svc);
	return ret;
}
