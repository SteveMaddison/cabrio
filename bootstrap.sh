#!/bin/sh -e

test -n "${srcdir}" || srcdir=`dirname "$0"`
test -n "${srcdir}" || srcdir=.

if [ ! "${NO_CHANGELOG}" ] ; then
(
	cd "${srcdir}"
	svn log > ChangeLog
)
fi

libtoolize --ltdl
autoreconf --force --install --verbose "$srcdir"
test -n "${NOCONFIGURE}" || "${srcdir}/configure" "$@"
#test -n "${NOCONFIGURE}" || "${srcdir}/configure" --enable-silent-rules --disable-dependency-tracking "$@"

