/*
 * Chapter 5 - 带默认值的函数
 */

#ifndef PHP_CHAPTER5_H
#define PHP_CHAPTER5_H

extern zend_module_entry chapter5_module_entry;
#define phpext_chapter5_ptr &chapter5_module_entry

#define PHP_CHAPTER5_VERSION "1.0.0"

#if defined(ZTS) && defined(COMPILE_DL_CHAPTER5)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

#endif /* PHP_CHAPTER5_H */
