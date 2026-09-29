PHP_ARG_ENABLE([chapter9],
  [whether to enable chapter9 support],
  [AS_HELP_STRING([--enable-chapter9],
    [Enable chapter9 support])],
  [no])

if test "$PHP_CHAPTER9" != "no"; then
  AC_DEFINE(HAVE_CHAPTER9, 1, [ Have chapter9 support ])
  PHP_NEW_EXTENSION(chapter9, chapter9.c, $ext_shared)
fi