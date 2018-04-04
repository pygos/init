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
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>

#include "init.h"

extern char **environ;

static NORETURN void split_and_exec(char *cmd)
{
	char *argv[128];
	size_t i = 0;

	while (*cmd != '\0') {
		argv[i++] = cmd;	/* FIXME: buffer overflow!! */

		if (*cmd == '"') {
			while (*cmd != '\0' && *cmd != '"') {
				if (cmd[0] == '\\' && cmd[1] != '\0')
					++cmd;

				++cmd;
			}

			if (*cmd == '"')
				*(cmd++) = '\0';

			unescape(argv[i - 1]);
		} else {
			while (*cmd != '\0' && *cmd != ' ')
				++cmd;

			if (*cmd == ' ')
				*(cmd++) = '\0';
		}

		while (*cmd == ' ')
			++cmd;
	}

	argv[i] = NULL;

	execve(argv[0], argv, environ);
	perror(argv[0]);
	exit(EXIT_FAILURE);
}

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

int runlst_wait(char **exec, size_t num, const char *ctty)
{
	pid_t ret, pid;
	int status;
	size_t i;

	for (i = 0; i < num; ++i) {
		pid = fork();

		if (pid == 0) {
			if (child_setup(ctty))
				exit(EXIT_FAILURE);
			split_and_exec(exec[i]);
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

pid_t runlst(char **exec, size_t num, const char *ctty)
{
	int status;
	pid_t pid;

	pid = fork();

	if (pid == 0) {
		if (child_setup(ctty))
			exit(EXIT_FAILURE);

		if (num > 1) {
			status = runlst_wait(exec, num, NULL);
			exit(status);
		} else {
			split_and_exec(exec[0]);
		}
	}

	if (pid == -1)
		perror("fork");

	return pid;
}
