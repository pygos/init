/* SPDX-License-Identifier: ISC */
#ifndef INIT_H
#define INIT_H

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <endian.h>
#include <stdio.h>
#include <errno.h>
#include <poll.h>

#include <linux/reboot.h>
#include <sys/signalfd.h>
#include <sys/reboot.h>
#include <signal.h>

#include "initsock.h"
#include "service.h"
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

void supervisor_answer_status_request(int fd, const void *dest_addr,
				      size_t addrlen, E_SERVICE_STATE filter);

void supervisor_start(int id);

void supervisor_stop(int id);

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
