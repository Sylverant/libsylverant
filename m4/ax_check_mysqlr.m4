# ===========================================================================
#     https://www.gnu.org/software/autoconf-archive/ax_check_mysqlr.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_CHECK_MYSQLR
#
# DESCRIPTION
#
#   First check if mysqlconfig exists. It fails if mysqlconfig is not in
#   path. Then it checks for the libraries and replaces -lmysqlclient
#   statement with -lmysqlclient_r statement, to enable threaded client
#   library.
#
#   The following are exported environment variables:
#
#     MYSQL_LIBS
#     MYSQL_CFLAGS
#
# LICENSE
#
#   Copyright (c) 2008 Can Bican <bican@yahoo.com>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

# Modified to look for MariaDB, not MySQL, and updated variables/functions to
# match.

#serial 6

AU_ALIAS([AC_CHECK_MARIADB_CLIENT], [AX_CHECK_MARIADB_CLIENT])
AC_DEFUN([AX_CHECK_MARIADB_CLIENT],[
AC_PATH_PROG(mariadbconfig,mariadb_config)
if test [ -z "$mariadbconfig" ]
then
    AC_MSG_ERROR([mariadb_config executable not found])
else
    AC_MSG_CHECKING(mariadb libraries)
    MARIADB_LIBS=`${mariadbconfig} --libs`
    AC_MSG_RESULT($MARIADB_LIBS)
    AC_MSG_CHECKING(mariadb includes)
    MARIADB_CFLAGS=`${mariadbconfig} --cflags`
    AC_MSG_RESULT($MARIADB_CFLAGS)
fi
])
