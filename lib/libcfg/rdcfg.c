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
#include "libcfg.h"

#include <string.h>
#include <stdio.h>
#include <ctype.h>

static const cfg_param_t *find_param(rdline_t *rd, const char *name,
				     const cfg_param_t *params, size_t count)
{
	size_t i;

	for (i = 0; i < count; ++i) {
		if (!strcmp(params[i].key, name))
			return params + i;
	}

	fprintf(stderr, "%s: %zu: unknown keyword '%s'\n",
		rd->filename, rd->lineno, name);
	return NULL;
}

static int splitkv(rdline_t *rd, char **k, char **v)
{
	char *key = rd->line, *value = rd->line;

	while (*value != ' ' && *value != '\0') {
		if (!isalpha(*value)) {
			fprintf(stderr,
				"%s: %zu: unexpected '%c' in keyword\n",
				rd->filename, rd->lineno, *value);
			return -1;
		}
		++value;
	}

	if (*value != ' ') {
		fprintf(stderr, "%s: %zu: expected argument after '%s'\n",
			rd->filename, rd->lineno, key);
		return -1;
	}

	*(value++) = '\0';

	*k = key;
	*v = value;
	return 0;
}

int rdcfg(void *cfgobj, rdline_t *rd, const cfg_param_t *params, size_t count,
	  int flags)
{
	const cfg_param_t *p;
	char *key, *value;
	int ret;

	while ((ret = rdline(rd)) == 0) {
		if (splitkv(rd, &key, &value))
			return -1;

		p = find_param(rd, key, params, count);
		if (p == NULL)
			return -1;

		if (p->allow_block && *value == '{') {
			for (++value; *value == ' '; ++value)
				;

			if (*value != '\0') {
				ret = p->handle(cfgobj, value, rd, flags);
				if (ret)
					return -1;
			}

			while ((ret = rdline(rd)) == 0) {
				if (strcmp(rd->line, "}") == 0)
					break;
				if (p->handle(cfgobj, rd->line, rd, flags))
					return -1;
			}

			if (ret < 0)
				return -1;
			if (ret > 0)
				goto fail_bra;
		} else if (p->handle(cfgobj, value, rd, flags)) {
			return -1;
		}
	}

	return ret < 0 ? -1 : 0;
fail_bra:
	fprintf(stderr, "%s: missing '}' before end-of-file\n", rd->filename);
	return -1;
}
