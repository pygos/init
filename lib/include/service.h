#ifndef SERVICE_H
#define SERVICE_H

#include <sys/types.h>

enum {
	SVC_ONCE = 0,
	SVC_WAIT,
	SVC_RESPAWN,
};

enum {
	TGT_BOOT = 0,
	TGT_SHUTDOWN,
	TGT_REBOOT,
	TGT_CAD,

	TGT_MAX
};

typedef struct service_t {
	int type;		/* SVC_* service type */
	int target;		/* TGT_* service target */
	char *name;		/* canonical service name */
	char *desc;		/* description string */
	char **exec;		/* command lines to execute */
	size_t num_exec;	/* number of command lines */
	char *ctty;		/* controlling tty or log file */

	char **before;
	size_t num_before;
	char **after;
	size_t num_after;

	pid_t pid;
	int status;		/* process exit status */

	struct service_t *next;
} service_t;

typedef struct {
	service_t *targets[TGT_MAX];
} service_list_t;

/*
	Read a service from a file.
*/
service_t *rdsrv(int dirfd, const char *filename);

void delsrv(service_t *srv);

int srvscan(const char *directory, service_list_t *list);

void del_srv_list(service_list_t *list);

/*
	Sort a list of services by dependencies.
*/
service_t *srv_tsort(service_t *list);

#endif /* SERVICE_H */

