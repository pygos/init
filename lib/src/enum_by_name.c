#include <string.h>

#include "util.h"

const enum_map_t *enum_by_name(const enum_map_t *map, const char *name)
{
	size_t i;

	for (i = 0; map[i].name != NULL; ++i) {
		if (!strcmp(map[i].name, name))
			return map + i;
	}

	return NULL;
}
