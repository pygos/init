#include <stdlib.h>

#include "service.h"

void delsrv(service_t *srv)
{
	size_t i;

	for (i = 0; i < srv->num_exec; ++i)
		free(srv->exec[i]);

	for (i = 0; i < srv->num_before; ++i)
		free(srv->before[i]);

	for (i = 0; i < srv->num_after; ++i)
		free(srv->after[i]);

	free(srv->before);
	free(srv->after);
	free(srv->name);
	free(srv->desc);
	free(srv->exec);
	free(srv->ctty);
	free(srv);
}
