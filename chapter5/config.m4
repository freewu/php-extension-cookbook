PHP_ARG_ENABLE([chapter5],
  [whether to enable chapter5 support],
  [AS_HELP_STRING([--enable-chapter5],
    [Enable chapter5 support])],
  [no])

if test "$PHP_CHAPTER5" != "no"; then
  AC_DEFINE(HAVE_CHAPTER5, 1, [ Have chapter5 support ])
  PHP_NEW_EXTENSION(chapter5, chapter5.c, $ext_shared)
fi
