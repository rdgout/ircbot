/*******************************************************************************
 * ircbot                                                                      *
 * Copyright (C) 2013 Rick Gout                                                *
 *                                                                             *
 * This program is free software; you can redistribute it and/or               *
 * modify it under the terms of the GNU General Public License                 *
 * as published by the Free Software Foundation; either version 2              *
 * of the License, or (at your option) any later version.                      *
 *                                                                             *
 * This program is distributed in the hope that it will be useful,             *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               *
 * GNU General Public License for more details.                                *
 *                                                                             *
 * You should have received a copy of the GNU General Public License           *
 * along with this program; if not, write to the Free Software                 *
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA. *
 *******************************************************************************/

AC_PREREQ([2.61])
AC_INIT([ircbot], [1.0], [metroidz@gmail.com])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([config])
m4_ifndef([AM_SILENT_RULES], [m4_define([AM_SILENT_RULES],[])])
AM_SILENT_RULES([yes])

AM_INIT_AUTOMAKE([ircbot], [$PACKAGE_VERSION])
AM_CONFIG_HEADER([config.h])

dnl SED is mandatory for configure process and libtool.
dnl Set it now, allowing it to be changed later.
AC_PATH_PROG([SED], [sed], [not_found],
  [$PATH:/usr/bin:/usr/local/bin])
if test -z "$SED" || test "$SED" = "not_found"; then
  AC_MSG_ERROR([sed not found in PATH. Cannot continue without sed.])
fi
AC_SUBST([SED])

PACKAGE=ircbot
VERSION=`$SED -ne 's/^#define IRCBOT_VERSION "\(.*\)"/\1/p' ircbot_version.h`

AC_PREFIX_DEFAULT([\${HOME}/ircbot])

AC_PROG_MAKE_SET
AC_PROG_CC
AC_PROG_INSTALL
AM_PROG_CC_C_O
AC_PROG_LIBTOOL

LT_INIT([dlopen disable-static win32-dll])

AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h arpa/nameser.h arpa/nameser_compat.h fcntl.h limits.h netdb.h netinet/in.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h unistd.h])

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
