/* SPDX-License-Identifier: ISC */
#include "runsvc.h"

static int setup_tty(const char *tty, bool truncate)
{
	int fd;

	if (tty == NULL)
		return 0;

	fd = open(tty, O_RDWR);
	if (fd < 0) {
		perror(tty);
		return -1;
	}

	if (truncate)
		ftruncate(fd, 0);

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	setsid();

	dup2(fd, STDIN_FILENO);
	dup2(fd, STDOUT_FILENO);
	dup2(fd, STDERR_FILENO);
	close(fd);
	return 0;
}

static __attribute__((noreturn)) void argv_exec(exec_t *e)
{
	char **argv = alloca(sizeof(char *) * (e->argc + 1)), *ptr;
	int i;

	for (ptr = e->args, i = 0; i < e->argc; ++i, ptr += strlen(ptr) + 1)
		argv[i] = ptr;

	argv[i] = NULL;
	execvp(argv[0], argv);
	perror(argv[0]);
	exit(EXIT_FAILURE);
}

static int run_sequentially(exec_t *list)
{
	pid_t ret, pid;
	int status;

	for (; list != NULL; list = list->next) {
		if (list->next == NULL)
			argv_exec(list);

		pid = fork();

		if (pid == 0)
			argv_exec(list);

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

/*****************************************************************************/

int main(int argc, char **argv)
{
	service_t *svc = NULL;
	int dirfd;

	if (argc != 3) {
		fputs("usage: runsvc <directory> <filename>\n", stderr);
		return EXIT_FAILURE;
	}

	if (getppid() != 1) {
		fputs("must be run by init!\n", stderr);
		return EXIT_FAILURE;
	}

	dirfd = open(argv[1], O_RDONLY | O_DIRECTORY);
	if (dirfd < 0) {
		perror(argv[1]);
		return EXIT_FAILURE;
	}

	svc = rdsvc(dirfd, argv[2], RDSVC_NO_FNAME | RDSVC_NO_DEPS);
	close(dirfd);
	if (svc == NULL)
		return EXIT_FAILURE;

	if (initenv())
		return EXIT_FAILURE;

	if (setup_tty(svc->ctty, (svc->flags & SVC_FLAG_TRUNCATE_OUT) != 0))
		return EXIT_FAILURE;

	return run_sequentially(svc->exec);
}
