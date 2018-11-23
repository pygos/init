/* SPDX-License-Identifier: ISC */
#include <stdlib.h>

#include "service.h"

void delsvc(service_t *svc)
{
	exec_t *e;

	if (svc == NULL)
		return;

	while (svc->exec != NULL) {
		e = svc->exec;
		svc->exec = e->next;

		free(e);
	}

	free(svc->before);
	free(svc->after);
	free(svc->fname);
	free(svc->desc);
	free(svc->exec);
	free(svc->ctty);
	free(svc);
}
