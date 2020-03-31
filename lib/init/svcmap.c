/* SPDX-License-Identifier: ISC */
#include <string.h>
#include "service.h"

static const char *type_map[] = {
	"once",
	"wait",
	"respawn",
};

static const char *target_map[] = {
	"boot",
	"shutdown",
	"reboot",
};

const char *svc_type_to_string(int type)
{
	return type >= 0 && type < SVC_MAX ? type_map[type] : NULL;
}

int svc_type_from_string(const char *type)
{
	size_t i;

	for (i = 0; i < sizeof(type_map) / sizeof(type_map[0]); ++i) {
		if (strcmp(type_map[i], type) == 0)
			return i;
	}

	return -1;
}

const char *svc_target_to_string(int target)
{
	return target >= 0 && target < TGT_MAX ? target_map[target] : NULL;
}

int svc_target_from_string(const char *target)
{
	size_t i;

	for (i = 0; i < sizeof(target_map) / sizeof(target_map[0]); ++i) {
		if (strcmp(target_map[i], target) == 0)
			return i;
	}

	return -1;
}
