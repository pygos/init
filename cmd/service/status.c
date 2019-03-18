/* SPDX-License-Identifier: ISC */
#include "servicecmd.h"
#include "initsock.h"
#include "service.h"
#include "config.h"

#include <unistd.h>

static void free_resp(init_status_response_t *resp)
{
	free(resp->filename);
	free(resp->service_name);
}

static int cmd_status(int argc, char **argv)
{
	init_status_response_t resp;
	int fd, ret = EXIT_FAILURE;
	char tmppath[256];
	const char *state;
	bool is_tty;

	if (check_arguments(argv[0], argc, 1, 1))
		return EXIT_FAILURE;

	sprintf(tmppath, "/tmp/svcstatus.%d.sock", (int)getpid());
	fd = init_socket_open(tmppath);

	if (fd < 0) {
		unlink(tmppath);
		return EXIT_FAILURE;
	}

	if (init_socket_send_request(fd, EIR_STATUS))
		goto out;

	is_tty = (isatty(STDOUT_FILENO) == 1);

	for (;;) {
		memset(&resp, 0, sizeof(resp));

		if (init_socket_recv_status(fd, &resp)) {
			perror("reading from initd socket");
			free_resp(&resp);
			goto out;
		}

		if (resp.state == ESS_NONE) {
			free_resp(&resp);
			break;
		}

		switch (resp.state) {
		case ESS_RUNNING:
			if (!is_tty) {
				state = "Running";
				break;
			}
			state = "\033[22;32mRunning\033[0m";
			break;
		case ESS_ENQUEUED:
			state = " Queue ";
			break;
		case ESS_EXITED:
			if (!is_tty) {
				state = "Exited ";
				break;
			}
			if (resp.exit_status == EXIT_SUCCESS) {
				state = "\033[22;33mExited \033[0m";
			} else {
				state = "\033[22;31mExited \033[0m";
			}
			break;
		default:
			if (!is_tty) {
				state = "Unknown";
				break;
			}
			state = "\033[22;31mUnknown\033[0m";
			break;
		}

		printf("[%s] %s\n", state, resp.filename);

		free_resp(&resp);
	}

	ret = EXIT_SUCCESS;
out:
	close(fd);
	unlink(tmppath);
	return ret;
}

static command_t status = {
	.cmd = "status",
	.usage = "",
	.s_desc = "report the status of the currently enabled services",
	.l_desc = "Gathers a list of all currently running services and the "
		  "state that they are in (currently running, exited, wating "
		  "to get scheduled).",
	.run_cmd = cmd_status,
};

REGISTER_COMMAND(status)
