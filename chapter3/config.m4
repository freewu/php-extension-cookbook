PHP_ARG_ENABLE([chapter3],
  [whether to enable chapter3 support],
  [AS_HELP_STRING([--enable-chapter3],
    [Enable chapter3 support])],
  [no])

if test "$PHP_CHAPTER3" != "no"; then
  AC_DEFINE(HAVE_CHAPTER3, 1, [ Have chapter3 support ])
  PHP_NEW_EXTENSION(chapter3, chapter3.c, $ext_shared)
fi
