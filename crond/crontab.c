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
#include <string.h>

#include "crontab.h"

void cron_tm_to_mask(crontab_t *out, struct tm *t)
{
	memset(out, 0, sizeof(*out));
	out->minute     = 1UL << ((unsigned long)t->tm_min);
	out->hour       = 1 << t->tm_hour;
	out->dayofmonth = 1 << (t->tm_mday - 1);
	out->month      = 1 << t->tm_mon;
	out->dayofweek  = 1 << t->tm_wday;
}

bool cron_should_run(const crontab_t *t, const crontab_t *mask)
{
	if ((t->minute & mask->minute) == 0)
		return false;

	if ((t->hour & mask->hour) == 0)
		return false;

	if ((t->dayofmonth & mask->dayofmonth) == 0)
		return false;

	if ((t->month & mask->month) == 0)
		return false;

	if ((t->dayofweek & mask->dayofweek) == 0)
		return false;

	return true;
}
