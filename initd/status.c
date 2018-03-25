/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Copyright (C) 2018 - David Oberhollenzer
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
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
