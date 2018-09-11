#!/bin/sh
#
# Copyright (C) 2018  Dridi Boukelmoune
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
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

set -e
set -u

# Test environment

readonly TEST_SRC=$(basename "$0")
readonly TEST_DIR=$(dirname "$0")
readonly WORK_DIR=$(mktemp -d "bjxa.${TEST_SRC}.XXXXXXXX")

trap 'rm -fr "$WORK_DIR"' EXIT

# Test title

_() {
        if printf %s "$1" | grep -q -e '^-'
        then
                echo "------$*"
        else
                echo "TEST: $*"
        fi
}

# Test fixtures

expect_error() {
	msg=$1
	shift
	if "$@" >"$WORK_DIR"/stdout 2>"$WORK_DIR"/stderr
	then
		echo "expect_error: command succeeded" >&2
		return 1
	fi
	if ! grep -q "$msg" "$WORK_DIR"/stderr
	then
		echo "expect_error: message not found: $msg" >&2
		return 1
	fi
	echo "expect_error ok: $msg"
}

sha1() {
	case ${SHA1:-} in
		sha1sum) sha1sum "$1" ;;
		sha1) command sha1 -r "$1" ;;
		*) ${SHA1:-sha1sum} "$1" ;;
	esac |
	awk '{print $1}'
}

expect_sha1() {
	expect=$1
	shift
	if ! "$@" >"$WORK_DIR"/stdout 2>"$WORK_DIR"/stderr
	then
		echo "expect_sha1: command failed" >&2
		cat "$WORK_DIR"/stderr >&2
		return 1
	fi
	actual=$(sha1 "$WORK_DIR"/stdout)
	if [ "$expect" != "$actual" ]
	then
		echo "expect_sha1: expected $expect got $actual" >&2
		return 1
	fi
	echo "expect_sha1 ok: $expect"
}
