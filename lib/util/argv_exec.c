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
#include "service.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>

int setup_tty(const char *tty, bool truncate)
{
	int fd;

	if (tty == NULL)
		return 0;

	fd = open(tty, O_RDWR);
	if (fd < 0) {
		perror(tty);
		return -1;
	}

	if (truncate)
		ftruncate(fd, 0);

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	setsid();

	dup2(fd, STDIN_FILENO);
	dup2(fd, STDOUT_FILENO);
	dup2(fd, STDERR_FILENO);
	close(fd);
	return 0;
}

void argv_exec(exec_t *e)
{
	char **argv = alloca(sizeof(char *) * (e->argc + 1)), *ptr;
	int i;

	for (ptr = e->args, i = 0; i < e->argc; ++i, ptr += strlen(ptr) + 1)
		argv[i] = ptr;

	argv[i] = NULL;
	execvp(argv[0], argv);
	perror(argv[0]);
	exit(EXIT_FAILURE);
}
