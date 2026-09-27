/*
 * Chapter 4 - 数组参数与数组返回值
 *
 * 演示：
 *   - Z_PARAM_ARRAY 接收数组
 *   - ZEND_HASH_FOREACH_VAL / ZEND_HASH_FOREACH_KEY_VAL 遍历
 *   - array_init / add_next_index_* / zend_hash_update 构造数组
 *
 * 编译：phpize && ./configure --enable-chapter4 && make
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "Zend/zend_operators.h"
#include "php_chapter4.h"

PHP_FUNCTION(chapter4_sum);
PHP_FUNCTION(chapter4_squares);
PHP_FUNCTION(chapter4_keys_upper);
PHP_FUNCTION(chapter4_filter_even);

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chapter4_sum, 0, 1, IS_DOUBLE, 0)
    ZEND_ARG_TYPE_INFO(0, numbers, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chapter4_squares, 0, 1, IS_ARRAY, 0)
    ZEND_ARG_TYPE_INFO(0, n, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chapter4_keys_upper, 0, 1, IS_ARRAY, 0)
    ZEND_ARG_TYPE_INFO(0, input, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chapter4_filter_even, 0, 1, IS_ARRAY, 0)
    ZEND_ARG_TYPE_INFO(0, numbers, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry chapter4_functions[] = {
    PHP_FE(chapter4_sum,         arginfo_chapter4_sum)
    PHP_FE(chapter4_squares,     arginfo_chapter4_squares)
    PHP_FE(chapter4_keys_upper,  arginfo_chapter4_keys_upper)
    PHP_FE(chapter4_filter_even, arginfo_chapter4_filter_even)
    PHP_FE_END
};

/* -------------------------------------------------------------------------
 * float chapter4_sum(array $numbers)
 *
 * ZEND_HASH_FOREACH_VAL 只遍历值，忽略键。
 * ---------------------------------------------------------------------- */
PHP_FUNCTION(chapter4_sum)
{
    zend_array *numbers;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ARRAY_HT(numbers)
    ZEND_PARSE_PARAMETERS_END();

    double sum = 0.0;
    zval *val;

    ZEND_HASH_FOREACH_VAL(numbers, val) {
        sum += zval_get_double(val);
    } ZEND_HASH_FOREACH_END();

    RETURN_DOUBLE(sum);
}

/* -------------------------------------------------------------------------
 * array chapter4_squares(int $n)
 *
 * 构造一个列表（连续数字下标）返回值。
 * ---------------------------------------------------------------------- */
PHP_FUNCTION(chapter4_squares)
{
    zend_long n;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(n)
    ZEND_PARSE_PARAMETERS_END();

    if (n < 0) {
        zend_argument_value_error(1, "must be greater than or equal to 0");
        RETURN_THROWS();
    }

    /* array_init 把 return_value 初始化为空数组 */
    array_init(return_value);

    for (zend_long i = 0; i < n; i++) {
        add_next_index_long(return_value, i * i);
    }
}

/* -------------------------------------------------------------------------
 * array chapter4_keys_upper(array $input)
 *
 * 遍历键和值，把字符串键转成大写，构造新数组返回。
 * ---------------------------------------------------------------------- */
PHP_FUNCTION(chapter4_keys_upper)
{
    zend_array *input;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ARRAY_HT(input)
    ZEND_PARSE_PARAMETERS_END();

    array_init_size(return_value, zend_hash_num_elements(input));

    zend_ulong num_key;
    zend_string *str_key;
    zval *val;

    ZEND_HASH_FOREACH_KEY_VAL(input, num_key, str_key, val) {
        zval copy;
        ZVAL_COPY(&copy, val);          /* 增加引用计数，所有权交给新数组 */

        if (str_key) {
            zend_string *upper = zend_string_toupper(str_key);
            zend_hash_update(Z_ARRVAL_P(return_value), upper, &copy);
            zend_string_release(upper);
        } else {
            zend_hash_index_update(Z_ARRVAL_P(return_value), num_key, &copy);
        }
    } ZEND_HASH_FOREACH_END();
}

/* -------------------------------------------------------------------------
 * array chapter4_filter_even(array $numbers)
 *
 * 过滤出偶数，构造列表返回值。
 * ---------------------------------------------------------------------- */
PHP_FUNCTION(chapter4_filter_even)
{
    zend_array *numbers;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ARRAY_HT(numbers)
    ZEND_PARSE_PARAMETERS_END();

    array_init(return_value);

    zval *val;
    ZEND_HASH_FOREACH_VAL(numbers, val) {
        if (Z_TYPE_P(val) == IS_LONG && (Z_LVAL_P(val) % 2) == 0) {
            zval copy;
            ZVAL_COPY(&copy, val);
            add_next_index_zval(return_value, &copy);
        }
    } ZEND_HASH_FOREACH_END();
}

PHP_MINFO_FUNCTION(chapter4)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "chapter4 support", "enabled");
    php_info_print_table_row(2, "Version", PHP_CHAPTER4_VERSION);
    php_info_print_table_end();
}

zend_module_entry chapter4_module_entry = {
    STANDARD_MODULE_HEADER,
    "chapter4",
    chapter4_functions,
    NULL,
    NULL,
    NULL,
    NULL,
    PHP_MINFO(chapter4),
    PHP_CHAPTER4_VERSION,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_CHAPTER4
ZEND_GET_MODULE(chapter4)
#endif
