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
#ifndef SERVICE_H
#define SERVICE_H

#include <sys/types.h>

#include "util.h"

enum {
	/*
		Start the service in the background and continue with
		other services. The service will eventually terminate.
	*/
	SVC_ONCE = 0,
	SVC_WAIT,		/* run service and wait until it finishes */

	/*
		Similar to SVC_ONCE, but restart the service when
		it terminates.
	*/
	SVC_RESPAWN,
};

enum {
	TGT_BOOT = 0,		/* run service when the system boots */
	TGT_SHUTDOWN,		/* run service when at system shut down */
	TGT_REBOOT,		/* run service when during system reboot */

	TGT_MAX
};

enum {
	RDSVC_NO_FNAME = 0x01,	/* do not store a copy of the filename */
	RDSVC_NO_EXEC = 0x02,	/* do not store executable script */
	RDSVC_NO_CTTY = 0x04,	/* do not store the controlling tty */
	RDSVC_NO_DEPS = 0x08,	/* do not store dependencies */
};

enum {
	/* truncate stdout */
	SVC_FLAG_TRUNCATE_OUT = 0x01,
};

typedef struct exec_t {
	struct exec_t *next;
	int argc;		/* number of elements in argument vector */
	char args[];		/* argument vectot string blob */
} exec_t;

typedef struct service_t {
	struct service_t *next;

	char *fname;		/* source file name */

	int type;		/* SVC_* service type */
	int target;		/* TGT_* service target */
	char *desc;		/* description string */
	char *ctty;		/* controlling tty or log file */
	int rspwn_limit;	/* maximum respawn count */
	unsigned int flags;	/* SVC_FLAG_* bit field */

	/* linked list of command lines to execute */
	exec_t *exec;

	char *before;	/* services that must be executed later */
	char *after;	/* services that must be executed first */

	int num_before;
	int num_after;

	pid_t pid;
	int status;		/* process exit status */

	char name[];		/* canonical service name */
} service_t;

typedef struct {
	service_t *targets[TGT_MAX];
} service_list_t;

/*
	Read a service from a file.
*/
service_t *rdsvc(int dirfd, const char *filename, int flags);

void delsvc(service_t *svc);

/*
	Rebuild a service list by scanning a directory and parsing all
	service descriptions.

	Returns 0 on success, -1 on failure. The function takes care of
	printing error messages on failure.
*/
int svcscan(const char *directory, service_list_t *list, int flags);

void del_svc_list(service_list_t *list);

/*
	Sort a list of services by dependencies.
*/
service_t *svc_tsort(service_t *list);

const char *svc_type_to_string(int type);

int svc_type_from_string(const char *type);

const char *svc_target_to_string(int target);

int svc_target_from_string(const char *target);

int setup_tty(const char *tty, bool truncate);

NORETURN void argv_exec(exec_t *e);

#endif /* SERVICE_H */

