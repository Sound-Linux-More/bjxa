.. Copyright (C) 2018  Dridi Boukelmoune
..
.. This program is free software: you can redistribute it and/or modify
.. it under the terms of the GNU General Public License as published by
.. the Free Software Foundation, either version 3 of the License, or
.. (at your option) any later version.
..
.. This program is distributed in the hope that it will be useful,
.. but WITHOUT ANY WARRANTY; without even the implied warranty of
.. MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
.. GNU General Public License for more details.
..
.. You should have received a copy of the GNU General Public License
.. along with this program.  If not, see <http://www.gnu.org/licenses/>.

====
bjxa
====

-----------------------
BandJAM XA audio format
-----------------------

:Manual section: 5

PROLOG
======

The goal of this manual is to describe the on-disk format of BandJAM XA audio
files. It is similar to the CD-i(tm) and PlayStation(tm) ADPCM audio or CDROM
XA, with a custom file header better suited for in-memory or general purpose
storage rather than CDROM sectors. While CDROM XA is meant for real time audio
decoding, the use of XA files in BandJAM and other games was meant to allow
reasonable lossy audio compression at low latency. As a result BandJAM XA is
not bound by CDROM rotation speed and does not share the constraint of allowed
audio frequencies.

This document is the result of testing and reverse engineering of the
following files from the BandJAM project::

    78cd57226b54c2f3551b263e71946336f5532bdc  xadec.dll
    58cda874bf811bd753fc2664df8956313d9163a4  xa.exe

Checksums were computed with the SHA-1 hash function.

DESCRIPTION
===========

Unlike CDROM XA that defines quality levels A, B and C, BandJAM XA relies on
compression levels of 4, 6 and 8 bits. Unlike CDROM XA where audio data on a
sector is divided into units, further divided into blocks of either 28 (mono)
or 14 (stereo) samples, BandJAM XA divides the audio data into blocks of 32
samples with no further hierarchy. For stereo sound, blocks are interleaved
with samples from the left channels first followed by the right channel.

A block contains a profile followed by 32 ADPCM samples. The profile includes
a range parameter and a gain parameter. The gain of a sample is adjusted based
on its two preceding samples on a given channel.

All bytes are 8 bits long as per ISO/IEC 2382-1:1993 and as a result nibbles
are 4 bits long.

Header
------

The header is 32 byte longs, all multi-byte fields are encoded as little
endian integers. Signed integers are encoded using two's complement. The
header field names are taken from **xadec.h** that was shipped with
**xadec.dll** in BandJAM and other games.

**id**: a magic number to identify BandJAM XA files, the ASCII characters
*KWD1*.

**nDataLen** (unsigned, 32 bits): the total size of all ADPCM blocks. It MUST
account to complete blocks for the number of channels.

**nSamples** (unsigned, 32 bits): the number of samples per channel. It MAY
account to less samples than needed to have complete ADPCM blocks. Remaining
samples for the last per-channel blocks are ignored.

**nSamplesPerSec** (unsigned, 16 bits): the sampling frequency in Hertz.

**nBits** (unsigned, 8 bits): the number of bits per ADPCM samples. It MUST be
either 4, 6 or 8.

**nChannels** (unsigned, 8 bits): the number of audio channels. It MUST be
either 1 for mono or 2 for stereo.

**nLoopPtr** (unsigned, 32 bits): unknown.

**befL** (signed, 16 bits, twice): the initial state for the preceding samples
used with the gain factors for the left (or single) channel.

**befR** (signed, 16 bits, twice): the initial state for the preceding samples
used with the gain factors for the right channel.

**pad**: 4 bytes to align the XA header to exactly 32 bytes.

Block
-----

The first byte of a block contains the filter profile. It is composed of two
nibbles with a range parameter (bits 0 to 3) and a gain parameter (bits 4 to
7).

There are only 5 sets of gain factors, so the gain parameter can only have a
value from 0 to 4. The available gain factors K0 for the previous sample and
K1 for the one before are:

