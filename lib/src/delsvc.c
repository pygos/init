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
#include <stdlib.h>

#include "service.h"

void delsvc(service_t *svc)
{
	size_t i;

	for (i = 0; i < svc->num_exec; ++i)
		free(svc->exec[i]);

	for (i = 0; i < svc->num_before; ++i)
		free(svc->before[i]);

	for (i = 0; i < svc->num_after; ++i)
		free(svc->after[i]);

	free(svc->before);
	free(svc->after);
	free(svc->name);
	free(svc->desc);
	free(svc->exec);
	free(svc->ctty);
	free(svc);
}
