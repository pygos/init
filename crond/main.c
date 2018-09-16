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

static crontab_t *jobs;
static sig_atomic_t run = 1;
static sig_atomic_t rescan = 1;

static void read_config(void)
{
	if (cronscan(GCRONDIR, &jobs)) {
		fputs("Error reading configuration. Continuing anyway.\n",
		      stderr);
	}
}

static void cleanup_config(void)
{
	crontab_t *t;

	while (jobs != NULL) {
		t = jobs;
		jobs = jobs->next;
		delcron(t);
	}
}

static int calc_timeout(void)
{
	time_t now = time(NULL), future;
	struct tm tmstruct;
	crontab_t mask, *t;
	int minutes;

	for (minutes = 0; minutes < 120; ++minutes) {
		future = now + minutes * 60;

		localtime_r(&future, &tmstruct);
		cron_tm_to_mask(&mask, &tmstruct);

		for (t = jobs; t != NULL; t = t->next) {
			if (cron_should_run(t, &mask))
				goto out;
		}
	}
out:
	return minutes ? minutes * 60 : 60;
}

static void runjobs(void)
{
	time_t now = time(NULL);
	struct tm tmstruct;
	crontab_t mask, *t;

	localtime_r(&now, &tmstruct);
	cron_tm_to_mask(&mask, &tmstruct);

	for (t = jobs; t != NULL; t = t->next) {
		if (cron_should_run(t, &mask))
			runjob(t);
	}
}

static void sighandler(int signo)
{
	switch (signo) {
	case SIGINT:
	case SIGTERM:
		run = 0;
		break;
	case SIGHUP:
		rescan = 1;
		break;
	}
}

int main(void)
{
	struct timespec stime;
	struct sigaction act;
	crontab_t *t;
	int timeout;
	pid_t pid;

	memset(&act, 0, sizeof(act));
	act.sa_handler = sighandler;
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGHUP, &act, NULL);

	while (run) {
		if (rescan == 1) {
			cleanup_config();
			read_config();
			timeout = 60;
			rescan = 0;
		} else {
			runjobs();
			timeout = calc_timeout();
		}

		stime.tv_sec = timeout;
		stime.tv_nsec = 0;

		while (nanosleep(&stime, &stime) != 0 && run && !rescan) {
			if (errno != EINTR) {
				perror("nanosleep");
				break;
			}
		}

		while ((pid = waitpid(-1, NULL, WNOHANG)) != -1) {
			for (t = jobs; t != NULL; t = t->next) {
				if (t->pid == pid) {
					t->pid = -1;
					break;
				}
			}
		}
	}

	return EXIT_SUCCESS;
}
