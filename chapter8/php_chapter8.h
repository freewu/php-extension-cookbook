/*
 * Chapter 8 - 资源（Resource）
 */

#ifndef PHP_CHAPTER8_H
#define PHP_CHAPTER8_H

extern zend_module_entry chapter8_module_entry;
#define phpext_chapter8_ptr &chapter8_module_entry

#define PHP_CHAPTER8_VERSION "1.0.0"

#if defined(ZTS) && defined(COMPILE_DL_CHAPTER8)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

#endif /* PHP_CHAPTER8_H */
