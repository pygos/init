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

#include <linux/reboot.h>
#include <sys/reboot.h>

#include "service.h"
#include "telinit.h"
#include "util.h"

enum {
	STATUS_OK = 0,
	STATUS_FAIL,
	STATUS_WAIT,
};

/********** runlst.c **********/

/*
	Plow through an array of strings and execute each one, i.e. do
	a fork() and exec().

	In the parent process, wait() until the child is done before
	continuing through the list.

	If ctty is not NULL, open it and redirect all I/O of the child
	process to that file.

	If everyhing works, the function returns EXIT_SUCCESS. If one child
	does not exit with EXIT_SUCCESS, processing of the list is aborted
	and the function returns the exit status of the failed process.
*/
int runlst_wait(char **exec, size_t num, const char *ctty);

/*
	Does basically the same as runlst_wait, but asynchronously.

	A child process is created that calls runlst_wait exits with the
	result of runlst_wait. In the parent process, the function returns
	immediately with the PID of the child process.

	Alternatively, if num is 1, the child process directly exec()s the
	given command.
*/
pid_t runlst(char **exec, size_t num, const char *ctty);

/********** setup_tty.c **********/

/*
	Initial tty setup for init. Makes /dev/console our controlling tty and
	reroutes all output there. Closes stdin because we presumably won't
	need it anymore.

	Returns 0 on success, -1 on failure. The function takes care of
	printing error messages on failure.
*/
int setup_tty(void);

/********** status.c **********/

/*
	Print a status message. Type is either STATUS_OK, STATUS_FAIL or
	STATUS_WAIT.

	A new-line is appended to the mssage, UNLESS type is STATUS_WAIT.

	If update is true, print a carriage return first to overwrite the
	current line (e.g. after a STATUS_WAIT message).
*/
void print_status(const char *msg, int type, bool update);

/********** mksock.c **********/

/*
	Create a UNIX socket that programs can use to pass messages to init.

	Returns the socked fd or -1 on failure. The function takes care of
	printing error messages on failure.

	The socket has the CLOEXEC flag set.
*/
int mksock(void);

/********** svclist.c **********/

/*
	Returns true if the list of running services contains
	single shot processes.
*/
bool svclist_have_singleshot(void);

/* Add a service to the list of running services */
void svclist_add(service_t *svc);

/*
	Remove a service, identifierd by PID, from the list of
	running services.

	Returns the service identified by the PID or NULL if there
	is no such service.
*/
service_t *svclist_remove(pid_t pid);

#endif /* INIT_H */

