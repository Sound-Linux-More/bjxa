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

_ ---------------
_ Unreadable file
_ ---------------

expect_error "bjxa_fread_header" bjxa <&1

_ ------------------
_ Wrong magic number
_ ------------------

mk_hex <<EOF
4b574432 | KWD2 (id)
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

_ -------
_ ENODATA
_ -------

mk_hex <<EOF
4b574431 | KWD1 (id)
00000000 | 0 (nDataLen)
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

_ ----------
_ ENOSAMPLES
_ ----------

mk_hex <<EOF
4b574431 | KWD1 (id)
c0680a00 | 682176 (nDataLen)
00000000 | 0 (nSamples)
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

_ ---------------
_ ETOOMANYSAMPLES
_ ---------------

mk_hex <<EOF
4b574431 | KWD1 (id)
c0680a00 | 682176 (nDataLen)
a1bb0d00 | 900001 (nSamplesPerSec)
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

_ -----------------
_ ENOTENOUGHSAMPLES
_ -----------------

mk_hex <<EOF
4b574431 | KWD1 (id)
c0680a00 | 682176 (nDataLen)
2a000000 | 42 (nSamplesPerSec)
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

_ -----------
_ EWAYTOOSLOW
_ -----------

mk_hex <<EOF
4b574431 | KWD1 (id)
c0680a00 | 682176 (nDataLen)
fc170a00 | 661500 (nSamples)
0000     | 0 (nSamplesPerSec)
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

_ -------------------
_ Unknown compression
_ -------------------

mk_hex <<EOF
4b574431 | KWD1 (id)
c0680a00 | 682176 (nDataLen)
fc170a00 | 661500 (nSamples)
44ac     | 44100 (nSamplesPerSec)
0c       | 12 (nBits)
01       | 1 (nChannels)
00000000 | 0 (nLoopPtr)
0000     | 0 (befL[0])
0000     | 0 (befL[1])
0000     | 0 (befR[0])
0000     | 0 (befR[1])
00000000 | 0 (pad)
EOF

expect_error "bjxa_fread_header" bjxa <"$WORK_DIR"/bin

_ -----------
_ Home studio
_ -----------

mk_hex <<EOF
4b574431 | KWD1 (id)
c0680a00 | 682176 (nDataLen)
fc170a00 | 661500 (nSamples)
44ac     | 44100 (nSamplesPerSec)
08       | 8 (nBits)
05       | 5 (nChannels)
00000000 | 0 (nLoopPtr)
0000     | 0 (befL[0])
0000     | 0 (befL[1])
0000     | 0 (befR[0])
0000     | 0 (befR[1])
00000000 | 0 (pad)
EOF

expect_error "bjxa_fread_header" bjxa <"$WORK_DIR"/bin
