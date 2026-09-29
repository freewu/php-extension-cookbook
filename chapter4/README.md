# Chapter 4 - 数组参数与数组返回值

## 目标

接收数组参数、遍历数组、构造数组作为返回值。

## 提供的函数

| 函数 | 签名 | 说明 |
| --- | --- | --- |
| `chapter4_sum` | `(array $numbers): float` | 求和 |
| `chapter4_squares` | `(int $n): array` | 返回 `[0,1,4,9,...]` |
| `chapter4_keys_upper` | `(array $input): array` | 字符串键转大写 |
| `chapter4_filter_even` | `(array $numbers): array` | 过滤出偶数 |

## 关键知识点

### 1. 接收数组

```c
zend_array *arr;                 // HashTable == zend_array
Z_PARAM_ARRAY_HT(arr)            // 直接得到 zend_array *
Z_PARAM_ARRAY_HT_OR_NULL(arr)    // 允许 null

zval *zv;                        // 若需要 zval：
Z_PARAM_ARRAY(zv)                // 得到 zval *（内部是 IS_ARRAY）
```

> 注意：PHP 8 中 `Z_PARAM_ARRAY(dest)` 的 `dest` 是 `zval *`，
> 想要 `zend_array *` 请用 `Z_PARAM_ARRAY_HT(dest)`。

### 2. 遍历

```c
zval *val;
ZEND_HASH_FOREACH_VAL(arr, val) {          // 只关心值
    ...
} ZEND_HASH_FOREACH_END();

zend_ulong num_key;
zend_string *str_key;
zval *val;
ZEND_HASH_FOREACH_KEY_VAL(arr, num_key, str_key, val) {  // 键 + 值
    ...
} ZEND_HASH_FOREACH_END();
```

- 字符串键时 `str_key != NULL`，数字键时 `str_key == NULL` 且用 `num_key`。

### 3. 构造数组

```c
array_init(return_value);                     // 空数组
array_init_size(return_value, n);             // 预分配容量
add_next_index_long(return_value, 42);        // 追加：$arr[] = 42
add_assoc_string(return_value, "k", "v");     // $arr['k'] = 'v'
zend_hash_index_update(Z_ARRVAL_P(return_value), i, &zv);
zend_hash_update(Z_ARRVAL_P(return_value), key, &zv);
```

### 4. 引用计数（重点）

数组里的 zval 都“拥有”自己的引用。把源数组的元素放进返回数组前，
必须增加引用计数，否则会出现悬空指针 / 提前释放：

```c
zval copy;
ZVAL_COPY(&copy, val);      // 引用计数 +1
add_next_index_zval(return_value, &copy);
```

### 5. 参数错误

```c
zend_argument_value_error(1, "must be greater than or equal to 0");
RETURN_THROWS();
```

会生成标准的 `ValueError: chapter4_squares(): Argument #1 ($n) ...` 消息。

## 编译与运行

```bash
cd chapter4
phpize && ./configure --enable-chapter4 && make
php -d extension=$(pwd)/modules/chapter4.so demo.php
make test
```
<!-- 本章代码 begin:chapter4 -->

## 完整代码（chapter4.c）

> 本章扩展的完整 C 源码，已带中文注释。由 [`scripts/sync-code-readme.sh`](../scripts/sync-code-readme.sh) 自动同步；以源码文件 [`chapter4.c`](chapter4.c) 为准。

<details>
<summary>展开 / 收起 chapter4.c（共 182 行）</summary>

```c
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
```

</details>

<!-- 本章代码 end:chapter4 -->
