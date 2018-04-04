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
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>

#include "service.h"

static bool has_dependencies(service_t *list, service_t *svc)
{
	size_t i;

	while (list != NULL) {
		if (svc->after != NULL) {
			for (i = 0; svc->after[i] != NULL; ++i) {
				if (!strcmp(svc->after[i], list->name))
					return true;
			}
		}

		if (list->before != NULL) {
			for (i = 0; list->before[i] != NULL; ++i) {
				if (!strcmp(list->before[i], svc->name))
					return true;
			}
		}

		list = list->next;
	}

	return false;
}

service_t *svc_tsort(service_t *list)
{
	service_t *nl = NULL, *end = NULL;
	service_t *svc, *prev;

	while (list != NULL) {
		/* remove first service without dependencies */
		prev = NULL;
		svc = list;

		while (svc != NULL) {
			if (has_dependencies(list, svc)) {
				prev = svc;
				svc = svc->next;
			} else {
				if (prev != NULL) {
					prev->next = svc->next;
				} else {
					list = svc->next;
				}
				svc->next = NULL;
				break;
			}
		}

		/* cycle! */
		if (svc == NULL) {
			if (end == NULL) {
				nl = list;
			} else {
				end->next = list;
			}
			errno = ELOOP;
			break;
		}

		/* append to new list */
		if (end == NULL) {
			nl = end = svc;
		} else {
			end->next = svc;
			end = svc;
		}
	}

	return nl;
}
