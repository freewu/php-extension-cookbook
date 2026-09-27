/*
 * Chapter 3 - 函数参数与返回值
 *
 * 演示：
 *   - 用 ZEND_PARSE_PARAMETERS_START/END 解析参数
 *   - Z_PARAM_LONG / Z_PARAM_DOUBLE / Z_PARAM_STR / Z_PARAM_ZVAL
 *   - 返回 int / float / string
 *   - 在 C 层抛出异常
 *
 * 编译：phpize && ./configure --enable-chapter3 && make
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_operators.h"
#include "php_chapter3.h"

/* 函数声明 */
PHP_FUNCTION(chapter3_add);
PHP_FUNCTION(chapter3_concat);
PHP_FUNCTION(chapter3_upper);
PHP_FUNCTION(chapter3_divide);
PHP_FUNCTION(chapter3_type_of);

/* -------------------------------------------------------------------------
 * arginfo：PHP 8 起必须提供，同时定义参数/返回值类型
 * ---------------------------------------------------------------------- */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chapter3_add, 0, 2, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, a, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, b, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chapter3_concat, 0, 2, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, a, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, b, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chapter3_upper, 0, 1, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, s, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chapter3_divide, 0, 2, IS_DOUBLE, 0)
    ZEND_ARG_TYPE_INFO(0, a, IS_DOUBLE, 0)
    ZEND_ARG_TYPE_INFO(0, b, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chapter3_type_of, 0, 1, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry chapter3_functions[] = {
    PHP_FE(chapter3_add,     arginfo_chapter3_add)
    PHP_FE(chapter3_concat,  arginfo_chapter3_concat)
    PHP_FE(chapter3_upper,   arginfo_chapter3_upper)
    PHP_FE(chapter3_divide,  arginfo_chapter3_divide)
    PHP_FE(chapter3_type_of, arginfo_chapter3_type_of)
    PHP_FE_END
};

/* -------------------------------------------------------------------------
 * int chapter3_add(int $a, int $b)
 *
 * ZEND_PARSE_PARAMETERS_START(必需个数, 最大个数)
 * 结束用 ZEND_PARSE_PARAMETERS_END()（解析失败会自动 return）。
 * ---------------------------------------------------------------------- */
PHP_FUNCTION(chapter3_add)
{
    zend_long a, b;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_LONG(a)
        Z_PARAM_LONG(b)
    ZEND_PARSE_PARAMETERS_END();

    RETURN_LONG(a + b);
}

/* string chapter3_concat(string $a, string $b) */
PHP_FUNCTION(chapter3_concat)
{
    zend_string *a, *b;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STR(a)
        Z_PARAM_STR(b)
    ZEND_PARSE_PARAMETERS_END();

    /* zend_string_concat2 返回新的 zend_string，所有权交给 return_value */
    RETURN_STR(zend_string_concat2(
        ZSTR_VAL(a), ZSTR_LEN(a),
        ZSTR_VAL(b), ZSTR_LEN(b)));
}

/* string chapter3_upper(string $s) */
PHP_FUNCTION(chapter3_upper)
{
    zend_string *s;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(s)
    ZEND_PARSE_PARAMETERS_END();

    RETURN_STR(zend_string_toupper(s));
}

/* float chapter3_divide(float $a, float $b) */
PHP_FUNCTION(chapter3_divide)
{
    double a, b;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_DOUBLE(a)
        Z_PARAM_DOUBLE(b)
    ZEND_PARSE_PARAMETERS_END();

    if (b == 0.0) {
        /* 在 C 层抛出 PHP 异常；RETURN_THROWS() 表示已经设置了异常 */
        zend_throw_exception_ex(zend_ce_division_by_zero_error, 0,
                                "Division by zero");
        RETURN_THROWS();
    }

    RETURN_DOUBLE(a / b);
}

/* string chapter3_type_of(mixed $value) */
PHP_FUNCTION(chapter3_type_of)
{
    zval *value;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(value)
    ZEND_PARSE_PARAMETERS_END();

    RETURN_STRING(zend_zval_type_name(value));
}

/* -------------------------------------------------------------------------
 * MINFO / 模块入口
 * ---------------------------------------------------------------------- */
PHP_MINFO_FUNCTION(chapter3)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "chapter3 support", "enabled");
    php_info_print_table_row(2, "Version", PHP_CHAPTER3_VERSION);
    php_info_print_table_end();
}

zend_module_entry chapter3_module_entry = {
    STANDARD_MODULE_HEADER,
    "chapter3",
    chapter3_functions,
    NULL,
    NULL,
    NULL,
    NULL,
    PHP_MINFO(chapter3),
    PHP_CHAPTER3_VERSION,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_CHAPTER3
ZEND_GET_MODULE(chapter3)
#endif
