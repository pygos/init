/* SPDX-License-Identifier: ISC */
#include <stdlib.h>

#include "service.h"

void del_svc_list(service_list_t *list)
{
	service_t *svc;
	int i;

	for (i = 0; i < TGT_MAX; ++i) {
		while (list->targets[i] != NULL) {
			svc = list->targets[i];
			list->targets[i] = svc->next;

			delsvc(svc);
		}
	}
}
