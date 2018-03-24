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
#include "servicecmd.h"
#include "service.h"
#include "config.h"

static int cmd_list(int argc, char **argv)
{
	int i, ret = EXIT_SUCCESS;
	service_list_t list;
	service_t *svc;

	(void)argc; (void)argv;

	if (srvscan(SVCDIR, &list)) {
		fprintf(stderr, "Error while reading services from %s\n",
			SVCDIR);
		ret = EXIT_FAILURE;
	}

	for (i = 0; i < TGT_MAX; ++i) {
		if (list.targets[i] == NULL)
			continue;

		fputs("******** target: ", stdout);

		switch (i) {
		case TGT_BOOT:
			fputs("boot", stdout);
			break;
		case TGT_SHUTDOWN:
			fputs("shutdown", stdout);
			break;
		case TGT_REBOOT:
			fputs("reboot", stdout);
			break;
		case TGT_CAD:
			fputs("ctrl-alt-delete", stdout);
			break;
		}

		fputs(" ********\n", stdout);

		for (svc = list.targets[i]; svc != NULL; svc = svc->next) {
			fprintf(stdout, "Name: %s\n", svc->name);
			fprintf(stdout, "Descrption: %s\n", svc->desc);

			fputs("Type: ", stdout);
			switch (svc->type) {
			case SVC_ONCE: fputs("once\n", stdout); break;
			case SVC_WAIT: fputs("wait\n", stdout); break;
			case SVC_RESPAWN: fputs("respawn\n", stdout); break;
			}

			fputc('\n', stdout);
		}

		fputc('\n', stdout);
	}

	del_srv_list(&list);
	return ret;
}

static command_t list = {
	.cmd = "list",
	.usage = "",
	.s_desc = "print a list of currently enabled services",
	.l_desc = "Print a list of currently enabled services.",
	.run_cmd = cmd_list,
};

REGISTER_COMMAND(list)
