/* SPDX-License-Identifier: ISC */
#include <stdio.h>

#include "init.h"

int sigsetup(void)
{
	sigset_t mask;
	int sfd;

	sigfillset(&mask);
	if (sigprocmask(SIG_SETMASK, &mask, NULL) == -1) {
		perror("sigprocmask");
		return -1;
	}

	sigemptyset(&mask);
	sigaddset(&mask, SIGCHLD);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGTERM);
	sigaddset(&mask, SIGUSR1);

	sfd = signalfd(-1, &mask, SFD_CLOEXEC);
	if (sfd == -1) {
		perror("signalfd");
		return -1;
	}

	if (reboot(LINUX_REBOOT_CMD_CAD_OFF))
		perror("cannot disable CTRL+ALT+DEL");

	return sfd;
}

void sigreset(void)
{
	sigset_t mask;

	sigemptyset(&mask);
	sigprocmask(SIG_SETMASK, &mask, NULL);
}
