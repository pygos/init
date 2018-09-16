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
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <pwd.h>
#include <grp.h>

#include "crontab.h"
#include "libcfg.h"
#include "util.h"


static const enum_map_t weekday[] = {
	{ "MON", 1 },
	{ "TUE", 2 },
	{ "WED", 3 },
	{ "THU", 4 },
	{ "FRI", 5 },
	{ "SAT", 6 },
	{ "SUN", 0 },
};

static const enum_map_t month[] = {
	{ "JAN", 1 },
	{ "FEB", 2 },
	{ "MAR", 3 },
	{ "APR", 4 },
	{ "MAY", 5 },
	{ "JUN", 6 },
	{ "JUL", 7 },
	{ "AUG", 8 },
	{ "SEP", 9 },
	{ "OCT", 10 },
	{ "NOV", 11 },
	{ "DEC", 12 },
};

static const struct {
	const char *macro;
	crontab_t tab;
} intervals[] = {
	{
		.macro = "yearly",
		.tab = {
			.minute = 0x01,
			.hour = 0x01,
			.dayofmonth = 0x01,
			.month = 0x01,
			.dayofweek = 0xFF
		},
	}, {
		.macro = "annually",
		.tab = {
			.minute = 0x01,
			.hour = 0x01,
			.dayofmonth = 0x01,
			.month = 0x01,
			.dayofweek = 0xFF
		},
	}, {
		.macro = "monthly",
		.tab = {
			.minute = 0x01,
			.hour = 0x01,
			.dayofmonth = 0x01,
			.month = 0xFFFF,
			.dayofweek = 0xFF
		},
	}, {
		.macro = "weekly",
		.tab = {
			.minute = 0x01,
			.hour = 0x01,
			.dayofmonth = 0xFFFFFFFF,
			.month = 0xFFFF,
			.dayofweek = 0x01
		},
	}, {
		.macro = "daily",
		.tab = {
			.minute = 0x01,
			.hour = 0x01,
			.dayofmonth = 0xFFFFFFFF,
			.month = 0xFFFF,
			.dayofweek = 0xFF
		},
	}, {
		.macro = "hourly",
		.tab = {
			.minute = 0x01,
			.hour = 0xFFFFFFFF,
			.dayofmonth = 0xFFFFFFFF,
			.month = 0xFFFF,
			.dayofweek = 0xFF
		},
	},
};

/*****************************************************************************/

static int try_unescape(char *arg, rdline_t *rd)
{
	if (unescape(arg)) {
		fprintf(stderr, "%s: %zu: malformed string constant\n",
			rd->filename, rd->lineno);
		return -1;
	}
	return 0;
}

static char *try_strdup(const char *str, rdline_t *rd)
{
	char *out = strdup(str);

	if (out == NULL) {
		fprintf(stderr, "%s: %zu: out of memory\n",
			rd->filename, rd->lineno);
	}
	return out;
}

static char *readnum(char *line, int *out, int minval, int maxval,
		     const enum_map_t *mnemonic, rdline_t *rd)
{
	int i, temp, value = 0;
	const enum_map_t *ev;

	if (!isdigit(*line)) {
		if (!mnemonic)
			goto fail_mn;

		for (i = 0; isalnum(line[i]); ++i)
			;
		if (i == 0)
			goto fail_mn;

		temp = line[i];
		line[i] = '\0';
		ev = enum_by_name(mnemonic, line);
		if (!ev) {
			fprintf(stderr, "%s: %zu: unexpected '%s'",
				rd->filename, rd->lineno, line);
		}
		line[i] = temp;
		if (!ev)
			return NULL;
		*out = ev->value;
		return line + i;
	}

	while (isdigit(*line)) {
		i = ((*(line++)) - '0');
		if (value > (maxval - i) / 10)
			goto fail_of;
		value = value * 10 + i;
	}

	if (value < minval)
		goto fail_uf;

	*out = value;
	return line;
fail_of:
	fprintf(stderr, "%s: %zu: value exceeds maximum (%d > %d)\n",
		rd->filename, rd->lineno, value, maxval);
	return NULL;
fail_uf:
	fprintf(stderr, "%s: %zu: value too small (%d < %d)\n",
		rd->filename, rd->lineno, value, minval);
	return NULL;
fail_mn:
	fprintf(stderr, "%s: %zu: expected numeric value",
		rd->filename, rd->lineno);
	return NULL;
}

