/* SPDX-License-Identifier: ISC */
#include "service.h"
#include "util.h"

static const enum_map_t type_map[] = {
	{ "once", SVC_ONCE },
	{ "wait", SVC_WAIT },
	{ "respawn", SVC_RESPAWN },
	{ NULL, 0 },
};

static const enum_map_t target_map[] = {
	{ "boot", TGT_BOOT },
	{ "shutdown", TGT_SHUTDOWN },
	{ "reboot", TGT_REBOOT },
	{ NULL, 0 },
};

const char *svc_type_to_string(int type)
{
	return enum_to_name(type_map, type);
}

int svc_type_from_string(const char *type)
{
	const enum_map_t *ent = enum_by_name(type_map, type);

	return ent == NULL ? -1 : ent->value;
}

const char *svc_target_to_string(int target)
{
	return enum_to_name(target_map, target);
}

int svc_target_from_string(const char *target)
{
	const enum_map_t *ent = enum_by_name(target_map, target);

	return ent == NULL ? -1 : ent->value;
}