|
| |     K0    |     K1    |
| |  0.0      |  0.0      |
| |  0.9375   |  0.0      |
| |  1.796875 | -0.8125   |
| |  1.53125  | -0.858375 |
| |  1.90625  | -0.9375   |

It is possible on a system capable of 32 bit integer arithmetics to avoid
floating-point factors. With an extra division by 256 (easily optimized as a bit
shift) it is possible to multiply values from this table by 256 without losing
precision:

|
| |  K0  |  K1  |
| |    0 |    0 |
| |  240 |    0 |
| |  460 | -208 |
| |  392 | -220 |
| |  488 | -240 |

The ADPCM samples of 4, 6 or 8 bits represent the most significant bits of the
16 bits PCM samples. This can be done by shifting left the ADPCM sample by
respectively 12, 10 or 8 bits. This is the PCM sample size (always 16 bits)
minus the ADPCM sample size.

For example, consider the following X, Y, Z and T ADPCM 6 bits samples encoded
with 3 bytes:

|
| X x x x x x Y y
| y y y y Z z z z
| z z T t t t t t

Once extended, and before the range is applied, they become 16 bits PCM
samples encoded on 8 bytes:

|
| X x x x x x ? ? ? ? ? ? ? ? ? ?
| Y y y y y y ? ? ? ? ? ? ? ? ? ?
| Z z z z z z ? ? ? ? ? ? ? ? ? ?
| T t t t t t ? ? ? ? ? ? ? ? ? ?

The least significant bits indicated with question marks are undefined in this
example, because they depend on whether the most significant bit (upper case)
is set with respect to two's complement encoding of the signed integers.

The value R of the range parameter adjusts the volume of the samples of a
block. When a sample of 4, 6 or 8 bits is converted to its 16 bits target, it
is then divided by 2^R (or shifted right R bits) to reposition the most
significant bits within th 16 bits range.

For example, a range of 2 would result in the following samples:

|
| ? ? X x x x x x ? ? ? ? ? ? ? ?
| ? ? Y y y y y y ? ? ? ? ? ? ? ?
| ? ? Z z z z z z ? ? ? ? ? ? ? ?
| ? ? T t t t t t ? ? ? ? ? ? ? ?

The gain factors can then be used to adjust the lower bits of the sample and
minimize the audio loss. For an ADPCM sample A already extended to 16 bits and
preceded by PCM samples P0 and P1, the gain G and resulting PCM sample P can
be computed this way::

    G := (P0 * K0) + (P1 * K1)
    P := (A / (2^R)) + G
    P1 = P0
    P0 = P

With integer-only arithmetics, the gain difference needs to be accounted for::

    P := (A / (2^R)) + (G / 256)

The computation result for P may overflow or underflow and should be kept
within the bounds of a signed 16 bits integer before it can be used as a PCM
sample or tracked as P0. The previous samples P0 and P1 are maintained across
all the blocks of a given channel.

BUGS
====

The meaning of *nLoopPtr* is unknown.

The initial state of *befL* and *befR* may always be zero. Reverse engineering
of **xadec.dll** shows that while the header looks like a data structure that
once loaded could be used as-is, in practice it isn't and the initial state is
always as if previous samples always started at zero. Comparing the output of
**bjxa decode** with **xa.exe -d** on a set of 3700 unique XA files seems to
confirm this hypothesis. In addition, CDROM XA audio decoding assumes that
previous samples for gain adjustment are indeed starting at zero.

It is very likely that only 4 gain parameters exist. They have the same exact
values as the CDROM XA parameters, but the 5th set of gain factors looks
consistent. Finding a BandJAM XA file with a block using the 5th gain
parameter would help settle this. The 5 parameters appear to be present in
both **xadec.dll** and **xa.exe**, based on a dump of both objects.

Reverse engineering of **xa.exe** could provide a definitive answer to all the
known bugs in the specification.

SEE ALSO
========

**bjxa**\(1),
**bjxa**\(3)
