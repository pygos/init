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
#ifndef PROTO_H
#define PROTO_H

#include <sys/types.h>
#include <time.h>

typedef struct {
	int facility;
	int level;
	time_t timestamp;
	pid_t pid;
	const char *ident;
	const char *message;
} syslog_msg_t;

int syslog_msg_parse(syslog_msg_t *msg, char *str);

#endif /* PROTO_H */
