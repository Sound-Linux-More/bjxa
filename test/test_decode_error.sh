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

_ ----------
_ Empty file
_ ----------

expect_error "bjxa_fread_header" bjxa </dev/null

_ ------------------
_ Wrong magic number
_ ------------------

mk_hex <<EOF
4b574432 | KWD2 (magic)
c0680a00 | 682176 (nDataLen)
fc170a00 | 661500 (nSamples)
44ac     | 44100 (nSamplesPerSec)
08       | 8 (nBits)
01       | 1 (nChannels)
00000000 | 0 (nLoopPtr)
0000     | 0 (befL[0])
0000     | 0 (befL[1])
0000     | 0 (befR[0])
0000     | 0 (befR[1])
00000000 | 0 (pad)
EOF

expect_error "bjxa_fread_header" bjxa <"$WORK_DIR"/bin
