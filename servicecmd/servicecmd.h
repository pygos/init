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
#ifndef SERVICECMD_H
#define SERVICECMD_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "util.h"

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

/*
	Implemented in servicecmd.c. Prints program usage message and
	terminates with the given exit status.
*/
void usage(int status) NORETURN;

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

