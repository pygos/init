#ifndef SERVICECMD_H
#define SERVICECMD_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "util.h"

typedef struct command_t {
	struct command_t *next;

	const char *cmd;
	const char *usage;
	const char *s_desc;
	const char *l_desc;

	int (*run_cmd)(int argc, char **argv);
} command_t;

extern command_t *commands;

void usage(int status) NORETURN;

#define REGISTER_COMMAND(cmd) \
	static void __attribute__((constructor)) register_##cmd(void) \
	{ \
		command_t *c = (command_t *)&cmd; \
		\
		c->next = commands; \
		commands = c; \
	}

#endif /* SERVICECMD_H */

