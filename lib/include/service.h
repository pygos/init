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
	TGT_CAD,		/* run service when CTRL+ALT+DEL is pressed */

	TGT_MAX
};

typedef struct service_t {
	int type;		/* SVC_* service type */
	int target;		/* TGT_* service target */
	char *name;		/* canonical service name */
	char *desc;		/* description string */
	char **exec;		/* command lines to execute */
	size_t num_exec;	/* number of command lines */
	char *ctty;		/* controlling tty or log file */

	char **before;		/* services that must be executed later */
	size_t num_before;
	char **after;		/* services that must be executed first */
	size_t num_after;

	pid_t pid;
	int status;		/* process exit status */

	struct service_t *next;
} service_t;

typedef struct {
	service_t *targets[TGT_MAX];
} service_list_t;

/*
	Read a service from a file.
*/
service_t *rdsrv(int dirfd, const char *filename);

void delsrv(service_t *srv);

/*
	Rebuild a service list by scanning a directory and parsing all
	service descriptions.

	Returns 0 on success, -1 on failure. The function takes care of
	printing error messages on failure.
*/
int srvscan(const char *directory, service_list_t *list);

void del_srv_list(service_list_t *list);

/*
	Sort a list of services by dependencies.
*/
service_t *srv_tsort(service_t *list);

#endif /* SERVICE_H */

