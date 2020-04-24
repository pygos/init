/* SPDX-License-Identifier: ISC */
#include "servicecmd.h"
#include "initsock.h"
#include "service.h"
#include "config.h"

#include <fnmatch.h>
#include <getopt.h>
#include <unistd.h>

static const struct option long_opts[] = {
	{ "detail", no_argument, NULL, 'd' },
	{ NULL, 0, NULL, 0 },
};

static const char *short_opts = "d";

static int cmd_status(int argc, char **argv)
{
	bool is_tty, found, show_details = false;
	int i, fd, ret = EXIT_FAILURE;
	init_status_t resp;
	char tmppath[256];
	const char *state;
	service_t *svc;

	for (;;) {
		i = getopt_long(argc, argv, short_opts, long_opts, NULL);
		if (i == -1)
			break;

		switch (i) {
		case 'd':
			show_details = true;
			break;
		default:
			tell_read_help(argv[0]);
			return EXIT_FAILURE;
		}
	}

	sprintf(tmppath, "/tmp/svcstatus.%d.sock", (int)getpid());
	fd = init_socket_open(tmppath);

	if (fd < 0) {
		unlink(tmppath);
		return EXIT_FAILURE;
	}

	if (init_socket_send_request(fd, EIR_STATUS, ESS_NONE))
		goto out;

	is_tty = (isatty(STDOUT_FILENO) == 1);

	for (;;) {
		memset(&resp, 0, sizeof(resp));

		if (init_socket_recv_status(fd, &resp)) {
			perror("reading from initd socket");
			free_init_status(&resp);
			goto out;
		}

		if (resp.state == ESS_NONE) {
			free_init_status(&resp);
			break;
		}

		if (optind < argc) {
			found = false;

			for (i = optind; i < argc; ++i) {
				if (fnmatch(argv[i],
					    resp.service_name, 0) == 0) {
					found = true;
					break;
				}
				if (fnmatch(argv[i], resp.filename, 0) == 0) {
					found = true;
					break;
				}
			}

			if (!found) {
				free_init_status(&resp);
				continue;
			}
		}

		switch (resp.state) {
		case ESS_RUNNING:
			if (!is_tty) {
				state = "UP";
				break;
			}
			state = "\033[22;32m UP \033[0m";
			break;
		case ESS_ENQUEUED:
			state = "SCHED";
			break;
		case ESS_FAILED:
			if (!is_tty) {
				state = "FAIL";
				break;
			}
			state = "\033[22;31mFAIL\033[0m";
			break;
		case ESS_DONE:
			if (!is_tty) {
				state = "DONE";
				break;
			}

			state = "\033[22;33mDONE\033[0m";
			break;
		default:
			if (!is_tty) {
				state = "UNKNOWN";
				break;
			}
			state = "\033[22;31mUNKNOWN\033[0m";
			break;
		}

		if (show_details) {
			printf("Service: %s\n", resp.filename);
			printf("\tStatus: %s\n", state);
			printf("\tTemplate name: %s\n", resp.service_name);
			printf("\tExit status: %d\n", resp.exit_status);

			svc = loadsvc(SVCDIR, resp.filename);

			if (svc == NULL) {
				fputs("\tError loading service file\n", stdout);
			} else {
				printf("\tDescription: %s\n", svc->desc);
				printf("\tType: %s\n",
				       svc_type_to_string(svc->type));
				printf("\tTarget: %s\n",
				       svc_target_to_string(svc->target));
				delsvc(svc);
			}
		} else {
			printf("[%s] %s\n", state, resp.filename);
		}

		free_init_status(&resp);
	}

	ret = EXIT_SUCCESS;
out:
	close(fd);
	unlink(tmppath);
	return ret;
}

static command_t status = {
	.cmd = "status",
	.usage = "[-d|--detail] [services...]",
	.s_desc = "report the status of the currently enabled services",
	.l_desc = "Gathers a list of all currently running services and the "
		  "state that they are in (currently running, done, failed, "
		  "wating to get scheduled). A list of services with wildcard "
		  "globbing patterns can be specified. If ommitted, produces "
		  "a general overview of all services. If the --detail "
		  "is given, more details are shown about a service.",
	.run_cmd = cmd_status,
};

REGISTER_COMMAND(status)
