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
#include <sys/stat.h>
#include <stddef.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>

#include "crontab.h"

int cronscan(const char *directory, crontab_t **list)
{
	struct dirent *ent;
	int dfd, ret = 0;
	crontab_t *cron;
	DIR *dir;

	dir = opendir(directory);
	if (dir == NULL) {
		perror(directory);
		return -1;
	}

	dfd = dirfd(dir);
	if (dfd < 0) {
		perror(directory);
		closedir(dir);
		return -1;
	}

	for (;;) {
		errno = 0;
		ent = readdir(dir);

		if (ent == NULL) {
			if (errno != 0) {
				perror(directory);
				ret = -1;
			}
			break;
		}

		if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
			continue;

		cron = rdcron(dfd, ent->d_name);
		if (cron == NULL) {
			ret = -1;
			continue;
		}

		cron->next = *list;
		*list = cron;
	}

	closedir(dir);
	return ret;
}
