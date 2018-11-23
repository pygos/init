/* SPDX-License-Identifier: ISC */
#include "servicecmd.h"
#include "service.h"
#include "config.h"

static void print_services(service_t *svc, const char *target)
{
	printf("Services for target '%s' in dependency order:\n\n", target);

	for (; svc != NULL; svc = svc->next) {
		printf("%s - %s\n", svc->name, svc->desc);
		printf("\tType: %s\n", svc_type_to_string(svc->type));

		if (svc->type == SVC_RESPAWN && svc->rspwn_limit > 0)
			printf("\tRespawn limit: %d\n", svc->rspwn_limit);
	}
}

static int cmd_list(int argc, char **argv)
{
	int i, ret = EXIT_SUCCESS;
	service_list_t list;

	if (check_arguments(argv[0], argc, 1, 2))
		return EXIT_FAILURE;

	if (svcscan(SVCDIR, &list, 0)) {
		fprintf(stderr, "Error while reading services from %s\n",
			SVCDIR);
		ret = EXIT_FAILURE;
	}

	if (argc == 2) {
		i = svc_target_from_string(argv[1]);

		if (i == -1) {
			fprintf(stderr, "Unknown target `%s'\n", argv[1]);
			tell_read_help(argv[0]);
			ret = EXIT_FAILURE;
			goto out;
		}

		print_services(list.targets[i], argv[1]);
	} else {
		for (i = 0; i < TGT_MAX; ++i) {
			if (i != 0)
				printf("\n\n");
			print_services(list.targets[i],
				       svc_target_to_string(i));
		}
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
