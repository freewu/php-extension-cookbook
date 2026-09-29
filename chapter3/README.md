# Chapter 3 - 函数参数与返回值

## 目标

在扩展函数中接收参数、返回标量值，并在 C 层处理类型转换和异常。

## 提供的函数

| 函数 | 签名 | 说明 |
| --- | --- | --- |
| `chapter3_add` | `(int $a, int $b): int` | 整数相加 |
| `chapter3_concat` | `(string $a, string $b): string` | 字符串拼接 |
| `chapter3_upper` | `(string $s): string` | 转大写 |
| `chapter3_divide` | `(float $a, float $b): float` | 除法，除零抛异常 |
| `chapter3_type_of` | `(mixed $value): string` | 返回参数类型名 |

## 关键知识点

### 1. 解析参数

```c
ZEND_PARSE_PARAMETERS_START(2, 2)   // 最少 2 个、最多 2 个参数
    Z_PARAM_LONG(a)
    Z_PARAM_STR(b)
ZEND_PARSE_PARAMETERS_END();
```

- `ZEND_PARSE_PARAMETERS_START(min, max)`，`max = -1` 表示不限。
- 解析失败时宏内部已经 `return`，不会再往下执行。
- 常用取值宏：

| 宏 | C 类型 | 说明 |
| --- | --- | --- |
| `Z_PARAM_LONG` | `zend_long` | 整数 |
| `Z_PARAM_DOUBLE` | `double` | 浮点 |
| `Z_PARAM_BOOL` | `zend_bool` | 布尔 |
| `Z_PARAM_STR` | `zend_string *` | 字符串（已分离/安全） |
| `Z_PARAM_ZVAL` | `zval *` | 任意类型 |
| `Z_PARAM_ARRAY` | `zend_array *` | 数组（见第 4 章） |

### 2. arginfo 与类型

PHP 8 里类型信息主要来自 arginfo：

```c
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chapter3_add, 0, 2, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, a, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, b, IS_LONG, 0)
ZEND_END_ARG_INFO()
```

- 弱类型模式下，`Z_PARAM_LONG` 会把 `"3"` 之类的字符串转成整数。
- 传入数组等无法转换的值时，会抛 `TypeError`。

### 3. 返回值

| 宏 | 说明 |
| --- | --- |
| `RETURN_LONG(n)` | 返回整数 |
| `RETURN_DOUBLE(d)` | 返回浮点 |
| `RETURN_STRING(s)` | 返回字符串（会拷贝） |
| `RETURN_STR(zend_string *)` | 返回字符串，接管所有权 |
| `RETURN_TRUE` / `RETURN_FALSE` | 返回布尔 |
| `RETURN_NULL()` | 返回 null |

### 4. 抛出异常

```c
zend_throw_exception_ex(zend_ce_division_by_zero_error, 0, "Division by zero");
RETURN_THROWS();
```

`RETURN_THROWS()` 告诉引擎当前已存在异常，不要再设置返回值。

## 编译与运行

```bash
cd chapter3
phpize && ./configure --enable-chapter3 && make
php -d extension=$(pwd)/modules/chapter3.so demo.php
make test
```
<!-- 本章代码 begin:chapter3 -->

## 完整代码（chapter3.c）

> 本章扩展的完整 C 源码，已带中文注释。由 [`scripts/sync-code-readme.sh`](../scripts/sync-code-readme.sh) 自动同步；以源码文件 [`chapter3.c`](chapter3.c) 为准。

<details>
<summary>展开 / 收起 chapter3.c（共 171 行）</summary>

```c
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
```

</details>

<!-- 本章代码 end:chapter3 -->
