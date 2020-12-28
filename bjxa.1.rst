.. Copyright (C) 2018-2020  Dridi Boukelmoune
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

------------------------------
BandJAM XA audio codec program
------------------------------

:Manual section: 1

PROLOG
======

The goal of **bjxa** is to provide a clean command line interface to interact
with BandJAM XA audio files.

It is a replacement for the **xa.exe** program that was shipped with BandJAM
with a license allowing to use it for free of charge projects but its source
code is unavailable and the Windows executable only exists for 32 bits x86
systems.

SYNOPSIS
========

| **bjxa** help
| **bjxa** decode [*xa-file* [*wav-file*]]
| **bjxa** encode [--bits <*4|6|8*>] [*wav-file* [*xa-file*]]

DESCRIPTION
===========

Convert files between BandJAM XA audio and WAV audio with 16bit PCM samples.

When either *xa-file* or *wav-file* is omitted or **-**, the XA or WAV file
is either read from the standard input or written to the standard output
depending on the **decode** or **encode** command.

For encoding, the **--bits** specifies the number of bits per XA samples and
the default is 6 when omitted. XA audio can have either 4, 6 or 8 bits per
sample. Encoding is partially implemented.

EXAMPLE
=======

Play an XA file from the command line::

    bjxa decode snare.xa | play -

SEE ALSO
========

**bjxa**\(3),
**bjxa**\(5)
