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

# BJXA_ARG_ENABLE(FEATURE)
# ------------------------
AC_DEFUN([BJXA_ARG_ENABLE], [dnl
	AC_ARG_ENABLE([m4_translit([$1], [ ], [-])],
		[AS_HELP_STRING([--enable-m4_translit([$1], [ ], [-])],
			[enable $1])],
		[],
		[enable_[]m4_translit([$1], [ ], [_])=no])
])

# BJXA_ARG_WITH(FEATURE)
# ----------------------
AC_DEFUN([BJXA_ARG_WITH], [dnl
	AC_ARG_WITH([m4_translit([$1], [ ], [-])],
		[AS_HELP_STRING([--with-m4_translit([$1], [ ], [-])],
			[with $1])],
		[],
		[with_[]m4_translit([$1], [ ], [_])=no])
])

# BJXA_ARG_WITHOUT(FEATURE)
# -------------------------
AC_DEFUN([BJXA_ARG_WITHOUT], [dnl
	AC_ARG_WITH([m4_translit([$1], [ ], [-])],
		[AS_HELP_STRING([--without-m4_translit([$1], [ ], [-])],
			[without $1])],
		[],
		[with_[]m4_translit([$1], [ ], [_])=yes])
])

# _BJXA_CHECK_CFLAGS
# ------------------
AC_DEFUN_ONCE([_BJXA_CHECK_CFLAGS], [

bjxa_check_cflag() {
	bjxa_save_CFLAGS=$CFLAGS
	CFLAGS="$bjxa_chkd_cflags $[]1 $bjxa_orig_cflags"

	mv confdefs.h confdefs.h.orig
	touch confdefs.h

	AC_MSG_CHECKING([whether the compiler accepts $[]1])
	AC_RUN_IFELSE(
		[AC_LANG_SOURCE([int main(void) { return (0); }])],
		[bjxa_result=yes],
		[bjxa_result=no])
	AC_MSG_RESULT($bjxa_result)

	rm confdefs.h
	mv confdefs.h.orig confdefs.h

	CFLAGS=$bjxa_save_CFLAGS

	test "$bjxa_result" = yes
}

bjxa_check_cflags() {
	for _cflag
	do
		if bjxa_check_cflag $_cflag
		then
			bjxa_chkd_cflags="$bjxa_chkd_cflags $_cflag"
		fi
	done
}

])

# _BJXA_INIT_CFLAGS(VARIABLE)
# ---------------------------
AC_DEFUN([_BJXA_INIT_CFLAGS], [dnl
if test -z "$bjxa_orig_cflags_$1"
then
	bjxa_orig_cflags_$1="orig:$$1"
	bjxa_chkd_cflags_$1=
fi
])

# BJXA_CHECK_CFLAGS(VARIABLE, CFLAGS)
# -----------------------------------
AC_DEFUN([BJXA_CHECK_CFLAGS], [dnl
AC_REQUIRE([_BJXA_CHECK_CFLAGS])dnl
{
_BJXA_INIT_CFLAGS([$1])dnl
bjxa_orig_cflags=${bjxa_orig_cflags_$1#orig:}
bjxa_chkd_cflags=${bjxa_chkd_cflags_$1# }
bjxa_check_cflags m4_normalize([$2])
$1="$bjxa_chkd_cflags $bjxa_orig_cflags"
bjxa_chkd_cflags_$1=$bjxa_chkd_cflags
}
])

# BJXA_ADD_CFLAGS(VARIABLE, CFLAGS)
# ---------------------------------
AC_DEFUN([BJXA_ADD_CFLAGS], [dnl
AC_REQUIRE([_BJXA_CHECK_CFLAGS])dnl
{
_BJXA_INIT_CFLAGS([$1])dnl
bjxa_orig_cflags_$1="$bjxa_orig_cflags_$1 m4_normalize([$2])"
CFLAGS="$bjxa_chkd_cflags_$1 ${bjxa_orig_cflags_$1#orig:}"
}
])

# BJXA_CHECK_LIB(LIBRARY, FUNCTION[, SEARCH-LIBS])
# ------------------------------------------------
AC_DEFUN([BJXA_CHECK_LIB], [
bjxa_save_LIBS="$LIBS"
LIBS=""
AC_SEARCH_LIBS([$2], [$1 $3], [], [AC_MSG_ERROR([Could not find $2 support])])
AC_SUBST(m4_toupper([$1])_LIBS, [$LIBS])
LIBS="$bjxa_save_LIBS"
])

# BJXA_CHECK_PROG(VARIABLE, PROGS, DESC)
# --------------------------------------
AC_DEFUN([BJXA_CHECK_PROG], [
AC_ARG_VAR([$1], [$3])
{
AC_CHECK_PROGS([$1], [$2], [no])
test "$[$1]" = no &&
AC_MSG_ERROR([Could not find program $2])
}
])

# BJXA_COND_PROG(VARIABLE, PROGS, DESC)
# --------------------------------------
AC_DEFUN([BJXA_COND_PROG], [
AC_ARG_VAR([$1], [$3])
AC_CHECK_PROGS([$1], [$2], [no])
AM_CONDITIONAL([HAVE_$1], [test "$$1" != no])
])

# BJXA_COND_MODULE(VARIABLE, MODULE)
# -----------------------------------
AC_DEFUN([BJXA_COND_MODULE], [
PKG_CHECK_MODULES([$1], [$2], [$1=yes], [$1=no])
AC_SUBST([$1])
AM_CONDITIONAL([HAVE_$1], [test "$$1" = yes])
])
