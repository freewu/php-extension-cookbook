# Chapter 6 - 无固定参数（可变参数）函数

## 目标

实现 `...$args` 形式的可变参数函数，并支持 PHP 8 的具名参数。

## 提供的函数

| 函数 | 签名 | 说明 |
| --- | --- | --- |
| `chapter6_sum` | `(...$numbers): float` | 任意个数字求和 |
| `chapter6_join` | `(string $glue, ...$pieces): string` | 固定参数 + 可变参数 |
| `chapter6_pack` | `(...$values): array` | 打包成数组 |
| `chapter6_named` | `(...$args): array` | 同时收集位置参数与具名参数 |

## 关键知识点

### 1. arginfo

```c
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chapter6_sum, 0, 0, IS_DOUBLE, 0)
    ZEND_ARG_VARIADIC_INFO(0, numbers)
ZEND_END_ARG_INFO()
```

### 2. 解析可变参数

```c
zval    *args = NULL;
uint32_t argc = 0;

ZEND_PARSE_PARAMETERS_START(0, -1)   // 最大个数用 -1 表示不限
    Z_PARAM_VARIADIC('*', args, argc)
ZEND_PARSE_PARAMETERS_END();
```

- `args` 是**连续的 zval 数组**，不是 `HashTable`。
- `argc` 是可变参数个数，通过 `args[i]` 访问。
- 混用固定参数时，固定参数先解析，`Z_PARAM_VARIADIC` 放最后。

### 3. 具名参数（PHP 8）

```c
Z_PARAM_VARIADIC_WITH_NAMED(args, argc, named)
```

- 位置参数在 `args[0..argc-1]`。
- 额外的具名参数在 `named`（`HashTable *`，可能为 NULL）。
- 若不需要具名参数而调用方传了具名参数，用 `Z_PARAM_VARIADIC` 会报错，
  用 `Z_PARAM_VARIADIC_WITH_NAMED` 则能接住。

### 4. 类型转换

```c
double d = zval_get_double(&args[i]);     // 数字
zend_string *s = zval_get_string(&args[i]); // 字符串（需 zend_string_release）
```

## 编译与运行

```bash
cd chapter6
phpize && ./configure --enable-chapter6 && make
php -d extension=$(pwd)/modules/chapter6.so demo.php
make test
```
<!-- 本章代码 begin:chapter6 -->

## 完整代码（chapter6.c）

> 本章扩展的完整 C 源码，已带中文注释。由 [`scripts/sync-code-readme.sh`](../scripts/sync-code-readme.sh) 自动同步；以源码文件 [`chapter6.c`](chapter6.c) 为准。

<details>
<summary>展开 / 收起 chapter6.c（共 197 行）</summary>

```c
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
```

</details>

<!-- 本章代码 end:chapter6 -->
