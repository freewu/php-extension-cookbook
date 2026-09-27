PHP_ARG_ENABLE([chapter8],
  [whether to enable chapter8 support],
  [AS_HELP_STRING([--enable-chapter8],
    [Enable chapter8 support])],
  [no])

if test "$PHP_CHAPTER8" != "no"; then
  AC_DEFINE(HAVE_CHAPTER8, 1, [ Have chapter8 support ])
  PHP_NEW_EXTENSION(chapter8, chapter8.c, $ext_shared)
fi
