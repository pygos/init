#include <stdlib.h>

#include "service.h"

void del_srv_list(service_list_t *list)
{
	service_t *srv;
	int i;

	for (i = 0; i < TGT_MAX; ++i) {
		while (list->targets[i] != NULL) {
			srv = list->targets[i];
			list->targets[i] = srv->next;

			delsrv(srv);
		}
	}
}
