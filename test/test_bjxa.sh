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

. "$(dirname "$0")"/test_setup.sh

_ -----------
_ Help action
_ -----------

expect_success "Usage:" bjxa help

_ ---------
_ No action
_ ---------

expect_error "Missing an action" bjxa

_ --------------
_ Unknown action
_ --------------

expect_error "Unknown action" bjxa unknown

_ ----------------
_ Decode arguments
_ ----------------

expect_sha1 "9fa9edf0ac468129c2e73523df55095a504b8d26" \
	cat <"$TEST_DIR"/square-stereo-8.xa

expect_sha1 "4b10d39db9abfb75bb3561d7a789ca5afb046c75" \
	bjxa decode "$TEST_DIR"/square-stereo-8.xa

expect_sha1 "4b10d39db9abfb75bb3561d7a789ca5afb046c75" \
	bjxa decode "$TEST_DIR"/square-stereo-8.xa -

expect_sha1 "4b10d39db9abfb75bb3561d7a789ca5afb046c75" \
	bjxa decode - - <"$TEST_DIR"/square-stereo-8.xa

bjxa decode "$TEST_DIR"/square-stereo-8.xa "$WORK_DIR"/square-stereo-8.wav

expect_sha1 "4b10d39db9abfb75bb3561d7a789ca5afb046c75" \
	cat "$WORK_DIR"/square-stereo-8.wav

_ ------------------------
_ Invalid decode arguments
_ ------------------------

expect_error "Too many arguments" bjxa decode src.xa dst.wav jnk.arg

expect_error "Error:" bjxa decode "$WORK_DIR"/nonexistent.xa
expect_error "Error:" bjxa decode "$TEST_DIR"/square-stereo-8.xa \
	nonexistent/square-stereo-8.wav
