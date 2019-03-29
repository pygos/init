/* SPDX-License-Identifier: ISC */
#include "servicecmd.h"
#include "initsock.h"
#include "service.h"
#include "config.h"

#include <fnmatch.h>
#include <getopt.h>
#include <unistd.h>

static int cmd_startstop(int argc, char **argv,
			 E_SERVICE_STATE filter, E_INIT_REQUEST action)
{
	int i, fd, ret = EXIT_FAILURE;
	init_status_t resp;
	char tmppath[256];
	bool found;

	if (check_arguments(argv[0], argc, 2, 2))
		return EXIT_FAILURE;

	sprintf(tmppath, "/tmp/svcstatus.%d.sock", (int)getpid());
	fd = init_socket_open(tmppath);

	if (fd < 0) {
		unlink(tmppath);
		return EXIT_FAILURE;
	}

	if (init_socket_send_request(fd, EIR_STATUS, filter))
		goto out;

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

		found = false;

		for (i = optind; i < argc; ++i) {
			if (fnmatch(argv[i], resp.service_name, 0) == 0) {
				found = true;
				break;
			}
			if (fnmatch(argv[i], resp.filename, 0) == 0) {
				found = true;
				break;
			}
		}

		if (found) {
			if (init_socket_send_request(fd, action, resp.id))
				goto out;
		}

		free_init_status(&resp);
	}

	ret = EXIT_SUCCESS;
out:
	close(fd);
	unlink(tmppath);
	return ret;
}

static int cmd_start(int argc, char **argv)
{
	return cmd_startstop(argc, argv, ESS_NONE, EIR_START);
}

static int cmd_stop(int argc, char **argv)
{
	return cmd_startstop(argc, argv, ESS_NONE, EIR_STOP);
}

static command_t start = {
	.cmd = "start",
	.usage = "services...",
	.s_desc = "start a currently not running service",
	.l_desc = "Start one or more service that are currently not running. "
		  "Shell style globbing patterns can used for service names.",
	.run_cmd = cmd_start,
};

static command_t stop = {
	.cmd = "stop",
	.usage = "services...",
	.s_desc = "stop a currently running service",
	.l_desc = "Stop one or more service that are currently running. "
		  "Shell style globbing patterns can used for service names.",
	.run_cmd = cmd_stop,
};

REGISTER_COMMAND(start)
REGISTER_COMMAND(stop)
