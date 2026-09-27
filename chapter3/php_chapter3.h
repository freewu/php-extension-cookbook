/*
 * Chapter 3 - 函数参数与返回值
 */

#ifndef PHP_CHAPTER3_H
#define PHP_CHAPTER3_H

extern zend_module_entry chapter3_module_entry;
#define phpext_chapter3_ptr &chapter3_module_entry

#define PHP_CHAPTER3_VERSION "1.0.0"

#if defined(ZTS) && defined(COMPILE_DL_CHAPTER3)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

#endif /* PHP_CHAPTER3_H */
