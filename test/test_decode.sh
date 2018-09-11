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

expect_sha1 "ce495052bca4d7eddc91aaa5e63ac186ad9ada0b" \
	bjxa <"$TEST_DIR"/square-stereo-8.xa

_ ----------
_ 8 bit mono
_ ----------

expect_sha1 "9bdaa12181696bc61a4dfd562edb64a0def2f918" \
	cat <"$TEST_DIR"/square-mono-8.xa

expect_sha1 "7338cb03ea1c097139eccb4af52f7c963d5606ca" \
	bjxa <"$TEST_DIR"/square-mono-8.xa

_ ------------
_ 6 bit stereo
_ ------------

expect_sha1 "5241ffdb22617621a6bd7ee9e16055ccb5f59875" \
	cat <"$TEST_DIR"/square-stereo-6.xa

expect_sha1 "db7431f91edcfac54ffb5a4fa7f3e86d2d2a5290" \
	bjxa <"$TEST_DIR"/square-stereo-6.xa

_ ----------
_ 6 bit mono
_ ----------

expect_sha1 "90749ddb703d17d408dd197ff6a877085b80331d" \
	cat <"$TEST_DIR"/square-mono-6.xa

expect_sha1 "c29c0be871be8f8fb6c3e0d2be32295e44d649ce" \
	bjxa <"$TEST_DIR"/square-mono-6.xa

_ ------------
_ 4 bit stereo
_ ------------

expect_sha1 "43e9ddd9afb8208f7bc84cea991fbcd27807a707" \
	cat <"$TEST_DIR"/square-stereo-4.xa

expect_sha1 "09724b96aedee75ec284895fe15790db8f13fa37" \
	bjxa <"$TEST_DIR"/square-stereo-4.xa

_ ----------
_ 4 bit mono
_ ----------

expect_sha1 "02c7ec66ecebda313097462218d9dc05e8886806" \
	cat <"$TEST_DIR"/square-mono-4.xa

expect_sha1 "e83e5141ab8b274da974eacd30a5fb0df7e99878" \
	bjxa <"$TEST_DIR"/square-mono-4.xa
