PHP_ARG_ENABLE([chapter1],
  [whether to enable chapter1 support],
  [AS_HELP_STRING([--enable-chapter1],
    [Enable chapter1 support])],
  [no])

if test "$PHP_CHAPTER1" != "no"; then
  AC_DEFINE(HAVE_CHAPTER1, 1, [ Have chapter1 support ])
  PHP_NEW_EXTENSION(chapter1, chapter1.c, $ext_shared)
fi
