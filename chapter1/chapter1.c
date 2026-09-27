/*
 * Chapter 1 - Hello World
 *
 * 最小可用的 PHP 8 扩展：
 *   - 注册一个函数 hello_world()
 *   - 调用后返回字符串 "Hello World"
 *
 * 编译：phpize && ./configure --enable-chapter1 && make
 * 运行：php -d extension=$(pwd)/modules/chapter1.so demo.php
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_chapter1.h"

/* -------------------------------------------------------------------------
 * 函数声明
 *
 * PHP_FUNCTION(name) 展开为 void zif_name(zend_execute_data *execute_data,
 * zval *return_value)。所有对用户空间暴露的函数都需要这个宏。
 * ---------------------------------------------------------------------- */
PHP_FUNCTION(hello_world);

/* -------------------------------------------------------------------------
 * 参数信息（arginfo）
 *
 * PHP 8 以后，每个函数都必须提供 arginfo，用于：
 *   - 反射（ReflectionFunction）
 *   - 类型检查 / 参数解析报错信息
 *   - 被其它扩展调用时的签名
 * 没有参数的函数也要给出一个空的 arginfo。
 * ---------------------------------------------------------------------- */
ZEND_BEGIN_ARG_INFO_EX(arginfo_hello_world, 0, 0, 0)
ZEND_END_ARG_INFO()

/* -------------------------------------------------------------------------
 * 函数表：模块名 -> 函数实现的映射
 * 最后必须以 PHP_FE_END 结束。
 * ---------------------------------------------------------------------- */
static const zend_function_entry chapter1_functions[] = {
    PHP_FE(hello_world, arginfo_hello_world)
    PHP_FE_END
};

/* -------------------------------------------------------------------------
 * hello_world() 的实现
 * ---------------------------------------------------------------------- */
PHP_FUNCTION(hello_world)
{
    /* 本函数不接受任何参数，声明后如果传参 PHP 会自动报错 */
    ZEND_PARSE_PARAMETERS_NONE();

    /* RETURN_STRING 会拷贝一份字符串到 return_value 并设置类型 */
    RETURN_STRING("Hello World");
}

/* -------------------------------------------------------------------------
 * phpinfo() 中显示的信息
 * ---------------------------------------------------------------------- */
PHP_MINFO_FUNCTION(chapter1)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "chapter1 support", "enabled");
    php_info_print_table_row(2, "Version", PHP_CHAPTER1_VERSION);
    php_info_print_table_end();
}

/* -------------------------------------------------------------------------
 * 模块入口
 *
 * STANDARD_MODULE_HEADER 会自动填入 API 版本等信息。
 * 中间四个回调依次是：MINIT / MSHUTDOWN / RINIT / RSHUTDOWN，
 * 本章还用不到，先置为 NULL。
 * ---------------------------------------------------------------------- */
zend_module_entry chapter1_module_entry = {
    STANDARD_MODULE_HEADER,
    "chapter1",              /* 模块名 */
    chapter1_functions,      /* 函数表 */
    NULL,                    /* MINIT    */
    NULL,                    /* MSHUTDOWN*/
    NULL,                    /* RINIT    */
    NULL,                    /* RSHUTDOWN*/
    PHP_MINFO(chapter1),     /* MINFO    */
    PHP_CHAPTER1_VERSION,    /* 版本号   */
    STANDARD_MODULE_PROPERTIES
};

/* 以动态库（.so）方式加载时需要 */
#ifdef COMPILE_DL_CHAPTER1
ZEND_GET_MODULE(chapter1)
#endif
