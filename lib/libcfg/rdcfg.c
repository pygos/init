/* SPDX-License-Identifier: ISC */
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

int rdcfg(void *cfgobj, rdline_t *rd, const cfg_param_t *params, size_t count)
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
				ret = p->handle(cfgobj, value, rd);
				if (ret)
					return -1;
			}

			while ((ret = rdline(rd)) == 0) {
				if (strcmp(rd->line, "}") == 0)
					break;
				if (p->handle(cfgobj, rd->line, rd))
					return -1;
			}

			if (ret < 0)
				return -1;
			if (ret > 0)
				goto fail_bra;
		} else if (p->handle(cfgobj, value, rd)) {
			return -1;
		}
	}

	return ret < 0 ? -1 : 0;
fail_bra:
	fprintf(stderr, "%s: missing '}' before end-of-file\n", rd->filename);
	return -1;
}
