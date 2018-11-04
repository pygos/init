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
#ifndef CRONTAB_H
#define CRONTAB_H

#include <sys/types.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "service.h"

typedef struct crontab_t {
	struct crontab_t *next;
	exec_t *exec;
	char *ctty;

	uid_t uid;
	gid_t gid;

	uint64_t minute;
	uint32_t hour;
	uint32_t dayofmonth;
	uint16_t month;
	uint8_t dayofweek;

	unsigned int tty_truncate : 1;
} crontab_t;

crontab_t *rdcron(int dirfd, const char *filename);

void delcron(crontab_t *cron);

int cronscan(const char *directory, crontab_t **list);

void cron_tm_to_mask(crontab_t *out, struct tm *t);

bool cron_should_run(const crontab_t *t, const crontab_t *mask);

#endif /* CRONTAB_H */
