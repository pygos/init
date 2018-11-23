/* SPDX-License-Identifier: ISC */
#ifndef RUNSVC_H
#define RUNSVC_H

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

#include "service.h"
#include "libcfg.h"
#include "util.h"

#define ENVFILE ETCPATH "/initd.env"

int initenv(void);

#endif /* RUNSVC_H */
