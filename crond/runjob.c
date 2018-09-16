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
#include "gcrond.h"

int runjob(crontab_t *tab)
{
	struct sigaction act;
	pid_t pid;
	exec_t *e;
	int ret;

	if (tab->exec == NULL)
		return 0;

	pid = fork();
	if (pid == -1) {
		perror("fork");
		return -1;
	}

	if (pid != 0) {
		tab->pid = pid;
		return 0;
	}

	/* XXX: inside the child process */
	memset(&act, 0, sizeof(act));
	act.sa_handler = SIG_DFL;
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGHUP, &act, NULL);

	if (setup_tty(tab->ctty, tab->tty_truncate))
		exit(EXIT_FAILURE);

	if (tab->gid != 0) {
		if (setresgid(tab->gid, tab->gid, tab->gid)) {
			perror("setgid");
			exit(EXIT_FAILURE);
		}
	}

	if (tab->uid != 0) {
		if (setresuid(tab->uid, tab->uid, tab->uid)) {
			perror("setuid");
			exit(EXIT_FAILURE);
		}
	}

	if (tab->exec->next == NULL)
		argv_exec(tab->exec);

	for (e = tab->exec; e != NULL; e = e->next) {
		pid = fork();
		if (pid == -1) {
			perror("fork");
			exit(EXIT_FAILURE);
		}

		if (pid == 0)
			argv_exec(e);

		while (waitpid(pid, &ret, 0) != pid)
			;

		ret = WIFEXITED(ret) ? WEXITSTATUS(ret) : EXIT_FAILURE;
		if (ret != EXIT_SUCCESS)
			break;
	}

	exit(ret);
}
