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
code is unavailable and the Windows executable only exists for 32 bit x86
systems.

SYNOPSIS
========

| **bjxa** help
| **bjxa** decode [*xa-file* [*wav-file*]]

DESCRIPTION
===========

Decode one XA file and convert it into a WAV file.

With no *xa-file*, or when *xa-file* is **-**, the XA file is read from the
standard input.

With no *wav-file*, or when *wav-file* is **-**, the WAV file is written to
the standard output.

EXAMPLE
=======

Play an XA file from the command line::

    bjxa decode snare.xa | play -

SEE ALSO
========

**bjxa**\(3),
**bjxa**\(5)
