/*
 * Chapter 6 - 无固定参数（可变参数）函数
 *
 * 演示：
 *   - Z_PARAM_VARIADIC('*', args, argc)        收集位置可变参数
 *   - Z_PARAM_VARIADIC_WITH_NAMED(...)         收集命名（具名）参数
 *   - 可变参数与普通参数混用
 *
 * 编译：phpize && ./configure --enable-chapter6 && make
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "Zend/zend_smart_str.h"
#include "Zend/zend_operators.h"
#include "php_chapter6.h"

PHP_FUNCTION(chapter6_sum);
PHP_FUNCTION(chapter6_join);
PHP_FUNCTION(chapter6_pack);
PHP_FUNCTION(chapter6_named);

/* arginfo：用 ZEND_ARG_VARIADIC_INFO 表示 ...$args */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chapter6_sum, 0, 0, IS_DOUBLE, 0)
    ZEND_ARG_VARIADIC_INFO(0, numbers)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chapter6_join, 0, 1, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, glue, IS_STRING, 0)
    ZEND_ARG_VARIADIC_INFO(0, pieces)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chapter6_pack, 0, 0, IS_ARRAY, 0)
    ZEND_ARG_VARIADIC_INFO(0, values)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chapter6_named, 0, 0, IS_ARRAY, 0)
    ZEND_ARG_VARIADIC_INFO(0, args)
ZEND_END_ARG_INFO()

static const zend_function_entry chapter6_functions[] = {
    PHP_FE(chapter6_sum,   arginfo_chapter6_sum)
    PHP_FE(chapter6_join,  arginfo_chapter6_join)
    PHP_FE(chapter6_pack,  arginfo_chapter6_pack)
    PHP_FE(chapter6_named, arginfo_chapter6_named)
    PHP_FE_END
};

/* -------------------------------------------------------------------------
 * float chapter6_sum(...$numbers)
 *
 * argc 是可变参数个数；args 是一个连续的 zval 数组（不是 HashTable）。
 * ---------------------------------------------------------------------- */
PHP_FUNCTION(chapter6_sum)
{
    zval    *args = NULL;
    uint32_t argc = 0;

    ZEND_PARSE_PARAMETERS_START(0, -1)     /* -1 表示参数个数不限 */
        Z_PARAM_VARIADIC('*', args, argc)
    ZEND_PARSE_PARAMETERS_END();

    double sum = 0.0;
    for (uint32_t i = 0; i < argc; i++) {
        sum += zval_get_double(&args[i]);
    }
    RETURN_DOUBLE(sum);
}

/* -------------------------------------------------------------------------
 * string chapter6_join(string $glue, ...$pieces)
 *
 * 普通参数 + 可变参数混用：先解析固定参数，再收集剩余参数。
 * ---------------------------------------------------------------------- */
PHP_FUNCTION(chapter6_join)
{
    zend_string *glue;
    zval        *pieces = NULL;
    uint32_t     argc   = 0;

    ZEND_PARSE_PARAMETERS_START(1, -1)
        Z_PARAM_STR(glue)
        Z_PARAM_VARIADIC('*', pieces, argc)
    ZEND_PARSE_PARAMETERS_END();

    smart_str buf = {0};
    for (uint32_t i = 0; i < argc; i++) {
        if (i > 0) {
            smart_str_append(&buf, glue);
        }
        /* 把任意标量/对象转成字符串 */
        zend_string *piece = zval_get_string(&pieces[i]);
        smart_str_append(&buf, piece);
        zend_string_release(piece);
    }
    smart_str_0(&buf);

    if (buf.s == NULL) {
        RETURN_EMPTY_STRING();
    }
    RETURN_STR(buf.s);
}

/* -------------------------------------------------------------------------
 * array chapter6_pack(...$values)
 *
 * 把可变参数打包成数组返回，等价于 func_get_args()。
 * ---------------------------------------------------------------------- */
PHP_FUNCTION(chapter6_pack)
{
    zval    *args = NULL;
    uint32_t argc = 0;

    ZEND_PARSE_PARAMETERS_START(0, -1)
        Z_PARAM_VARIADIC('*', args, argc)
    ZEND_PARSE_PARAMETERS_END();

    array_init_size(return_value, argc);

    for (uint32_t i = 0; i < argc; i++) {
        zval copy;
        ZVAL_COPY(&copy, &args[i]);
        add_next_index_zval(return_value, &copy);
    }
}

/* -------------------------------------------------------------------------
 * array chapter6_named(...$args)
 *
 * PHP 8 支持具名参数。Z_PARAM_VARIADIC_WITH_NAMED 会把
 * “未匹配到形参的具名实参”放进 extra_named_params 哈希表。
 * ---------------------------------------------------------------------- */
PHP_FUNCTION(chapter6_named)
{
    zval       *args  = NULL;
    uint32_t    argc  = 0;
    HashTable  *named = NULL;

    ZEND_PARSE_PARAMETERS_START(0, -1)
        Z_PARAM_VARIADIC_WITH_NAMED(args, argc, named)
    ZEND_PARSE_PARAMETERS_END();

    array_init(return_value);

    /* 位置参数 */
    for (uint32_t i = 0; i < argc; i++) {
        zval copy;
        ZVAL_COPY(&copy, &args[i]);
        add_next_index_zval(return_value, &copy);
    }

    /* 具名参数 */
    if (named) {
        zend_string *key;
        zval        *val;
        ZEND_HASH_FOREACH_STR_KEY_VAL(named, key, val) {
            zval copy;
            ZVAL_COPY(&copy, val);
            if (key) {
                zend_hash_update(Z_ARRVAL_P(return_value), key, &copy);
            } else {
                add_next_index_zval(return_value, &copy);
            }
        } ZEND_HASH_FOREACH_END();
    }
}

PHP_MINFO_FUNCTION(chapter6)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "chapter6 support", "enabled");
    php_info_print_table_row(2, "Version", PHP_CHAPTER6_VERSION);
    php_info_print_table_end();
}

zend_module_entry chapter6_module_entry = {
    STANDARD_MODULE_HEADER,
    "chapter6",
    chapter6_functions,
    NULL,
    NULL,
    NULL,
    NULL,
    PHP_MINFO(chapter6),
    PHP_CHAPTER6_VERSION,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_CHAPTER6
ZEND_GET_MODULE(chapter6)
#endif
