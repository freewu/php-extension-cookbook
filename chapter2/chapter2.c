/*
 * Chapter 2 - 扩展配置
 *
 * 编译：phpize && ./configure --enable-chapter2 && make
 * 运行：php -d extension=$(pwd)/modules/chapter2.so demo.php
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "Zend/zend_smart_str.h"
#include "php_chapter2.h"

/* 定义模块全局变量实例 */
ZEND_DECLARE_MODULE_GLOBALS(chapter2)

/* 函数声明 */
PHP_FUNCTION(chapter2_greet);
PHP_FUNCTION(chapter2_is_enabled);
PHP_FUNCTION(chapter2_get_config);
PHP_FUNCTION(chapter2_set_greeting);

/* arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_chapter2_greet, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_chapter2_is_enabled, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_chapter2_get_config, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chapter2_set_greeting, 0, 1, _IS_BOOL, 0)
    ZEND_ARG_TYPE_INFO(0, greeting, IS_STRING, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry chapter2_functions[] = {
    PHP_FE(chapter2_greet,        arginfo_chapter2_greet)
    PHP_FE(chapter2_is_enabled,   arginfo_chapter2_is_enabled)
    PHP_FE(chapter2_get_config,   arginfo_chapter2_get_config)
    PHP_FE(chapter2_set_greeting, arginfo_chapter2_set_greeting)
    PHP_FE_END
};

/* -------------------------------------------------------------------------
 * INI 配置项定义
 *
 * STD_PHP_INI_ENTRY(name, default, modifiable, on_modify,
 *                   mh_arg1, mh_arg2, mh_arg3)
 *   - mh_arg1/2/3 用于计算字段在模块全局结构体里的偏移量，
 *     配合 OnUpdate* 系列回调自动完成赋值。
 * ---------------------------------------------------------------------- */
PHP_INI_BEGIN()
    STD_PHP_INI_BOOLEAN("chapter2.enabled",  "1",     PHP_INI_ALL,
                        OnUpdateBool,   enabled,  zend_chapter2_globals, chapter2_globals)
    STD_PHP_INI_ENTRY  ("chapter2.limit",    "3",     PHP_INI_ALL,
                        OnUpdateLong,   limit,    zend_chapter2_globals, chapter2_globals)
    STD_PHP_INI_ENTRY  ("chapter2.greeting", "Hello", PHP_INI_ALL,
                        OnUpdateString, greeting, zend_chapter2_globals, chapter2_globals)
    STD_PHP_INI_ENTRY  ("chapter2.name",     "World", PHP_INI_ALL,
                        OnUpdateString, name,     zend_chapter2_globals, chapter2_globals)
PHP_INI_END()

/* -------------------------------------------------------------------------
 * 模块全局变量初始化
 * ---------------------------------------------------------------------- */
static PHP_GINIT_FUNCTION(chapter2)
{
#if defined(COMPILE_DL_CHAPTER2) && defined(ZTS)
    ZEND_TSRMLS_CACHE_UPDATE();
#endif
    /* 先清零；真正的默认值由 REGISTER_INI_ENTRIES() 写入 */
    memset(chapter2_globals, 0, sizeof(*chapter2_globals));
}

/* -------------------------------------------------------------------------
 * 生命周期回调
 * ---------------------------------------------------------------------- */
PHP_MINIT_FUNCTION(chapter2)
{
    REGISTER_INI_ENTRIES();
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(chapter2)
{
    UNREGISTER_INI_ENTRIES();
    return SUCCESS;
}

/* -------------------------------------------------------------------------
 * 函数实现
 * ---------------------------------------------------------------------- */

/* string chapter2_greet() */
PHP_FUNCTION(chapter2_greet)
{
    ZEND_PARSE_PARAMETERS_NONE();

    if (!CHAPTER2_G(enabled)) {
        RETURN_EMPTY_STRING();
    }

    /* 用 limit 控制重复次数，展示数值型配置的读取 */
    zend_long limit = CHAPTER2_G(limit);
    if (limit < 1) {
        limit = 1;
    }

    zend_string *greeting = zend_string_init(CHAPTER2_G(greeting), strlen(CHAPTER2_G(greeting)), 0);
    zend_string *name     = zend_string_init(CHAPTER2_G(name), strlen(CHAPTER2_G(name)), 0);

    smart_str buf = {0};
    for (zend_long i = 0; i < limit; i++) {
        if (i > 0) {
            smart_str_appendl(&buf, ", ", 2);
        }
        smart_str_append(&buf, greeting);
        smart_str_appendc(&buf, ' ');
        smart_str_append(&buf, name);
    }
    smart_str_0(&buf);

    zend_string_release(greeting);
    zend_string_release(name);

    RETURN_STR(buf.s);
}

/* bool chapter2_is_enabled() */
PHP_FUNCTION(chapter2_is_enabled)
{
    ZEND_PARSE_PARAMETERS_NONE();
    RETURN_BOOL(CHAPTER2_G(enabled));
}

/* string chapter2_get_config() */
PHP_FUNCTION(chapter2_get_config)
{
    ZEND_PARSE_PARAMETERS_NONE();

    char *buf;
    size_t len = spprintf(&buf, 0,
        "enabled=%d; limit=" ZEND_LONG_FMT "; greeting=%s; name=%s",
        CHAPTER2_G(enabled) ? 1 : 0,
        CHAPTER2_G(limit),
        CHAPTER2_G(greeting) ? CHAPTER2_G(greeting) : "",
        CHAPTER2_G(name)     ? CHAPTER2_G(name)     : "");

    RETVAL_STRINGL(buf, len);
    efree(buf);
}

/* bool chapter2_set_greeting(string $greeting) */
PHP_FUNCTION(chapter2_set_greeting)
{
    zend_string *greeting;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(greeting)
    ZEND_PARSE_PARAMETERS_END();

    /* 运行时修改 INI，等价于 ini_set('chapter2.greeting', ...) */
    zend_string *key = zend_string_init("chapter2.greeting", sizeof("chapter2.greeting") - 1, 0);
    zend_result result = zend_alter_ini_entry_chars(
        key, ZSTR_VAL(greeting), ZSTR_LEN(greeting),
        PHP_INI_USER, PHP_INI_STAGE_RUNTIME);
    zend_string_release(key);

    if (result == SUCCESS) {
        RETURN_TRUE;
    }
    RETURN_FALSE;
}

/* -------------------------------------------------------------------------
 * MINFO
 * ---------------------------------------------------------------------- */
PHP_MINFO_FUNCTION(chapter2)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "chapter2 support", "enabled");
    php_info_print_table_row(2, "Version", PHP_CHAPTER2_VERSION);
    php_info_print_table_end();

    DISPLAY_INI_ENTRIES();
}

/* -------------------------------------------------------------------------
 * 模块入口（带 globals）
 * ---------------------------------------------------------------------- */
zend_module_entry chapter2_module_entry = {
    STANDARD_MODULE_HEADER,
    "chapter2",
    chapter2_functions,
    PHP_MINIT(chapter2),
    PHP_MSHUTDOWN(chapter2),
    NULL,
    NULL,
    PHP_MINFO(chapter2),
    PHP_CHAPTER2_VERSION,
    PHP_MODULE_GLOBALS(chapter2),
    PHP_GINIT(chapter2),
    NULL,
    NULL,
    STANDARD_MODULE_PROPERTIES_EX
};

#ifdef COMPILE_DL_CHAPTER2
ZEND_GET_MODULE(chapter2)
#endif
