#include "init.h"

static service_t *running = NULL;	/* currently supervised services */
static int singleshot = 0;		/* active singleshot services */

bool svclist_have_singleshot(void)
{
	return singleshot > 0;
}

void svclist_add(service_t *svc)
{
	svc->next = running;
	running = svc;

	if (svc->type == SVC_ONCE)
		singleshot += 1;
}

service_t *svclist_remove(pid_t pid)
{
	service_t *prev = NULL, *svc = running;

	while (svc != NULL) {
		if (svc->pid == pid) {
			if (prev != NULL) {
				prev->next = svc->next;
			} else {
				running = svc->next;
			}
			svc->next = NULL;

			if (svc->type == SVC_ONCE)
				singleshot -= 1;
			break;
		}

		prev = svc;
		svc = svc->next;
	}

	return svc;
}