static char *readfield(char *line, uint64_t *out, int minval, int maxval,
		       const enum_map_t *mnemonic, rdline_t *rd)
{
	int value, endvalue, step;
	uint64_t v = 0;
next:
	if (*line == '*') {
		++line;
		value = minval;
		endvalue = maxval;
	} else {
		line = readnum(line, &value, minval, maxval, mnemonic, rd);
		if (!line)
			goto fail;

		if (*line == '-') {
			line = readnum(line + 1, &endvalue, minval, maxval,
				       mnemonic, rd);
			if (!line)
				goto fail;
		} else {
			endvalue = value;
		}
	}

	if (endvalue < value)
		goto fail;

	if (*line == '/') {
		line = readnum(line + 1, &step, 1, maxval + 1, NULL, rd);
		if (!line)
			goto fail;
	} else {
		step = 1;
	}

	while (value <= endvalue) {
		v |= 1UL << (unsigned long)(value - minval);
		value += step;
	}

	if (*line == ',' || *line == ' ') {
		++line;
		goto next;
	}

	if (*line != '\0')
		goto fail;

	*out = v;
	return line;
fail:
	fprintf(stderr, "%s: %zu: invalid time range expression\n",
		rd->filename, rd->lineno);
	return NULL;
}

/*****************************************************************************/

static int cron_exec(void *user, char *arg, rdline_t *rd, int flags)
{
	crontab_t *cron = user;
	exec_t *e, *end;
	(void)flags;

	e = calloc(1, sizeof(*e) + strlen(arg) + 1);
	if (e == NULL) {
		fprintf(stderr, "%s: %zu: out of memory\n",
			rd->filename, rd->lineno);
		return -1;
	}

	strcpy(e->args, arg);

	e->argc = pack_argv(e->args);
	if (e->argc < 0) {
		fprintf(stderr, "%s: %zu: malformed string constant\n",
			rd->filename, rd->lineno);
		return -1;
	}

	if (cron->exec == NULL) {
		cron->exec = e;
	} else {
		for (end = cron->exec; end->next != NULL; end = end->next)
			;
		end->next = e;
	}
	return 0;
}

static int cron_hour(void *user, char *arg, rdline_t *rd, int flags)
{
	crontab_t *cron = user;
	uint64_t value;
	(void)flags;

	if (!readfield(arg, &value, 0, 23, NULL, rd))
		return -1;

	cron->hour = value;
	return 0;
}

static int cron_minute(void *user, char *arg, rdline_t *rd, int flags)
{
	crontab_t *cron = user;
	uint64_t value;
	(void)flags;

	if (!readfield(arg, &value, 0, 59, NULL, rd))
		return -1;

	cron->minute = value;
	return 0;
}

static int cron_dayofmonth(void *user, char *arg, rdline_t *rd, int flags)
{
	crontab_t *cron = user;
	uint64_t value;
	(void)flags;

	if (!readfield(arg, &value, 1, 31, NULL, rd))
		return -1;

	cron->dayofmonth = value;
	return 0;
}

static int cron_dayofweek(void *user, char *arg, rdline_t *rd, int flags)
{
	crontab_t *cron = user;
	uint64_t value;
	(void)flags;

	if (!readfield(arg, &value, 0, 6, weekday, rd))
		return -1;

	cron->dayofweek = value;
	return 0;
}

static int cron_month(void *user, char *arg, rdline_t *rd, int flags)
{
	crontab_t *cron = user;
	uint64_t value;
	(void)flags;

	if (!readfield(arg, &value, 1, 12, month, rd))
		return -1;

	cron->month = value;
	return 0;
}

