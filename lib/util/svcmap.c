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
