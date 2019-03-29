/* SPDX-License-Identifier: ISC */
#include <stdlib.h>

#include "initsock.h"

void free_init_status(init_status_t *resp)
{
	free(resp->filename);
	free(resp->service_name);
}
