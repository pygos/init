/* SPDX-License-Identifier: ISC */
#ifndef SERVICECMD_H
#define SERVICECMD_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "service.h"
#include "config.h"

/*
	Describes a command that can be launched by passing its name as
	second command line argument to the main() function (i.e. immediately
	after the actual program name).

	Short and long descriptions can be provided to print out help text.

	The main() function calls into a callback in this structure to execute
	the command.
*/
typedef struct command_t {
	struct command_t *next;

	const char *cmd;	/* command name */
	const char *usage;	/* list of possible arguments */
	const char *s_desc;	/* short description used by help */
	const char *l_desc;	/* long description used by help */

	/*
		Semantics are the same as for main(). Called from main()
		function with first argument (i.e. top level program name)
		removed.
	*/
	int (*run_cmd)(int argc, char **argv);
} command_t;

/* Global list of available commands */
extern command_t *commands;

service_t *loadsvc(const char *directory, const char *filename, int flags);

/*
	Implemented in servicecmd.c. Prints program usage message and
	terminates with the given exit status.
*/
void usage(int status) __attribute__((noreturn));

/*
	Write a message to stderr that advises the user how to consult the
	help text for a specific command.
*/
void tell_read_help(const char *cmd);

/*
	Check if the argument count is within specified bounds (minc and maxc
	inclusive). If it is, return 0.

	If it isn't, complain about a wrong number of arguments for a
	command (cmd), tell the user to consult the help text and return -1.
*/
int check_arguments(const char *cmd, int argc, int minc, int maxc);

/*
	To implement a new command, add a global, static instance of a
	command_t (or derived) structure to a C file and pass it to this
	macro to have it automatically registered on program startup.
*/
#define REGISTER_COMMAND(cmd) \
	static void __attribute__((constructor)) register_##cmd(void) \
	{ \
		command_t *c = (command_t *)&cmd; \
		\
		c->next = commands; \
		commands = c; \
	}

#endif /* SERVICECMD_H */
