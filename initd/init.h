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
#ifndef INIT_H
#define INIT_H

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <poll.h>

#include <linux/reboot.h>
#include <sys/signalfd.h>
#include <sys/reboot.h>
#include <signal.h>

#include "service.h"
#include "telinit.h"
#include "util.h"

#define RUNSVCBIN SCRIPTDIR "/runsvc"

enum {
	STATUS_OK = 0,
	STATUS_FAIL,
	STATUS_WAIT,
	STATUS_STARTED,
};

/********** main.c **********/

void target_completed(int target);

/********** runsvc.c **********/

/*
	Invoke the runsvc command to execute the comands of a service.

	Returns the pid of the child process containing the runsvc instance.
*/
pid_t runsvc(service_t *svc);

/********** status.c **********/

/*
	Print a status message. Type is either STATUS_OK, STATUS_FAIL,
	STATUS_WAIT or STATUS_STARTED.

	A new-line is appended to the mssage, UNLESS type is STATUS_WAIT.

	If update is true, print a carriage return first to overwrite the
	current line (e.g. after a STATUS_WAIT message).
*/
void print_status(const char *msg, int type, bool update);

/********** supervisor.c **********/

void supervisor_handle_exited(pid_t pid, int status);

void supervisor_set_target(int next);

void supervisor_init(void);

bool supervisor_process_queues(void);

/********** signal_<platform>.c **********/

/*
	Setup signal handling. Returns -1 on error, a file descriptor on
	success.

	The returned file descriptor can be polled and becomes readable
	when a signal arrives. Reading from it returns a signalfd_siginfo
	structure.

	The returned file descriptor has the close on exec flag set.

	The kernel is also told to send us SIGINT signals if a user presses
	the local equivalent of CTRL+ALT+DEL.
*/
int sigsetup(void);

/*
	Undo everything that sigsetup() changed about signal handling and
	restore the default.
*/
void sigreset(void);

#endif /* INIT_H */

