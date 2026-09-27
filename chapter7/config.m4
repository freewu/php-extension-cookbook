PHP_ARG_ENABLE([chapter7],
  [whether to enable chapter7 support],
  [AS_HELP_STRING([--enable-chapter7],
    [Enable chapter7 support])],
  [no])

if test "$PHP_CHAPTER7" != "no"; then
  AC_DEFINE(HAVE_CHAPTER7, 1, [ Have chapter7 support ])
  PHP_NEW_EXTENSION(chapter7, chapter7.c, $ext_shared)
fi
