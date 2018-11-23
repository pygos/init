/* SPDX-License-Identifier: ISC */
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>

#include "service.h"

static bool has_dependencies(service_t *list, service_t *svc)
{
	const char *ptr;
	int i;

	while (list != NULL) {
		for (ptr = svc->after, i = 0; i < svc->num_after; ++i) {
			if (!strcmp(ptr, list->name))
				return true;
			ptr += strlen(ptr) + 1;
		}

		for (ptr = list->before, i = 0; i < list->num_before; ++i) {
			if (!strcmp(ptr, svc->name))
				return true;
			ptr += strlen(ptr) + 1;
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
