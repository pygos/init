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
#ifndef LOGFILE_H
#define LOGFILE_H

#include "proto.h"

enum {
	/*
	  Rotate log data in a way that we still generate a continuous stream
	  of log data. E.g. in the case of log files, move the current log file
	  to one suffixed with a timestamp. We don't lose any log data.
	 */
	LOG_ROTATE_CONTINUOUS = 0x00,

	/*
	  Rotate log data by overwriting old data with more recent data.
	  E.g. in the case of log files, move the current log file to one
	  with a constant prefix, overwriting any existing data.
	 */
	LOG_ROTATE_OVERWRITE = 0x01,

	/*
	  Automatically do a log rotatation if a log stream reaches a preset
	  size limit.
	 */
	LOG_ROTATE_SIZE_LIMIT = 0x10,
};

typedef struct log_backend_t {
	int (*init)(struct log_backend_t *log, int flags, size_t sizelimit);

	void (*cleanup)(struct log_backend_t *log);

	int (*write)(struct log_backend_t *log, const syslog_msg_t *msg);

	void (*rotate)(struct log_backend_t *log);
} log_backend_t;


extern log_backend_t *logmgr;


#endif /* LOGFILE_H */
