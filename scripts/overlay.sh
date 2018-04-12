#!/bin/sh

lower=/cfg/preserve/${1}
upper=/cfg/overlay/${1}
work=/cfg/overlay/${1}_work
target=${2}

if [ ! -d "$target" ]; then
	exit
fi

if [ -d "$lower" ]; then
	if [ -d "$upper" ]; then
		mkdir -p "$work"
		mount -t overlay overlay \
			-olowerdir=${lower},upperdir=${upper},workdir=${work} \
			${target}
	else
		mount --bind "$lower" "$target"
	fi
fi
