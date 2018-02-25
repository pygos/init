#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>

#include "service.h"

static bool has_dependencies(service_t *list, service_t *svc)
{
	size_t i;

	while (list != NULL) {
		for (i = 0; i < svc->num_after; ++i) {
			if (!strcmp(svc->after[i], list->name))
				return true;
		}

		for (i = 0; i < list->num_before; ++i) {
			if (!strcmp(list->before[i], svc->name))
				return true;
		}

		list = list->next;
	}

	return false;
}

service_t *srv_tsort(service_t *list)
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
