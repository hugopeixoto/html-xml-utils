# AC_FLEX_OPTIMIZE
# --------------------------------------------------------------
# Check whether we can use option -Cfe to optimize the lexer
AC_DEFUN([AC_FLEX_OPTIMIZE],
[case "$ac_cv_prog_LEX" in
  *flex) lex_opt_flags=-Cfe;;
esac])
