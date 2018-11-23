/* SPDX-License-Identifier: ISC */
#include <string.h>

#include "util.h"

const char *enum_to_name(const enum_map_t *map, int value)
{
	size_t i;

	for (i = 0; map[i].name != NULL; ++i) {
		if (map[i].value == value)
			return map[i].name;
	}

	return NULL;
}