static int cron_interval(void *user, char *arg, rdline_t *rd, int flags)
{
	crontab_t *cron = user;
	size_t i;
	(void)flags;

	for (i = 0; i < ARRAY_SIZE(intervals); ++i) {
		if (!strcmp(intervals[i].macro, arg)) {
			cron->minute = intervals[i].tab.minute;
			cron->hour = intervals[i].tab.hour;
			cron->dayofmonth = intervals[i].tab.dayofmonth;
			cron->month = intervals[i].tab.month;
			cron->dayofweek = intervals[i].tab.dayofweek;
			return 0;
		}
	}

	fprintf(stderr, "%s: %zu: unknown interval '%s'\n",
		rd->filename, rd->lineno, arg);
	return -1;
}

static int cron_user(void *user, char *arg, rdline_t *rd, int flags)
{
	crontab_t *cron = user;
	struct passwd *pwd;
	bool isnumeric;
	char *ptr;
	int value;
	(void)flags;

	for (ptr = arg; isdigit(*ptr); ++ptr)
		;

	isnumeric = (*ptr == '\0');
	pwd = getpwnam(arg);

	if (pwd == NULL && !isnumeric) {
		fprintf(stderr, "%s: %zu: unknown user '%s'\n",
			rd->filename, rd->lineno, arg);
		return -1;
	}

	if (pwd != NULL) {
		cron->uid = pwd->pw_uid;
	} else {
		if (readnum(arg, &value, 0, INT_MAX, NULL, rd))
			return -1;
		cron->uid = value;
	}
	return 0;
}

static int cron_group(void *user, char *arg, rdline_t *rd, int flags)
{
	crontab_t *cron = user;
	struct group *group;
	bool isnumeric;
	char *ptr;
	int value;
	(void)flags;

	for (ptr = arg; isdigit(*ptr); ++ptr)
		;

	isnumeric = (*ptr == '\0');
	group = getgrnam(arg);

	if (group == NULL && !isnumeric) {
		fprintf(stderr, "%s: %zu: unknown group '%s'\n",
			rd->filename, rd->lineno, arg);
		return -1;
	}

	if (group != NULL) {
		cron->gid = group->gr_gid;
	} else {
		if (readnum(arg, &value, 0, INT_MAX, NULL, rd))
			return -1;
		cron->gid = value;
	}
	return 0;
}

static int cron_tty(void *user, char *arg, rdline_t *rd, int flags)
{
	crontab_t *cron = user;
	(void)flags;

	if (strncmp(arg, "truncate", 8) == 0 && isspace(arg[8])) {
		cron->tty_truncate = 1;
		arg += 8;
		while (isspace(*arg))
			++arg;
	}

	if (try_unescape(arg, rd))
		return -1;

	cron->ctty = try_strdup(arg, rd);
	return cron->ctty == NULL ? -1 : 0;
}



static const cfg_param_t cron_params[] = {
	{ "hour", 0, cron_hour },
	{ "minute", 0, cron_minute },
	{ "dayofmonth", 0, cron_dayofmonth },
	{ "dayofweek", 0, cron_dayofweek },
	{ "month", 0, cron_month },
	{ "interval", 0, cron_interval },
	{ "user", 0, cron_user },
	{ "group", 0, cron_group },
	{ "tty", 0, cron_tty },
	{ "exec", 1, cron_exec },
};

crontab_t *rdcron(int dirfd, const char *filename)
{
	crontab_t *cron;
	rdline_t rd;
	int fd, ret;

	fd = openat(dirfd, filename, O_RDONLY);
	if (fd < 0) {
		perror(filename);
		return NULL;
	}

	cron = calloc(1, sizeof(*cron));
	if (cron == NULL) {
		fputs("out of memory\n", stderr);
		goto out;
	}

	cron->pid = -1;

	rdline_init(&rd, fd, filename, 0, NULL);
	ret = rdcfg(cron, &rd, cron_params, ARRAY_SIZE(cron_params), 0);
	if (ret) {
		delcron(cron);
		cron = NULL;
	}
out:
	close(fd);
	return cron;
}
