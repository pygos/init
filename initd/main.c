/* SPDX-License-Identifier: ISC */
#include "init.h"

static int sigfd = -1;
static int sockfd = -1;

static void handle_signal(void)
{
	struct signalfd_siginfo info;
	int status;
	pid_t pid;

	if (read(sigfd, &info, sizeof(info)) != sizeof(info)) {
		perror("read on signal fd");
		return;
	}

	switch (info.ssi_signo) {
	case SIGCHLD:
		while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
			status = WIFEXITED(status) ? WEXITSTATUS(status) :
						     EXIT_FAILURE;

			supervisor_handle_exited(pid, status);
		}
		break;
	case SIGTERM:
		supervisor_set_target(TGT_SHUTDOWN);
		break;
	case SIGINT:
		supervisor_set_target(TGT_REBOOT);
		break;
	case SIGHUP:
		supervisor_reload_config();
		break;
	case SIGUSR1:
		if (sockfd >= 0) {
			close(sockfd);
			unlink(INIT_SOCK_PATH);
			sockfd = -1;
		}
		sockfd = init_socket_create();
		break;
	}
}

static void handle_request(void)
{
	struct sockaddr_un addr;
	init_request_t rq;
	socklen_t addrlen;
	ssize_t ret;
retry:
	memset(&rq, 0, sizeof(rq));
	addrlen = sizeof(addr);
	ret = recvfrom(sockfd, &rq, sizeof(rq), MSG_DONTWAIT | MSG_TRUNC,
		       (struct sockaddr *)&addr, &addrlen);

	if (ret < 0 && errno == EINTR)
		goto retry;

	if ((size_t)ret < sizeof(rq))
		return;

	switch (rq.rq) {
	case EIR_STATUS:
		supervisor_answer_status_request(sockfd, &addr, addrlen,
						 rq.arg.status.filter);
		break;
	case EIR_START:
		rq.arg.startstop.id = be32toh(rq.arg.startstop.id);
		supervisor_start(rq.arg.startstop.id);
		break;
	case EIR_STOP:
		rq.arg.startstop.id = be32toh(rq.arg.startstop.id);
		supervisor_stop(rq.arg.startstop.id);
		break;
	}
}

void target_completed(int target)
{
	switch (target) {
	case TGT_BOOT:
		if (sockfd < 0)
			sockfd = init_socket_create();
		break;
	case TGT_SHUTDOWN:
		for (;;)
			reboot(RB_POWER_OFF);
		break;
	case TGT_REBOOT:
		for (;;)
			reboot(RB_AUTOBOOT);
		break;
	}
}

static int sigsetup(void)
{
	sigset_t mask;
	int sfd;

	sigfillset(&mask);
	if (sigprocmask(SIG_SETMASK, &mask, NULL) == -1) {
		perror("sigprocmask");
		return -1;
	}

	sfd = signalfd(-1, &mask, SFD_CLOEXEC);
	if (sfd == -1) {
		perror("signalfd");
		return -1;
	}

	if (reboot(LINUX_REBOOT_CMD_CAD_OFF))
		perror("cannot disable CTRL+ALT+DEL");

	return sfd;
}

int main(void)
{
	int i, ret, count;
	struct pollfd pfd[2];

	if (getpid() != 1) {
		fputs("init does not have pid 1, terminating!\n", stderr);
		return EXIT_FAILURE;
	}

	supervisor_init();

	sigfd = sigsetup();
	if (sigfd < 0)
		return -1;

	for (;;) {
		while (supervisor_process_queues())
			;

		memset(pfd, 0, sizeof(pfd));
		count = 0;

		pfd[count].fd = sigfd;
		pfd[count].events = POLLIN;
		++count;

		if (sockfd >= 0) {
			pfd[count].fd = sockfd;
			pfd[count].events = POLLIN;
			++count;
		}

		ret = poll(pfd, count, -1);
		if (ret <= 0)
			continue;

		for (i = 0; i < count; ++i) {
			if (pfd[i].revents & POLLIN) {
				if (pfd[i].fd == sigfd)
					handle_signal();
				if (pfd[i].fd == sockfd)
					handle_request();
			}
		}
	}

	return EXIT_SUCCESS;
}
