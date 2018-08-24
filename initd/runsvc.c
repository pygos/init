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
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "init.h"

pid_t runsvc(service_t *svc)
{
	char *argv[4], *envp[1];
	pid_t pid;

	argv[0] = (char *)RUNSVCBIN;
	argv[1] = (char *)SVCDIR;
	argv[2] = svc->fname;
	argv[3] = NULL;

	envp[0] = NULL;

	pid = fork();

	if (pid == -1)
		perror("fork");

	if (pid == 0) {
		sigreset();
		execve(argv[0], argv, envp);
		perror(argv[0]);
		exit(EXIT_FAILURE);
	}

	return pid;
}
