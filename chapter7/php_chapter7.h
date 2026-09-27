/*
 * Chapter 7 - 实现类
 */

#ifndef PHP_CHAPTER7_H
#define PHP_CHAPTER7_H

extern zend_module_entry chapter7_module_entry;
#define phpext_chapter7_ptr &chapter7_module_entry

#define PHP_CHAPTER7_VERSION "1.0.0"

#if defined(ZTS) && defined(COMPILE_DL_CHAPTER7)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

#endif /* PHP_CHAPTER7_H */
