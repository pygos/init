#!/bin/sh
#
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Copyright (C) 2018 - David Oberhollenzer
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#

resolve() {
	local domain="$1"
	local server="$2"

	if [ -x "$(command -v dig)" ]; then
		if [ -z "$server" ]; then
			dig +short "$domain"
		else
			dig +short "@$server" "$domain"
		fi
		return $?
	fi

	if [ -x "$(command -v drill)" ]; then
		if [ -z "$server" ]; then
			drill "$domain" | grep "^${domain}." | cut -d$'\t' -f5
		else
			drill "@$server" "$domain" | grep "^${domain}." |\
				cut -d$'\t' -f5
		fi
		return $?
	fi
	exit 1
}

try_update() {
	while read ip; do
		if ntpdate -bu "$ip"; then
			return 0
		fi
	done

	return 1
}

pool="pool.ntp.org"
dns="1.1.1.1"

# try default DNS server first
resolve "$pool" "" | try_update
[ $? -eq 0 ] && exit 0

# try fallback public dns server
ping -q -c 1 "$dns" || exit 1

resolve "$pool" "$dns" | try_update
exit $?
