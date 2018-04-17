#!/bin/sh -uef

# A small helper script taken from mtd-utils to generate a release tar ball.

fatal() {
	printf "Error: %s\n" "$1" >&2
	exit 1
}

usage() {
	cat <<EOF
Usage: ${0##*/} <new_ver> <outdir>

<new_ver>  - mtd utils version to create in X.Y[.Z[-rcX]] format
<outdir>   - the output directory where to store the tarball
EOF
	exit 0
}

[ $# -eq 0 ] && usage
[ $# -eq 2 ] || fatal "Insufficient or too many argumetns"

new_ver="$1"
outdir="$2"

release_name="init-$new_ver"
tag_name="v$new_ver"

# Make sure the input is sane and the makefile contains sensible version
VER_REGEX="[0-9]\+.[0-9]\+\(.[0-9]\+\)\?\(-rc[0-9]\+\)\?"

echo "$new_ver" | grep -q -x "$VER_REGEX" ||
        fatal "please, provide new version in X.Y[.Z][-rcX] format"

grep -q -x "m4_define(\[RELEASE\], $VER_REGEX)" configure.ac ||
        fatal "configure.ac does not contain a valid version string"

# Make sure the git index is up-to-date
[ -z "$(git status --porcelain)" ] || fatal "Git index is not up-to-date"

# Make sure the tag does not exist
[ -z "$(git tag -l "$tag_name")" ] || fatal "Tag $tag_name already exists"

# Change the version in the configure.ac
sed -i -e "s/^m4_define(\[RELEASE\], $VER_REGEX)/m4_define([RELEASE], $new_ver)/" configure.ac

# And commit the change
git commit -m "Release $release_name" configure.ac

# Create new tag
echo "Signing tag $tag_name"
git tag -m "$release_name" "$tag_name"

# Prepare tarball
./autogen.sh
./configure
make distcheck
mkdir -p "$outdir"
mv "$release_name.tar.xz" "$outdir"
