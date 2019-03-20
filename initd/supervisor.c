/* SPDX-License-Identifier: ISC */
#include "init.h"

static service_list_t cfg;

static int target = -1;
static service_t *running = NULL;
static service_t *terminated = NULL;
static service_t *queue = NULL;
static service_t *completed = NULL;
static service_t *failed = NULL;
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
				goto out_failure;
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
		if (svc->status != EXIT_SUCCESS)
			goto out_failure;
		break;
	case SVC_ONCE:
		singleshot -= 1;
		print_status(svc->desc,
			     svc->status == EXIT_SUCCESS ?
			     STATUS_OK : STATUS_FAIL, false);
		if (singleshot == 0 && queue == NULL && !waiting)
			target_completed(target);
		if (svc->status != EXIT_SUCCESS)
			goto out_failure;
		break;
	}
	svc->next = completed;
	completed = svc;
	return;
out_failure:
	svc->next = failed;
	failed = svc;
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

	if (!(svc->flags & SVC_FLAG_HAS_EXEC)) {
		print_status(svc->desc, STATUS_OK, false);
		svc->status = EXIT_SUCCESS;
		svc->next = completed;
		completed = svc;
		goto out;
	}

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
out:
	if (singleshot == 0 && queue == NULL && !waiting)
		target_completed(target);
	return true;
}

static int send_svc_list(int fd, const void *dst, size_t addrlen,
			 E_SERVICE_STATE state, service_t *list)
{
	while (list != NULL) {
		if (init_socket_send_status(fd, dst, addrlen, state, list))
			return -1;

		list = list->next;
	}

	return 0;
}

void supervisor_answer_status_request(int fd, const void *dst, size_t addrlen)
{
	if (send_svc_list(fd, dst, addrlen, ESS_RUNNING, running))
		return;
	if (send_svc_list(fd, dst, addrlen, ESS_DONE, completed))
		return;
	if (send_svc_list(fd, dst, addrlen, ESS_FAILED, failed))
		return;
	if (send_svc_list(fd, dst, addrlen, ESS_ENQUEUED, queue))
		return;
	if (send_svc_list(fd, dst, addrlen, ESS_ENQUEUED, terminated))
		return;
	init_socket_send_status(fd, dst, addrlen, ESS_NONE, NULL);
}
