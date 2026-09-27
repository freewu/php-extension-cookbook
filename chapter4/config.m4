PHP_ARG_ENABLE([chapter4],
  [whether to enable chapter4 support],
  [AS_HELP_STRING([--enable-chapter4],
    [Enable chapter4 support])],
  [no])

if test "$PHP_CHAPTER4" != "no"; then
  AC_DEFINE(HAVE_CHAPTER4, 1, [ Have chapter4 support ])
  PHP_NEW_EXTENSION(chapter4, chapter4.c, $ext_shared)
fi
