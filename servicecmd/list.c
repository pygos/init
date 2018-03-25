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

static void print_services(service_t *svc)
{
	size_t i;

	for (; svc != NULL; svc = svc->next) {
		printf("Name: %s\n", svc->name);
		printf("\tDescrption: %s\n", svc->desc);
		printf("\tType: %s\n", svc_type_to_string(svc->type));
		printf("\tTarget: %s\n", svc_target_to_string(svc->target));

		if (svc->num_before) {
			fputs("\tMust be run before:\n", stdout);

			for (i = 0; i < svc->num_before; ++i)
				printf("\t\t%s\n", svc->before[i]);
		}

		if (svc->num_after) {
			fputs("\tMust be run after:\n", stdout);

			for (i = 0; i < svc->num_after; ++i)
				printf("\t\t%s\n", svc->after[i]);
		}
	}
}

static int cmd_list(int argc, char **argv)
{
	int i, ret = EXIT_SUCCESS;
	service_list_t list;

	if (check_arguments(argv[0], argc, 1, 2))
		return EXIT_FAILURE;

	if (svcscan(SVCDIR, &list)) {
		fprintf(stderr, "Error while reading services from %s\n",
			SVCDIR);
		ret = EXIT_FAILURE;
	}

	if (argc == 2) {
		i = svc_type_from_string(argv[1]);

		if (i == -1) {
			fprintf(stderr, "Unknown target `%s'\n", argv[1]);
			tell_read_help(argv[1]);
			ret = EXIT_FAILURE;
			goto out;
		}

		print_services(list.targets[i]);
	} else {
		for (i = 0; i < TGT_MAX; ++i)
			print_services(list.targets[i]);
	}
out:
	del_svc_list(&list);
	return ret;
}

static command_t list = {
	.cmd = "list",
	.usage = "[target]",
	.s_desc = "print a list of currently enabled services",
	.l_desc = "Print a list of currently enabled services. If an "
		  "optional target is specified, print services for this "
		  "target.",
	.run_cmd = cmd_list,
};

REGISTER_COMMAND(list)
