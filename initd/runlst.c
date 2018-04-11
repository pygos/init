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
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

#include "init.h"

extern char **environ;

static int child_setup(const char *ctty)
{
	sigset_t mask;
	int fd;

	sigemptyset(&mask);
	sigprocmask(SIG_SETMASK, &mask, NULL);

	if (ctty != NULL) {
		fd = open(ctty, O_RDWR);
		if (fd < 0) {
			perror(ctty);
			return -1;
		}

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

static NORETURN void argv_exec(exec_t *e)
{
	char **argv = alloca(e->argc + 1), *ptr;
	int i;

	for (ptr = e->args, i = 0; i < e->argc; ++i, ptr += strlen(ptr) + 1)
		argv[i] = ptr;

	argv[i] = NULL;
	execve(argv[0], argv, environ);
	perror(argv[0]);
	exit(EXIT_FAILURE);
}

int runlst_wait(exec_t *list, const char *ctty)
{
	pid_t ret, pid;
	int status;

	for (; list != NULL; list = list->next) {
		pid = fork();

		if (pid == 0) {
			if (child_setup(ctty))
				exit(EXIT_FAILURE);
			argv_exec(list);
		}

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

pid_t runlst(exec_t *list, const char *ctty)
{
	pid_t pid = fork();

	if (pid == 0) {
		if (child_setup(ctty))
			exit(EXIT_FAILURE);

		if (list->next != NULL)
			exit(runlst_wait(list, NULL));

		argv_exec(list);
	}

	if (pid == -1)
		perror("fork");

	return pid;
}
