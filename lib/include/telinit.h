#ifndef TELINIT_H
#define TELINIT_H

#include "config.h"

#define INITSOCK SOCKDIR "/" "initd.socket"

enum {
	TI_SHUTDOWN = 1,
	TI_REBOOT = 2,
};

typedef struct {
	int type;
} ti_msg_t;

int opensock(void);

#endif /* TELINIT_H */

