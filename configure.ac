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
