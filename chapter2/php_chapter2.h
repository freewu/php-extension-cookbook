/*
 * Chapter 2 - 扩展配置（INI）
 *
 * 演示如何在扩展中：
 *   - 定义 php.ini 配置项
 *   - 用模块全局变量（module globals）缓存配置值
 *   - 在函数里读取 / 运行时修改配置
 */

#ifndef PHP_CHAPTER2_H
#define PHP_CHAPTER2_H

extern zend_module_entry chapter2_module_entry;
#define phpext_chapter2_ptr &chapter2_module_entry

#define PHP_CHAPTER2_VERSION "1.0.0"

/*
 * 模块全局变量。
 * PHP_INI 的 OnUpdate 回调会把解析后的配置写进这些字段，
 * 函数里通过 CHAPTER2_G(xxx) 读取即可，避免每次都查 ini 哈希表。
 */
ZEND_BEGIN_MODULE_GLOBALS(chapter2)
    zend_bool  enabled;   /* chapter2.enabled  */
    zend_long  limit;     /* chapter2.limit    */
    char      *greeting;  /* chapter2.greeting */
    char      *name;      /* chapter2.name     */
ZEND_END_MODULE_GLOBALS(chapter2)

ZEND_EXTERN_MODULE_GLOBALS(chapter2)

/* 访问当前线程/进程的模块全局变量 */
#define CHAPTER2_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(chapter2, v)

#if defined(ZTS) && defined(COMPILE_DL_CHAPTER2)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

#endif /* PHP_CHAPTER2_H */
