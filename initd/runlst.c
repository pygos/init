#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>

#include "init.h"

extern char **environ;

static NORETURN void split_and_exec(char *cmd)
{
	char *argv[128];
	size_t i = 0;

	while (*cmd != '\0') {
		argv[i++] = cmd;	/* FIXME: buffer overflow!! */

		while (*cmd != '\0' && !isspace(*cmd))
			++cmd;

		if (isspace(*cmd)) {
			*(cmd++) = '\0';

			while (isspace(*cmd))
				++cmd;
		}
	}

	argv[i] = NULL;

	execve(argv[0], argv, environ);
	perror(argv[0]);
	exit(EXIT_FAILURE);
}

static int child_setup(const char *ctty)
{
	sigset_t mask;
	int fd;

	sigemptyset(&mask);
	sigprocmask(SIG_SETMASK, &mask, NULL);

	if (ctty != NULL) {
		fd = open(ctty, O_RDWR);
		if (fd < 0) {
			perror(ctty);
			return -1;
		}

		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);

		setsid();

		dup2(fd, STDIN_FILENO);
		dup2(fd, STDOUT_FILENO);
		dup2(fd, STDERR_FILENO);
		close(fd);
	}

	return 0;
}

int runlst_wait(char **exec, size_t num, const char *ctty)
{
	pid_t ret, pid;
	int status;
	size_t i;

	for (i = 0; i < num; ++i) {
		pid = fork();

		if (pid == 0) {
			if (child_setup(ctty))
				exit(EXIT_FAILURE);
			split_and_exec(exec[i]);
		}

		if (pid == -1) {
			perror("fork");
			return EXIT_FAILURE;
		}

		do {
			ret = waitpid(pid, &status, 0);
		} while (ret != pid);

		if (!WIFEXITED(status))
			return EXIT_FAILURE;

		if (WEXITSTATUS(status) != EXIT_SUCCESS)
			return WEXITSTATUS(status);
	}

	return EXIT_SUCCESS;
}

pid_t runlst(char **exec, size_t num, const char *ctty)
{
	int status;
	pid_t pid;

	pid = fork();

	if (pid == 0) {
		if (child_setup(ctty))
			exit(EXIT_FAILURE);

		if (num > 1) {
			status = runlst_wait(exec, num, NULL);
			exit(status);
		} else {
			split_and_exec(exec[0]);
		}
	}

	if (pid == -1)
		perror("fork");

	return pid;
}
