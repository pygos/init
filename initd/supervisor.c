/* SPDX-License-Identifier: ISC */
#include "init.h"

static service_list_t cfg;

static int target = -1;
static service_t *running = NULL;
static service_t *terminated = NULL;
static service_t *queue = NULL;
static service_t *completed = NULL;
static int singleshot = 0;
static bool waiting = false;

static int start_service(service_t *svc)
{
	svc->pid = runsvc(svc);
	if (svc->pid == -1) {
		print_status(svc->desc, STATUS_FAIL, false);
		svc->next = completed;
		completed = svc;
		return -1;
	}

	svc->next = running;
	running = svc;
	return 0;
}

static void handle_terminated_service(service_t *svc)
{
	switch (svc->type) {
	case SVC_RESPAWN:
		if (target == TGT_REBOOT || target == TGT_SHUTDOWN)
			break;

		if (svc->rspwn_limit > 0) {
			svc->rspwn_limit -= 1;

			if (svc->rspwn_limit == 0) {
				print_status(svc->desc, STATUS_FAIL, false);
				break;
			}
		}

		start_service(svc);
		return;
	case SVC_WAIT:
		waiting = false;
		print_status(svc->desc,
			     svc->status == EXIT_SUCCESS ?
			     STATUS_OK : STATUS_FAIL, true);
		if (singleshot == 0 && queue == NULL)
			target_completed(target);
		break;
	case SVC_ONCE:
		singleshot -= 1;
		print_status(svc->desc,
			     svc->status == EXIT_SUCCESS ?
			     STATUS_OK : STATUS_FAIL, false);
		if (singleshot == 0 && queue == NULL && !waiting)
			target_completed(target);
		break;
	}
	svc->next = completed;
	completed = svc;
}

void supervisor_handle_exited(pid_t pid, int status)
{
	service_t *prev = NULL, *svc = running;

	while (svc != NULL && svc->pid != pid) {
		prev = svc;
		svc = svc->next;
	}

	if (svc == NULL)
		return;

	if (prev != NULL) {
		prev->next = svc->next;
	} else {
		running = svc->next;
	}

	svc->status = status;
	svc->next = terminated;
	terminated = svc;
}

void supervisor_set_target(int next)
{
	service_t *svc;

	if (target == TGT_REBOOT || target == TGT_SHUTDOWN || next == target)
		return;

	if (next == TGT_REBOOT || next == TGT_SHUTDOWN) {
		while (queue != NULL) {
			svc = queue;
			queue = queue->next;
			delsvc(svc);
		}
	}

	if (queue != NULL) {
		for (svc = queue; svc->next != NULL; svc = svc->next)
			;
		svc->next = cfg.targets[next];
	} else {
		queue = cfg.targets[next];
	}

	cfg.targets[next] = NULL;
	target = next;
}

void supervisor_init(void)
{
	int status = STATUS_OK;

	if (svcscan(SVCDIR, &cfg, RDSVC_NO_EXEC | RDSVC_NO_CTTY))
		status = STATUS_FAIL;

	target = TGT_BOOT;
	queue = cfg.targets[TGT_BOOT];
	cfg.targets[TGT_BOOT] = NULL;

	print_status("reading configuration from " SVCDIR, status, false);
}

bool supervisor_process_queues(void)
{
	service_t *svc;

	if (terminated != NULL) {
		svc = terminated;
		terminated = terminated->next;

		handle_terminated_service(svc);
		return true;
	}

	if (waiting || queue == NULL)
		return false;

	svc = queue;
	queue = queue->next;

	if (start_service(svc) != 0)
		return true;

	switch (svc->type) {
	case SVC_WAIT:
		print_status(svc->desc, STATUS_WAIT, false);
		waiting = true;
		break;
	case SVC_RESPAWN:
		print_status(svc->desc, STATUS_STARTED, false);
		break;
	case SVC_ONCE:
		singleshot += 1;
		break;
	}

	if (singleshot == 0 && queue == NULL && !waiting)
		target_completed(target);
	return true;
}
