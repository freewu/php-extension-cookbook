/*
 * Chapter 6 - 无固定参数（可变参数）函数
 */

#ifndef PHP_CHAPTER6_H
#define PHP_CHAPTER6_H

extern zend_module_entry chapter6_module_entry;
#define phpext_chapter6_ptr &chapter6_module_entry

#define PHP_CHAPTER6_VERSION "1.0.0"

#if defined(ZTS) && defined(COMPILE_DL_CHAPTER6)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

#endif /* PHP_CHAPTER6_H */
