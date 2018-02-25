#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

#include "init.h"

void do_shutdown(int type)
{
	struct timespec req, rem;

	print_status("sending SIGTERM to all processes", STATUS_WAIT, false);
	kill(-1, SIGTERM);

	memset(&req, 0, sizeof(req));
	memset(&rem, 0, sizeof(rem));
	req.tv_sec = 5;		/* TODO: make configurable? */

	while (nanosleep(&req, &rem) != 0 && errno == EINTR)
		req = rem;

	print_status("sending SIGTERM to all processes", STATUS_OK, true);
	kill(-1, SIGKILL);
	print_status("sending SIGKILL to remaining processes",
		     STATUS_OK, false);

	print_status("sync", STATUS_WAIT, false);
	sync();
	print_status("sync", STATUS_OK, true);

	reboot(type);
	perror("reboot system call");
	exit(EXIT_FAILURE);
}
