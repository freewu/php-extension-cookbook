/*
 * Chapter 9 - 敏感词检测（DFA / Aho-Corasick）
 */

#ifndef PHP_CHAPTER9_H
#define PHP_CHAPTER9_H

extern zend_module_entry chapter9_module_entry;
#define phpext_chapter9_ptr &chapter9_module_entry

#define PHP_CHAPTER9_VERSION "1.0.0"

ZEND_BEGIN_MODULE_GLOBALS(chapter9)
    char *bin_path;   /* chapter9.bin 配置项：词典路径 */
    void *trie;       /* c9_trie *：加载到内存的字典（实现见 chapter9.c） */
ZEND_END_MODULE_GLOBALS(chapter9)

ZEND_EXTERN_MODULE_GLOBALS(chapter9)
#define CHAPTER9_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(chapter9, v)

#if defined(ZTS) && defined(COMPILE_DL_CHAPTER9)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

#endif /* PHP_CHAPTER9_H */