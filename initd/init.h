#ifndef INIT_H
#define INIT_H

#include <linux/reboot.h>
#include <sys/reboot.h>

#include "service.h"
#include "telinit.h"
#include "util.h"

enum {
	STATUS_OK = 0,
	STATUS_FAIL,
	STATUS_WAIT,
};

int runlst_wait(char **exec, size_t num, const char *ctty);

pid_t runlst(char **exec, size_t num, const char *ctty);

int setup_tty(void);

void print_status(const char *msg, int type, bool update);

int mksock(void);

NORETURN void do_shutdown(int type);

bool svclist_have_singleshot(void);

void svclist_add(service_t *svc);

service_t *svclist_remove(pid_t pid);

#endif /* INIT_H */

