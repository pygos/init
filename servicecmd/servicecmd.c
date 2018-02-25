#include <stdlib.h>
#include <stdio.h>

#include "servicecmd.h"
#include "service.h"
#include "config.h"
#include "util.h"


command_t *commands;

extern char *__progname;


void usage(int status)
{
	FILE *stream = (status == EXIT_SUCCESS ? stdout : stderr);
	int padd = 0, len;
	command_t *cmd;

	fprintf(stream, "usage: %s <command> [args...]\n\n"
			"Available commands:\n\n", __progname);

	for (cmd = commands; cmd != NULL; cmd = cmd->next) {
		len = strlen(cmd->cmd);

		padd = len > padd ? len : padd;
	}

	for (cmd = commands; cmd != NULL; cmd = cmd->next) {
		fprintf(stream, "%*s - %s\n",
			(int)padd + 1, cmd->cmd, cmd->s_desc);
	}

	fprintf(stream, "\nTry `%s help <command>' for more information "
			"on a specific command\n", __progname);

	exit(status);
}

int main(int argc, char **argv)
{
	command_t *cmd;

	if (argc < 2)
		usage(EXIT_SUCCESS);

	for (cmd = commands; cmd != NULL; cmd = cmd->next) {
		if (!strcmp(cmd->cmd, argv[1])) {
			return cmd->run_cmd(argc - 1, argv + 1);
		}
	}

	fprintf(stderr, "Unknown command '%s'\n\n", argv[1]);
	usage(EXIT_FAILURE);
}
