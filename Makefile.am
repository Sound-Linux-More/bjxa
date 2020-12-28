# Copyright (C) 2018-2020  Dridi Boukelmoune
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

ACLOCAL_AMFLAGS = -I m4

include lcov.am
include rpm.am

# Binaries

lib_LTLIBRARIES = libbjxa.la
bin_PROGRAMS = bjxa
include_HEADERS = bjxa.h
noinst_HEADERS = bjxa_priv.h

libbjxa_la_LDFLAGS = -version-info 2:0:2
libbjxa_la_DEPENDENCIES = $(include_HEADERS) $(noinst_HEADERS)

if WITH_LD_VERSION_SCRIPT
libbjxa_la_LDFLAGS += -Wl,--version-script=$(srcdir)/libbjxa.map
libbjxa_la_DEPENDENCIES += libbjxa.map
endif

bjxa_SOURCES = bjxa.c bjxa_decode.c bjxa_encode.c
bjxa_LDADD = libbjxa.la

# Packaging

pkgconfigdir = $(libdir)/pkgconfig

dist_pkgconfig_DATA = bjxa.pc

# Documentation

bjxa_3_links = \
	bjxa_decode.3 \
	bjxa_decode_format.3 \
	bjxa_decoder.3 \
	bjxa_dump_pcm.3 \
	bjxa_dump_header.3 \
	bjxa_dump_riff_header.3 \
	bjxa_encode.3 \
	bjxa_encode_format.3 \
	bjxa_encode_init.3 \
	bjxa_encoder.3 \
	bjxa_fread_header.3 \
	bjxa_fread_riff_header.3 \
	bjxa_free_decoder.3 \
	bjxa_free_encoder.3 \
	bjxa_fwrite_header.3 \
	bjxa_fwrite_pcm.3 \
	bjxa_fwrite_riff_header.3 \
	bjxa_parse_header.3 \
	bjxa_parse_riff_header.3

dist_doc_DATA = README.rst
dist_man_MANS = bjxa.1 bjxa.3 bjxa.5 $(bjxa_3_links)

if MAINTAINER_MODE
DOCUMENTATION_SUFFIXES = \
	.1 .1.rst \
	.3 .3.rst \
	.5 .5.rst \
	.c .c.txt

.c.c.txt:
	$(AM_V_GEN) $(srcdir)/strip-c-source.sh $< $@

.1.rst.1 .3.rst.3 .5.rst.5:
	$(AM_V_GEN) $(RST2MAN) --strict $< $@

$(bjxa_3_links):
	$(AM_V_GEN) printf ".so man3/%s\n" bjxa.3 >$@

bjxa.3: bjxa_decode.c.txt bjxa_encode.c.txt
endif

# C#

if WITH_DOTNET
dotnet_binaries = \
	dotnet/bjxa.exe \
	dotnet/libbjxa.dll

dotnet/bjxa.exe: dotnet/libbjxa.dll

CSBUILD = $(AM_V_CCLD) $(CSC) $(CSFLAGS)

.cs.dll:
	$(CSBUILD) -target:library -out:$@ $<

.cs.exe:
	$(CSBUILD) -target:exe -r:dotnet/libbjxa.dll -out:$@ $<

DOTNET_DIST = dotnet/bjxa-$(VERSION).zip

$(DOTNET_DIST): $(dotnet_binaries)
	@rm -f $@
	$(AM_V_GEN) zip -jq $@ \
		$(dotnet_binaries) \
		$(srcdir)/dotnet/*.cs \
		$(srcdir)/gpl-3.0.txt \
		$(srcdir)/lgpl-2.1.txt

all-local: $(DOTNET_DIST)
endif

noinst_SCRIPTS = $(dotnet_binaries)

# Test suite

AM_TESTS_ENVIRONMENT = SRCDIR="$(srcdir)"
LOG_COMPILER = ./test/run.sh

dist_check_SCRIPTS = \
	test/test_bjxa.sh \
	test/test_decode.sh \
	test/test_decode_error.sh

check_PROGRAMS = \
	test/test_libbjxa_api

test_test_libbjxa_api_LDADD = libbjxa.la

TESTS = $(dist_check_SCRIPTS) $(check_PROGRAMS)

all-local: $(TESTS)

# Distribution

SUFFIXES = $(CONFIG_SUFFIXES) $(DOCUMENTATION_SUFFIXES)

DISTCHECK_CONFIGURE_FLAGS = --disable-docs
DISTCLEANFILES = $(nodist_noinst_SCRIPTS) $(dist_man_MANS)

CLEANFILES = $(noinst_SCRIPTS) $(DOTNET_DIST)

EXTRA_DIST = \
	bjxa.1.rst \
	bjxa.3.rst \
	bjxa.5.rst \
	bjxa.spec \
	dotnet/bjxa.cs \
	dotnet/libbjxa.cs \
	libbjxa.map \
	strip-c-source.sh \
	test/hex_decode \
	test/hex_encode \
	test/square-mono-4.xa \
	test/square-mono-6.xa \
	test/square-mono-8.xa \
	test/square-stereo-4.xa \
	test/square-stereo-6.xa \
	test/square-stereo-8.xa \
	test/test_setup.sh
