# CHECK_LIBIDN
# --------------------------------------------------------------
# Define --with-libidn and test macro HAVE_LIBIDN
AC_DEFUN([LIBIDN_CHECK],
[
AC_ARG_WITH(libidn, AC_HELP_STRING([--with-libidn=[PREFIX]],
                                [Look for libidn in PREFIX/lib and PREFIX/include]),
  libidn=$withval, libidn=yes)
if test "$libidn" != "no"; then
  if test "$libidn" != "yes"; then
    LDFLAGS="${LDFLAGS} -L$libidn/lib"
    CPPFLAGS="${CPPFLAGS} -I$libidn/include"
  fi
  AC_CHECK_HEADER(idna.h,
    AC_CHECK_LIB(idn, stringprep_check_version,
      [libidn=yes LIBS="${LIBS} -lidn"], libidn=no),
    libidn=no)
fi
if test "$libidn" != "no" ; then
  AC_DEFINE(HAVE_LIBIDN, 1, [Define to 1 if you want IDN support.])
#else
#  AC_MSG_WARN([libidn not found])
fi
AC_MSG_CHECKING([if libidn should be used])
AC_MSG_RESULT($libidn)
])dnl
