#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_PREREQ([2.69])
AC_INIT([html-xml-utils],[7.8])
dnl print all automake warnings with -Wall
dnl http://sources.redhat.com/automake/automake.html#Options
AM_INIT_AUTOMAKE([-Wall])
AC_CONFIG_SRCDIR([cexport.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIRS([m4])

# Checks for programs.
AC_PROG_YACC
AC_PROG_CC
AC_PROG_MAKE_SET
AM_PROG_LEX
# AC_PROG_INSTALL
# AC_PROG_CPP
# AC_PROG_LN_S
# AC_PROG_AWK
# AC_PROG_MAN2HTML

# Checks for libraries.
AC_SEARCH_LIBS(socket, socket nsl)
AC_SEARCH_LIBS(gethostbyname, socket nsl)
AM_ICONV
LIBIDN2_CHECK
LIBIDN_CHECK
LIBCURL_CHECK_CONFIG(yes, 7.9.7)

# Checks for header files.
AC_HEADER_STDC
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([arpa/inet.h errno.h fcntl.h inttypes.h libintl.h locale.h malloc.h netdb.h netinet/in.h stddef.h stdlib.h string.h strings.h sys/param.h sys/socket.h sys/time.h unistd.h search.h wchar.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STRERROR_R
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([atexit getcwd memchr gethostbyname memmove memset regcomp select setlocale socket strcasecmp strchr strcspn strdup strerror strncasecmp strndup strpbrk strrchr strspn strstr strtol strtoul fopencookie])
AC_REPLACE_FUNCS(strdup strerror strstr tsearch tfind twalk)

# Check for library variables
CHECK_GETOPT_OPTRESET

# Optimization flags for flex
AC_FLEX_OPTIMIZE
AC_SUBST(lex_opt_flags)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

if test "$libidn2" = "no" && test "$libidn" = "no"; then
  AC_MSG_WARN([Neither libidn2 nor libidn found])
fi   
