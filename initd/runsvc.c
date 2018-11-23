/* SPDX-License-Identifier: ISC */
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "init.h"

pid_t runsvc(service_t *svc)
{
	char *argv[4], *envp[1];
	pid_t pid;

	argv[0] = (char *)RUNSVCBIN;
	argv[1] = (char *)SVCDIR;
	argv[2] = svc->fname;
	argv[3] = NULL;

	envp[0] = NULL;

	pid = fork();

	if (pid == -1)
		perror("fork");

	if (pid == 0) {
		sigreset();
		execve(argv[0], argv, envp);
		perror(argv[0]);
		exit(EXIT_FAILURE);
	}

	return pid;
}
