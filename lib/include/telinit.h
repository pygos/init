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
#ifndef TELINIT_H
#define TELINIT_H

#include "config.h"

#define INITSOCK SOCKDIR "/" "initd.socket"

enum {
	TI_SHUTDOWN = 1,
	TI_REBOOT = 2,
};

typedef struct {
	int type;	/* TI_* message type identifier */
} ti_msg_t;

/* Try to connect to the init socket. */
int opensock(void);

#endif /* TELINIT_H */

