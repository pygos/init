/* SPDX-License-Identifier: ISC */
#include <sys/types.h>
#include <sys/stat.h>
#include <stddef.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>

#include "service.h"

int svcscan(const char *directory, service_list_t *list, int flags)
{
	int i, dfd, type, ret = 0;
	struct dirent *ent;
	const char *ptr;
	service_t *svc;
	struct stat sb;
	DIR *dir;

	for (i = 0; i < TGT_MAX; ++i)
		list->targets[i] = NULL;

	dir = opendir(directory);
	if (dir == NULL) {
		perror(directory);
		return -1;
	}

	dfd = dirfd(dir);
	if (dfd < 0) {
		perror(directory);
		closedir(dir);
		return -1;
	}

	for (;;) {
		errno = 0;
		ent = readdir(dir);

		if (ent == NULL) {
			if (errno != 0) {
				perror(directory);
				ret = -1;
			}
			break;
		}

		for (ptr = ent->d_name; isalnum(*ptr) || *ptr == '_'; ++ptr)
			;

		if (*ptr != '\0' && *ptr != '@')
			continue;

		if (fstatat(dfd, ent->d_name, &sb, AT_SYMLINK_NOFOLLOW)) {
			fprintf(stderr, "stat %s/%s: %s\n",
				directory, ent->d_name, strerror(errno));
			ret = -1;
			continue;
		}

		type = (sb.st_mode & S_IFMT);

		if (type != S_IFREG && type != S_IFLNK)
			continue;

		svc = rdsvc(dfd, ent->d_name, flags);
		if (svc == NULL) {
			ret = -1;
			continue;
		}

		svc->next = list->targets[svc->target];
		list->targets[svc->target] = svc;
	}

	for (i = 0; i < TGT_MAX; ++i) {
		if (list->targets[i] == NULL)
			continue;

		errno = 0;
		list->targets[i] = svc_tsort(list->targets[i]);

		if (errno != 0) {
			fprintf(stderr, "sorting services read from %s: %s\n",
				directory, strerror(errno));
		}
	}

	closedir(dir);
	return ret;
}
