/* SPDX-License-Identifier: ISC */
#include <stdio.h>

#include "init.h"

void print_status(const char *msg, int type, bool update)
{
	const char *str;

	switch (type) {
	case STATUS_FAIL:
		str = "\033[22;31mFAIL\033[0m";
		break;
	case STATUS_WAIT:
		str = "\033[22;33m .. \033[0m";
		break;
	case STATUS_STARTED:
		str = "\033[22;32m UP \033[0m";
		break;
	default:
		str = "\033[22;32m OK \033[0m";
		break;
	}

	if (update)
		fputc('\r', stdout);
	printf("[%s] %s", str, msg);
	if (type != STATUS_WAIT)
		fputc('\n', stdout);
	fflush(stdout);
}
