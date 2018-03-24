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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

#include "init.h"

void do_shutdown(int type)
{
	struct timespec req, rem;

	print_status("sending SIGTERM to all processes", STATUS_WAIT, false);
	kill(-1, SIGTERM);

	memset(&req, 0, sizeof(req));
	memset(&rem, 0, sizeof(rem));
	req.tv_sec = 5;		/* TODO: make configurable? */

	while (nanosleep(&req, &rem) != 0 && errno == EINTR)
		req = rem;

	print_status("sending SIGTERM to all processes", STATUS_OK, true);
	kill(-1, SIGKILL);
	print_status("sending SIGKILL to remaining processes",
		     STATUS_OK, false);

	print_status("sync", STATUS_WAIT, false);
	sync();
	print_status("sync", STATUS_OK, true);

	reboot(type);
	perror("reboot system call");
	exit(EXIT_FAILURE);
}
