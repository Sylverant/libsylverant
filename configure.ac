dnl
dnl This file is part of Sylverant PSO Server.
dnl
dnl Copyright (C) 2009, 2010, 2011, 2013, 2020, 2021 Lawrence Sebald
dnl
dnl This program is free software: you can redistribute it and/or modify
dnl it under the terms of the GNU Affero General Public License version 3
dnl as published by the Free Software Foundation.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU Affero General Public License for more details.
dnl
dnl You should have received a copy of the GNU Affero General Public License
dnl along with this program.  If not, see <http://www.gnu.org/licenses/>.

AC_PREREQ(2.61)
AC_INIT([libsylverant], [0.1.0])
AC_CONFIG_MACRO_DIR([m4])
AC_USE_SYSTEM_EXTENSIONS

AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE

AC_DISABLE_SHARED
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_PROG_GREP

echo $host_os | $GREP darwin > /dev/null && IS_OSX=yes

AC_LANG(C)

AC_PROG_CC

AC_C_BIGENDIAN
AC_C_CONST

AC_TYPE_INT8_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T

AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

AC_CHECK_FUNCS([timegm _mkgmtime])
AC_CHECK_FUNCS([strptime],,[AC_LIBOBJ([strptime])])

if test $IS_OSX; then
    test $libxml2_CFLAGS || libxml2_CFLAGS="-I/usr/include/libxml2"
    test $libxml2_LIBS || libxml2_LIBS="-lxml2"
else
    PKG_CHECK_MODULES([libxml2], [libxml-2.0 >= 2.6])
fi

CFLAGS="$CFLAGS $libxml2_CFLAGS"
LIBS="$LIBS $libxml2_LIBS"

AC_ARG_WITH([mariadb],
            [AS_HELP_STRING([--without-mariadb], [disable MariaDB support])], [],
            [with_mariadb=yes])

MARIADB_LIBS=
MARIADB_CFLAGS=

# Try to get the MariaDB stuff if the user hasn't disabled it with --without-mariadb
AS_IF([test "x$with_mariadb" != xno], [AX_CHECK_MARIADB_CLIENT()])

AM_CONDITIONAL([MARIADB], [test "x$with_mariadb" != xno])
AX_DEFINE_DIR([DATAROOTDIR], [datarootdir])

CFLAGS="$CFLAGS $MARIADB_CFLAGS"
LIBS="$LIBS $MARIADB_LIBS"

AC_CONFIG_FILES([Makefile
                 include/Makefile
                 include/sylverant/Makefile
                 src/Makefile
                 src/database/Makefile
                 src/utils/Makefile
                 src/encryption/Makefile])
AC_OUTPUT
