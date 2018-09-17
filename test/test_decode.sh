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

_ ------------
_ 8 bit stereo
_ ------------

expect_sha1 "9fa9edf0ac468129c2e73523df55095a504b8d26" \
	cat <"$TEST_DIR"/square-stereo-8.xa

expect_sha1 "4b10d39db9abfb75bb3561d7a789ca5afb046c75" \
	bjxa decode <"$TEST_DIR"/square-stereo-8.xa

_ ----------
_ 8 bit mono
_ ----------

expect_sha1 "9bdaa12181696bc61a4dfd562edb64a0def2f918" \
	cat <"$TEST_DIR"/square-mono-8.xa

expect_sha1 "1c7bdc2f42bd87ebaceb8184312a1857a9f6d8de" \
	bjxa decode <"$TEST_DIR"/square-mono-8.xa

_ ------------
_ 6 bit stereo
_ ------------

expect_sha1 "5241ffdb22617621a6bd7ee9e16055ccb5f59875" \
	cat <"$TEST_DIR"/square-stereo-6.xa

expect_sha1 "96eac5430bb7a73dc4801449684a4844b9b917c8" \
	bjxa decode <"$TEST_DIR"/square-stereo-6.xa

_ ----------
_ 6 bit mono
_ ----------

expect_sha1 "90749ddb703d17d408dd197ff6a877085b80331d" \
	cat <"$TEST_DIR"/square-mono-6.xa

expect_sha1 "ce3991eda98db098e45e876944d8324302726a66" \
	bjxa decode <"$TEST_DIR"/square-mono-6.xa

_ ------------
_ 4 bit stereo
_ ------------

expect_sha1 "43e9ddd9afb8208f7bc84cea991fbcd27807a707" \
	cat <"$TEST_DIR"/square-stereo-4.xa

expect_sha1 "35d8815e712737824c61a02f603145594c0827b7" \
	bjxa decode <"$TEST_DIR"/square-stereo-4.xa

_ ----------
_ 4 bit mono
_ ----------

expect_sha1 "02c7ec66ecebda313097462218d9dc05e8886806" \
	cat <"$TEST_DIR"/square-mono-4.xa

expect_sha1 "064c48434d77d41c7df3030f3e4a85972dcbac80" \
	bjxa decode <"$TEST_DIR"/square-mono-4.xa
