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

extern char *__progname;

static void pretty_print(const char *str, int padd, int len)
{
	int i, brk;
next:
	while (isspace(*str) && *str != '\n')
		++str;

	if (!(*str))
		return;

	if (*str == '\n') {
		fputc('\n', stdout);
		++str;
		len = padd;
		goto next;
	}

	for (i = 0, brk = 0; str[i]; ++i) {
		if (str[i] == '<' || str[i] == '[')
			++brk;
		if (str[i] == '>' || str[i] == ']')
			--brk;
		if (!brk && isspace(str[i]))
			break;
	}

	if ((len + i) < 80) {
		fwrite(str, 1, i, stdout);
		str += i;
		len += i;

		if ((len + 1) < 80) {
			fputc(' ', stdout);
			++len;
			goto next;
		}
	}

	printf("\n%*s", padd, "");
	len = padd;
	goto next;
}

static void print_cmd_usage(const char *cmd, const char *usage)
{
	int padd;

	padd = printf("Usage: %s %s ", __progname, cmd);

	if ((strlen(usage) + padd) < 80) {
		fputs(usage, stdout);
		return;
	}

	pretty_print(usage, padd, padd);
}

static int cmd_help(int argc, char **argv)
{
	const char *help;
	command_t *cmd;

	if (argc < 2)
		usage(EXIT_SUCCESS);

	if (argc > 2) {
		fprintf(stderr, "Too many arguments\n\n"
				"Usage: %s help <command>", __progname);
		return EXIT_FAILURE;
	}

	for (cmd = commands; cmd != NULL; cmd = cmd->next) {
		if (strcmp(cmd->cmd, argv[1]) != 0)
			continue;

		print_cmd_usage(cmd->cmd, cmd->usage);
		fputs("\n\n", stdout);

		help = cmd->l_desc ? cmd->l_desc : cmd->s_desc;

		if (islower(*help)) {
			fputc(toupper(*(help++)), stdout);
			pretty_print(help, 0, 1);
		} else {
			pretty_print(help, 0, 0);
		}

		fputc('\n', stdout);
		return EXIT_SUCCESS;
	}

	fprintf(stderr, "Unknown command '%s'\n\n"
			"Try `%s help' for a list of available commands\n",
			argv[1], __progname);
	return EXIT_FAILURE;
}

static command_t help = {
	.cmd = "help",
	.usage = "<command>",
	.s_desc = "print a help text for a command",
	.l_desc = "Print a help text for a specified command. If no command "
		"is specified, print a generic help text and a list of "
		"available commands.",
	.run_cmd = cmd_help,
};

REGISTER_COMMAND(help)
