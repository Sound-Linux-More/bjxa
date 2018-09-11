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

AC_INIT([bjxa], [0.1], [https://github.com/dridi/bjxa/issues])
AC_COPYRIGHT([Copyright (c) 2018 Dridi Boukelmoune])

AC_PREREQ([2.63])
AC_REVISION([revision 1])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([bjxa.c])
AC_CONFIG_AUX_DIR([build-aux])

AM_INIT_AUTOMAKE([1.12 foreign])
AM_SILENT_RULES([no])
AM_SANITY_CHECK

LT_INIT
LT_PREREQ([2.2.6])

AC_PROG_SED
AC_PROG_AWK
AC_PROG_CC_C99

BJXA_ARG_ENABLE([docs], [no])
BJXA_ARG_ENABLE([warnings], [no])
BJXA_ARG_ENABLE([hardening], [no])
BJXA_ARG_ENABLE([asan], [no])
BJXA_ARG_ENABLE([msan], [no])
BJXA_ARG_ENABLE([ubsan], [no])
BJXA_ARG_ENABLE([lcov], [no])

# Standards compliance
BJXA_CHECK_CFLAGS([CFLAGS], [
	-pedantic
	-std=c99
	-D_POSIX_C_SOURCE=200809L
])

# Documentation
AM_CONDITIONAL([DOCS], [test "$enable_docs" != "no"])
test "$enable_docs" != "no" &&
BJXA_CHECK_PROG([RST2MAN], [rst2man rst2man.py], [required to build docs])

# Dynamic analysis
test "$enable_asan" != no &&
BJXA_ADD_CFLAGS([CFLAGS], [-fno-omit-frame-pointer -fsanitize=address])

test "$enable_msan" != no &&
BJXA_ADD_CFLAGS([CFLAGS], [-fsanitize=memory])

test "$enable_ubsan" != no &&
BJXA_ADD_CFLAGS([CFLAGS], [-fsanitize=undefined])

# Coverage
AM_CONDITIONAL([LCOV_ENABLED], [test "$enable_lcov" != no])

if test "$enable_lcov" != no
then
	BJXA_CHECK_PROG([LCOV], [lcov], [code coverage])
	BJXA_CHECK_PROG([GCOV], [gcov], [required by lcov])
	BJXA_CHECK_PROG([GENHTML], [genhtml], [required by lcov])

	BJXA_ADD_CFLAGS([CFLAGS], [
		-O0 -g
		-fno-inline
		--coverage
		-DNDEBUG
	])

	enable_warnings=no

	if "$LCOV" --help | grep -q -e --config-file
	then
		LCOV_OPTS="--config-file \$(srcdir)/lcovrc"
		LCOV="$LCOV $LCOV_OPTS"
		GENHTML="$GENHTML $LCOV_OPTS"
	fi

	LCOV="$LCOV --gcov-tool '[\$](abs_top_builddir)/gcov-tool.sh'"
fi

# Warnings
if test "$enable_warnings" != no
then
	# FreeBSD's WARNS level 6
	BJXA_CHECK_CFLAGS([CFLAGS], [
		-Werror
		-Wall
		-W
		-Wstrict-prototypes
		-Wmissing-prototypes
		-Wpointer-arith
		-Wreturn-type
		-Wcast-qual
		-Wwrite-strings
		-Wswitch
		-Wshadow
		-Wunused-parameter
		-Wcast-align
		-Wchar-subscripts
		-Winline
		-Wnested-externs
		-Wredundant-decls
		-Wold-style-definition
		-Wmissing-variable-declarations
	])

	# Other desirable warnings
	BJXA_CHECK_CFLAGS([CFLAGS], [
		-Wextra
		-Wmissing-declarations
		-Wredundant-decls
		-Wsign-compare
		-Wunused-result
		-Wformat-security
	])

	# Clang unleashed-ish
	BJXA_CHECK_CFLAGS([CFLAGS], [
		-Weverything
		-Wno-padded
		-Wno-string-conversion
		-Wno-covered-switch-default
		-Wno-disabled-macro-expansion
	])

	# SunCC-specific warnings
	BJXA_CHECK_CFLAGS([CFLAGS], [
		-errwarn=%all
		-errtags=yes
	])

	# sparse(1) warnings
	BJXA_CHECK_CFLAGS([CFLAGS], [
		-Wsparse-all
		-Wsparse-error
	])
fi

# Hardening
if test "$enable_hardening" != no
then
	BJXA_CHECK_CFLAGS([CFLAGS], [
		-fstack-clash-protection
		-fstack-protector
	])
fi

# Configured files
AC_CONFIG_FILES([
	Makefile
	bjxa.3.rst
	bjxa.pc
	bjxa.spec
])

AC_OUTPUT

AS_ECHO("
	==== $PACKAGE_STRING ====

	prefix:       $prefix
	bindir:       $bindir
	libdir:       $libdir

	compiler:     $CC
	cflags:       $CFLAGS
	ldflags:      $LDFLAGS

	--enable-silent-rules=${enable_silent_rules:-no}
	--enable-docs=$enable_docs
	--enable-warnings=$enable_warnings
	--enable-hardening=$enable_hardening
	--enable-lcov=$enable_lcov
	--enable-asan=$enable_asan
	--enable-msan=$enable_msan
	--enable-ubsan=$enable_ubsan
")
