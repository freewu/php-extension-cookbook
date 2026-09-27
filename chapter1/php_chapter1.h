/*
 * Chapter 1 - Hello World
 *
 * 扩展头文件：声明模块入口、版本号，以及需要对外暴露给其它
 * C 文件（或其它扩展）使用的符号。
 */

#ifndef PHP_CHAPTER1_H
#define PHP_CHAPTER1_H

extern zend_module_entry chapter1_module_entry;
#define phpext_chapter1_ptr &chapter1_module_entry

#define PHP_CHAPTER1_VERSION "1.0.0"

#if defined(ZTS) && defined(COMPILE_DL_CHAPTER1)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

#endif /* PHP_CHAPTER1_H */
