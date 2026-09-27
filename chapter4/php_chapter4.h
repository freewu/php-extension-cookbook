/*
 * Chapter 4 - 数组参数与数组返回值
 */

#ifndef PHP_CHAPTER4_H
#define PHP_CHAPTER4_H

extern zend_module_entry chapter4_module_entry;
#define phpext_chapter4_ptr &chapter4_module_entry

#define PHP_CHAPTER4_VERSION "1.0.0"

#if defined(ZTS) && defined(COMPILE_DL_CHAPTER4)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

#endif /* PHP_CHAPTER4_H */
