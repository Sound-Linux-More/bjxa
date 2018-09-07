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

=======
libbjxa
=======

------------------------------
BandJAM XA audio codec library
------------------------------

:Title upper: BJXA
:Manual section: 3

PROLOG
======

The goal of **libbjxa** is to provide a portable and clean interface to
interact with BandJAM XA audio files. It provides a decoder that can convert
any XA file to a PCM stream of 16 bits samples.

It is a replacement for the **xadec.dll** library that was shipped with
BandJAM with a license allowing to use it for free of charge projects but its
source code is unavailable and the Windows library only exists for 32 bit x86
systems.

SYNOPSIS
========

| **#include <stdint.h>**
| **#include <stdio.h>**
| **#include <unistd.h>**
|
| **#include <bjxa.h>**
|
| **bjxa_decoder_t * bjxa_decoder(void);**
| **int bjxa_free_decoder(bjxa_decoder_t \*\***\ *decp*\ **);**
|
| **ssize_t bjxa_parse_header(bjxa_decoder_t \***\ *dec*\ **,** \
      **void \***\ *ptr*\ **, size_t** *len*\ **);**
| **ssize_t bjxa_fread_header(bjxa_decoder_t \***\ *dec*\ **,** \
      **FILE \***\ *file*\ **);**

DESCRIPTION
===========

**bjxa_decoder()** allocates a decoder for a single XA file at a time.

**bjxa_free_decoder()** takes a pointer to a decoder, frees the decoder and
clears the pointer.

**bjxa_parse_header()** and **bjxa_fread_header()** parse the header of an XA
file respectively from memory or from a file. On success, the decoder is ready
to convert samples. A used decoder can parse a new XA header at any time, even
in the middle of a conversion. The state of the decoder is updated only on
success.

RETURN VALUE
============

On success, a scalar greater or equal to zero or a valid pointer is returned.

On error, -1 or **NULL** is returned, and *errno* is set appropriately.

**bjxa_parse_header()** and **bjxa_fread_header()** return the number of bytes
read. On success this value is always 32 because XA files have a fixed-size
header. On error, **bjxa_fread_header()** may have effectively read up to 32
bytes nevertheless.

ERRORS
======

**EINVAL**

	*decp* is a null pointer or is not a pointer to a valid decoder.

	*dec* is null or not a valid decoder.

	**bjxa_parse_header()** got a null *ptr*.

	**bjxa_fread_header()** got a null *file*.

**ENOBUFS**

	**bjxa_parse_header()** got a *len* lower than 32, so the memory
	buffer can't hold a complete XA header.

**ENODATA**

	**bjxa_fread_header()** couldn't read a complete XA header.

**ENOMEM**

	**bjxa_decoder()** could not allocate a decoder.

SEE ALSO
========

**bjxa**\(1)
