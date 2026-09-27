PHP_ARG_ENABLE([chapter2],
  [whether to enable chapter2 support],
  [AS_HELP_STRING([--enable-chapter2],
    [Enable chapter2 support])],
  [no])

if test "$PHP_CHAPTER2" != "no"; then
  AC_DEFINE(HAVE_CHAPTER2, 1, [ Have chapter2 support ])
  PHP_NEW_EXTENSION(chapter2, chapter2.c, $ext_shared)
fi
