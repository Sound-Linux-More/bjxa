#!/bin/sh

set -e
set -u

strip_source() {
	awk '
		BEGIN { strip = license = 0 }
		$0 == "/* begin strip */" { strip = 2 }
		$1 == "/*-" { license = 2 }
		{ if (strip + license == 0) print }
		strip == 1 { strip = 0 }
		license == 1 { license = 0 }
		$0 == "/* end strip */" { strip = 1 }
		$1 == "*/" && license = 2 { license = 1 }
	'
}

test $# -eq 2 || {
	printf 'Usage: %s source destination' "$0" >&2
	exit 1
}


DST=$(mktemp)

if strip_source <"$1" >"$DST"
then
	mv "$DST" "$2"
else
	rm -f "$DST"
fi
