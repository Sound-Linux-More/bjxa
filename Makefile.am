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

ACLOCAL_AMFLAGS = -I m4

include lcov.am
include rpm.am

# Configuration

nodist_noinst_SCRIPTS = \
	gcov-tool.sh \
	test/run.sh

CONFIG_SUFFIXES = .sh.in .sh

.sh.in.sh:
	@mkdir -p test
	$(AM_V_GEN) $(SED) \
		-e 's:[@]builddir[@]:$(abs_top_builddir):' \
		-e 's:[@]GCOV[@]:$(GCOV):' \
		-e 's:[@]SHA1[@]:$(SHA1):' \
		-e 's:[@]XXD[@]:$(XXD):' \
		<'$<' >$@
	chmod +x $@

# Binaries

lib_LTLIBRARIES = libbjxa.la
bin_PROGRAMS = bjxa
include_HEADERS = bjxa.h
noinst_HEADERS = bjxa_priv.h

libbjxa_la_LDFLAGS = -version-info 0:0:0

if WITH_LD_VERSION_SCRIPT
libbjxa_la_LDFLAGS += -Wl,--version-script=$(srcdir)/libbjxa.map
libbjxa.lo: libbjxa.map
endif

bjxa_SOURCES = bjxa.c bjxa_decode.c
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
	bjxa_dump_riff_header.3 \
	bjxa_fread_header.3 \
	bjxa_free_decoder.3 \
	bjxa_fwrite_pcm.3 \
	bjxa_fwrite_riff_header.3 \
	bjxa_parse_header.3

dist_doc_DATA = README.rst
dist_man_MANS = bjxa.1 bjxa.3 bjxa.5 $(bjxa_3_links)

if WITH_DOCUMENTATION
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

bjxa.3: bjxa_decode.c.txt
endif

# C#

if WITH_DOTNET
dotnet_binaries = \
	dotnet/bjxa.exe \
	dotnet/libbjxa.dll

dotnet/bjxa.exe: dotnet/libbjxa.dll

CSBUILD = $(AM_V_CCLD) $(CSC) $(CSFLAGS)

.cs.dll:
	@mkdir -p dotnet/
	$(CSBUILD) -target:library -out:$@ $<

.cs.exe:
	$(CSBUILD) -target:exe -r:dotnet/libbjxa.dll -out:$@ $<
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

CLEANFILES = $(noinst_SCRIPTS)

EXTRA_DIST = \
	bjxa.1.rst \
	bjxa.3.rst \
	bjxa.5.rst \
	bjxa.spec \
	bootstrap \
	dotnet/bjxa.cs \
	dotnet/libbjxa.cs \
	gcov-tool.sh.in \
	libbjxa.map \
	strip-c-source.sh \
	test/hex_decode \
	test/hex_encode \
	test/run.sh.in \
	test/square-mono-4.xa \
	test/square-mono-6.xa \
	test/square-mono-8.xa \
	test/square-stereo-4.xa \
	test/square-stereo-6.xa \
	test/square-stereo-8.xa \
	test/test_setup.sh
