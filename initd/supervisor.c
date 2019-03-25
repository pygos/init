/* SPDX-License-Identifier: ISC */
#include "init.h"

static service_list_t cfg;

static int service_id = 1;
static int target = -1;
static service_t *running = NULL;
static service_t *terminated = NULL;
static service_t *queue = NULL;
static service_t *completed = NULL;
static service_t *failed = NULL;
static int singleshot = 0;
static bool waiting = false;

static bool find_service(service_t *list, service_t *svc)
{
	while (list != NULL) {
		if (strcmp(list->fname, svc->fname) == 0)
			return true;

		list = list->next;
	}
	return false;
}

static void remove_not_in_list(service_t **current, service_t *list, int tgt)
{
	service_t *it = *current, *prev = NULL;

	while (it != NULL) {
		if (it->target == tgt && !find_service(list, it)) {
			if (prev == NULL) {
				delsvc(it);
				*current = (*current)->next;
				it = *current;
			} else {
				prev->next = it->next;
				delsvc(it);
				it = prev->next;
			}
		} else {
			prev = it;
			it = it->next;
		}
	}
}

static bool have_service(service_t *svc)
{
	return find_service(running, svc) || find_service(terminated, svc) ||
		find_service(queue, svc) || find_service(completed, svc) ||
		find_service(failed, svc);
}

static int start_service(service_t *svc)
{
	if (svc->id < 1)
		svc->id = service_id++;

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

		if (svc->flags & SVC_FLAG_ADMIN_STOPPED)
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

void supervisor_reload_config(void)
{
	service_list_t newcfg;
	service_t *svc;
	int i;

	if (svcscan(SVCDIR, &newcfg, RDSVC_NO_EXEC | RDSVC_NO_CTTY))
		return;

	for (i = 0; i < TGT_MAX; ++i) {
		if (cfg.targets[i] == NULL) {
			remove_not_in_list(&queue, newcfg.targets[i], i);
			remove_not_in_list(&terminated, newcfg.targets[i], i);
			remove_not_in_list(&completed, newcfg.targets[i], i);
			remove_not_in_list(&failed, newcfg.targets[i], i);

			while (newcfg.targets[i] != NULL) {
				svc = newcfg.targets[i];
				newcfg.targets[i] = svc->next;

				if (have_service(svc)) {
					delsvc(svc);
				} else {
					svc->id = service_id++;
					svc->status = EXIT_SUCCESS;
					svc->next = completed;
					completed = svc;
				}
			}
		} else {
			svc = cfg.targets[i];
			cfg.targets[i] = newcfg.targets[i];
			newcfg.targets[i] = svc;
		}
	}

	del_svc_list(&newcfg);
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
			 E_SERVICE_STATE filter, E_SERVICE_STATE state,
			 service_t *list)
{
	if (filter != ESS_NONE && filter != state)
		return 0;

	while (list != NULL) {
		if (init_socket_send_status(fd, dst, addrlen, state, list))
			return -1;

		list = list->next;
	}

	return 0;
}

void supervisor_answer_status_request(int fd, const void *dst, size_t addrlen,
				      E_SERVICE_STATE filter)
{
	if (send_svc_list(fd, dst, addrlen, filter, ESS_RUNNING, running))
		return;
	if (send_svc_list(fd, dst, addrlen, filter, ESS_DONE, completed))
		return;
	if (send_svc_list(fd, dst, addrlen, filter, ESS_FAILED, failed))
		return;
	if (send_svc_list(fd, dst, addrlen, filter, ESS_ENQUEUED, queue))
		return;
	if (send_svc_list(fd, dst, addrlen, filter, ESS_ENQUEUED, terminated))
		return;
	init_socket_send_status(fd, dst, addrlen, ESS_NONE, NULL);
}

void supervisor_start(int id)
{
	service_t *svc;

	for (svc = completed; svc != NULL; svc = svc->next) {
		if (svc->id == id)
			goto found;
	}

	for (svc = failed; svc != NULL; svc = svc->next) {
		if (svc->id == id)
			goto found;
	}
	return;
found:
	if (svc->type == SVC_RESPAWN)
		svc->rspwn_limit = 0;

	svc->flags &= ~SVC_FLAG_ADMIN_STOPPED;
	svc->next = queue;
	queue = svc;
}

void supervisor_stop(int id)
{
	service_t *svc;

	for (svc = running; svc != NULL; svc = svc->next) {
		if (svc->id == id)
			break;
	}

	if (svc != NULL) {
		/* TODO: something more sophisticated? */
		svc->flags |= SVC_FLAG_ADMIN_STOPPED;
		kill(svc->pid, SIGTERM);
	}
}
