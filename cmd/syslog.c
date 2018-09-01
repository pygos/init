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
#include <getopt.h>
#include <syslog.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "util.h"

static int facility = LOG_USER;
static int level = LOG_INFO;
static int flags = LOG_NDELAY | LOG_NOWAIT;
static const char *ident = "(shell)";

static const enum_map_t facility_map[] = {
	{ "auth", LOG_AUTH },
	{ "cron", LOG_CRON },
	{ "daemon", LOG_DAEMON },
	{ "ftp", LOG_FTP },
	{ "local0", LOG_LOCAL0 },
	{ "local1", LOG_LOCAL1 },
	{ "local2", LOG_LOCAL2 },
	{ "local3", LOG_LOCAL3 },
	{ "local4", LOG_LOCAL4 },
	{ "local5", LOG_LOCAL5 },
	{ "local6", LOG_LOCAL6 },
	{ "local7", LOG_LOCAL7 },
	{ "lpr", LOG_LPR },
	{ "news", LOG_NEWS },
	{ "user", LOG_USER },
	{ "uucp", LOG_UUCP },
	{ NULL, 0 },
};

static const enum_map_t level_map[] = {
	{ "emergency", LOG_EMERG },
	{ "alert", LOG_ALERT },
	{ "critical", LOG_CRIT },
	{ "error", LOG_ERR },
	{ "warning", LOG_WARNING },
	{ "notice", LOG_NOTICE },
	{ "info", LOG_INFO },
	{ "debug", LOG_DEBUG },
	{ NULL, 0 },
};

static const struct option options[] = {
	{ "help", no_argument, NULL, 'h' },
	{ "version", no_argument, NULL, 'V' },
	{ "console", required_argument, NULL, 'c' },
	{ "facility", required_argument, NULL, 'f' },
	{ "level", required_argument, NULL, 'l' },
	{ "ident", required_argument, NULL, 'i' },
	{ NULL, 0, NULL, 0 },
};

static const char *shortopt = "hVcf:l:i:";

static const char *helptext =
"Usage: syslog [OPTION]... [STRING]...\n\n"
"Concatenate the given STRINGs and send a log message to the syslog daemon.\n"
"\n"
"The following OPTIONSs can be used:\n"
"  -f, --facility <facility>  Logging facilty name or numeric identifier.\n"
"  -l, --level <level>        Log level name or numeric identifier.\n"
"  -i, --ident <name>         Program name for log syslog message.\n"
"                             Default is %s.\n\n"
"  -c, --console              Write to the console if opening the syslog\n"
"                             socket fails.\n\n"
"  -h, --help                 Print this help text and exit\n"
"  -V, --version              Print version information and exit\n\n";

static void print_map(const enum_map_t *map, int defaultval,
		      const char *option)
{
	size_t i;

	printf("The following values can be used for %s:\n", option);

	for (i = 0; map[i].name != NULL; ++i) {
		if (map[i].value == defaultval) {
			printf("  %s (=%d), set as default\n",
			       map[i].name, map[i].value);
		} else {
			printf("  %s (=%d)\n", map[i].name, map[i].value);
		}
	}

	fputc('\n', stdout);
}

static NORETURN void usage(int status)
{
	if (status != EXIT_SUCCESS) {
		fputs("Try `syslog --help' for more information\n", stderr);
	} else {
		printf(helptext, ident);
		print_map(level_map, level, "--level");
		print_map(facility_map, facility, "--facility");
	}

	exit(status);
}

static int readint(const char *str)
{
	int x = 0;

	if (!isdigit(*str))
		return -1;

	while (isdigit(*str))
		x = x * 10 + (*(str++)) - '0';

	return (*str == '\0') ? x : -1;
}

static void process_options(int argc, char **argv)
{
	const enum_map_t *e;
	int c;

	for (;;) {
		c = getopt_long(argc, argv, shortopt, options, NULL);
		if (c == -1)
			break;

		switch (c) {
		case 'f':
			facility = readint(optarg);
			if (facility >= 0)
				break;
			e = enum_by_name(facility_map, optarg);
			if (e == NULL) {
				fprintf(stderr, "Unknown facility name '%s'\n",
					optarg);
				usage(EXIT_FAILURE);
			}
			facility = e->value;
			break;
		case 'l':
			level = readint(optarg);
			if (level >= 0)
				break;
			e = enum_by_name(level_map, optarg);
			if (e == NULL) {
				fprintf(stderr, "Unknown log level '%s'\n",
					optarg);
				usage(EXIT_FAILURE);
			}
			level = e->value;
			break;
		case 'i':
			ident = optarg;
			break;
		case 'c':
			flags |= LOG_CONS;
			break;
		case 'h':
			usage(EXIT_SUCCESS);
		case 'V':
			print_version("syslog");
		default:
			usage(EXIT_FAILURE);
		}
	}
}


int main(int argc, char **argv)
{
	size_t len = 0;
	char *str;
	int i;

	process_options(argc, argv);

	if (optind >= argc) {
		fputs("Error: no log string provided.\n", stderr);
		usage(EXIT_FAILURE);
	}

	for (i = optind; i < argc; ++i)
		len += strlen(argv[i]);

	len += argc - optind - 1;

	str = calloc(1, len + 1);
	if (str == NULL) {
		fputs("syslog: out of memory\n", stderr);
		return EXIT_FAILURE;
	}

	for (i = optind; i < argc; ++i) {
		if (i > optind)
			strcat(str, " ");
		strcat(str, argv[i]);
	}

	openlog(ident, flags, facility);
	syslog(level, "%s", str);
	closelog();

	free(str);
	return EXIT_SUCCESS;
}

