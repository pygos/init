#!/bin/sh

if [ -d "$1" ]; then
	if grep -qsE "[[:space:]]+$2$" "/proc/filesystems"; then
		mount -n -t "$2" -o "$3" "$2" "$1"
	fi
fi
