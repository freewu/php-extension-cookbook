PHP_ARG_ENABLE([chapter6],
  [whether to enable chapter6 support],
  [AS_HELP_STRING([--enable-chapter6],
    [Enable chapter6 support])],
  [no])

if test "$PHP_CHAPTER6" != "no"; then
  AC_DEFINE(HAVE_CHAPTER6, 1, [ Have chapter6 support ])
  PHP_NEW_EXTENSION(chapter6, chapter6.c, $ext_shared)
fi
