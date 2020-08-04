# CHECK_LIBIDN
# --------------------------------------------------------------
# Define --with-libidn2 and test macro HAVE_LIBIDN2
AC_DEFUN([LIBIDN2_CHECK],
[
AC_ARG_WITH(libidn2, AC_HELP_STRING([--with-libidn2=[PREFIX]],
                                [Look for libidn2 in PREFIX/lib and PREFIX/include]),
  libidn2=$withval, libidn2=yes)
if test "$libidn2" != "no"; then
  if test "$libidn2" != "yes"; then
    LDFLAGS="${LDFLAGS} -L$libidn2/lib"
    CPPFLAGS="${CPPFLAGS} -I$libidn2/include"
  fi
  AC_CHECK_HEADER(idn2.h,
    AC_CHECK_LIB(idn2, idn2_lookup_u8,
      [libidn2=yes LIBS="${LIBS} -lidn2"], libidn2=no),
    libidn2=no)
fi
if test "$libidn2" != "no" ; then
  AC_DEFINE(HAVE_LIBIDN2, 1, [Define to 1 if you want IDN2 support.])
#else
#  AC_MSG_WARN([libidn2 not found])
fi
AC_MSG_CHECKING([if libidn2 should be used])
AC_MSG_RESULT($libidn2)
])dnl
