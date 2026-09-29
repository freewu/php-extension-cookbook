# Chapter 5 - 带默认值的函数

## 目标

实现带默认参数值的扩展函数，并让 `Reflection` 也能读到默认值。

## 提供的函数

| 函数 | 签名 |
| --- | --- |
| `chapter5_greet` | `(string $name = 'World', string $greeting = 'Hello'): string` |
| `chapter5_repeat` | `(string $text, int $times = 2, string $separator = ','): string` |
| `chapter5_pow` | `(int $base, int $exponent = 2): int` |
| `chapter5_slice` | `(string $text, int $length = 5, bool $ellipsis = false): string` |

## 关键知识点

### 1. 声明可选参数

```c
ZEND_PARSE_PARAMETERS_START(1, 3)   // 至少 1 个，最多 3 个
    Z_PARAM_STR(text)
    Z_PARAM_OPTIONAL                 // 从这里开始都是可选参数
    Z_PARAM_LONG(times)
    Z_PARAM_STR_OR_NULL(separator)
ZEND_PARSE_PARAMETERS_END();
```

参数个数区间要与 arginfo 的 `required_num_args` 一致。

### 2. 如何表达默认值

C 层没有“默认值”语法，惯例是：

- 把变量初始化成默认值，例如 `zend_long times = 2;`
- 或者初始化为 `NULL`，函数体里再回退，例如：

```c
zend_string *name = NULL;
...
if (!name) name = zend_string_init("World", 5, 0);
```

> 可选参数未传入时，`Z_PARAM_*` 不会修改目标变量，
> 所以“预初始化 = 默认值”是安全的写法。

### 3. 让 Reflection 知道默认值

类型和默认值都要写进 arginfo：

```c
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chapter5_greet, 0, 0, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, name, IS_STRING, 0, "\"World\"")
ZEND_END_ARG_INFO()
```

否则 `ReflectionParameter::getDefaultValue()` 会抛异常，
`isOptional()` 也会与真实行为不符。

### 4. 可选参数与类型

`Z_PARAM_STR_OR_NULL` 允许显式传 `null`，`Z_PARAM_STR` 不允许。
按需选择。

## 编译与运行

```bash
cd chapter5
phpize && ./configure --enable-chapter5 && make
php -d extension=$(pwd)/modules/chapter5.so demo.php
make test
```
<!-- 本章代码 begin:chapter5 -->

## 完整代码（chapter5.c）

> 本章扩展的完整 C 源码，已带中文注释。由 [`scripts/sync-code-readme.sh`](../scripts/sync-code-readme.sh) 自动同步；以源码文件 [`chapter5.c`](chapter5.c) 为准。

<details>
<summary>展开 / 收起 chapter5.c（共 204 行）</summary>

```c
/*
 * Chapter 5 - 带默认值的函数
 *
 * 演示：
 *   - Z_PARAM_OPTIONAL 声明可选参数
 *   - 用 C 变量初始化默认值
 *   - arginfo 中的 ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE
 *     （让 Reflection 能拿到默认值）
 *
 * 编译：phpize && ./configure --enable-chapter5 && make
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "Zend/zend_smart_str.h"
#include "php_chapter5.h"

PHP_FUNCTION(chapter5_greet);
PHP_FUNCTION(chapter5_repeat);
PHP_FUNCTION(chapter5_pow);
PHP_FUNCTION(chapter5_slice);

/* -------------------------------------------------------------------------
 * arginfo：默认值写在 WITH_DEFAULT_VALUE 的最后一个字符串参数里
 * ---------------------------------------------------------------------- */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chapter5_greet, 0, 0, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, name,     IS_STRING, 0, "\"World\"")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, greeting, IS_STRING, 0, "\"Hello\"")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chapter5_repeat, 0, 1, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, times,     IS_LONG,   0, "2")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, separator, IS_STRING, 0, "\",\"")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chapter5_pow, 0, 1, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, base,     IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, exponent, IS_LONG, 0, "2")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chapter5_slice, 0, 1, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, length,   IS_LONG,   0, "5")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, ellipsis, _IS_BOOL,  0, "false")
ZEND_END_ARG_INFO()

static const zend_function_entry chapter5_functions[] = {
    PHP_FE(chapter5_greet,  arginfo_chapter5_greet)
    PHP_FE(chapter5_repeat, arginfo_chapter5_repeat)
    PHP_FE(chapter5_pow,    arginfo_chapter5_pow)
    PHP_FE(chapter5_slice,  arginfo_chapter5_slice)
    PHP_FE_END
};

/* -------------------------------------------------------------------------
 * string chapter5_greet(string $name = 'World', string $greeting = 'Hello')
 *
 * 可选参数在未传入时保持初始值，因此先初始化为 NULL，
 * 再在函数体里回退到默认值。
 * ---------------------------------------------------------------------- */
PHP_FUNCTION(chapter5_greet)
{
    zend_string *name = NULL;
    zend_string *greeting = NULL;

    ZEND_PARSE_PARAMETERS_START(0, 2)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_OR_NULL(name)
        Z_PARAM_STR_OR_NULL(greeting)
    ZEND_PARSE_PARAMETERS_END();

    RETURN_STR(strpprintf(0, "%s, %s!",
        greeting ? ZSTR_VAL(greeting) : "Hello",
        name     ? ZSTR_VAL(name)     : "World"));
}

/* -------------------------------------------------------------------------
 * string chapter5_repeat(string $text, int $times = 2, string $separator = ',')
 *
 * 数值/布尔型默认值：直接在 C 变量上初始化。
 * ---------------------------------------------------------------------- */
PHP_FUNCTION(chapter5_repeat)
{
    zend_string *text;
    zend_string *separator = NULL;
    zend_long    times = 2;      /* 默认值 */

    ZEND_PARSE_PARAMETERS_START(1, 3)
        Z_PARAM_STR(text)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(times)
        Z_PARAM_STR_OR_NULL(separator)
    ZEND_PARSE_PARAMETERS_END();

    if (times < 0) {
        times = 0;
    }

    const char *sep     = separator ? ZSTR_VAL(separator) : ",";
    size_t      sep_len = separator ? ZSTR_LEN(separator) : 1;

    smart_str buf = {0};
    for (zend_long i = 0; i < times; i++) {
        if (i > 0) {
            smart_str_appendl(&buf, sep, sep_len);
        }
        smart_str_append(&buf, text);
    }
    smart_str_0(&buf);

    if (buf.s == NULL) {
        RETURN_EMPTY_STRING();
    }
    RETURN_STR(buf.s);
}

/* -------------------------------------------------------------------------
 * int chapter5_pow(int $base, int $exponent = 2)
 * ---------------------------------------------------------------------- */
PHP_FUNCTION(chapter5_pow)
{
    zend_long base;
    zend_long exponent = 2;      /* 默认值 */

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_LONG(base)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(exponent)
    ZEND_PARSE_PARAMETERS_END();

    if (exponent < 0) {
        zend_argument_value_error(2, "must be greater than or equal to 0");
        RETURN_THROWS();
    }

    zend_long result = 1;
    for (zend_long i = 0; i < exponent; i++) {
        result *= base;
    }
    RETURN_LONG(result);
}

/* -------------------------------------------------------------------------
 * string chapter5_slice(string $text, int $length = 5, bool $ellipsis = false)
 * ---------------------------------------------------------------------- */
PHP_FUNCTION(chapter5_slice)
{
    zend_string *text;
    zend_long    length   = 5;      /* 默认值 */
    zend_bool    ellipsis = 0;      /* 默认值 */

    ZEND_PARSE_PARAMETERS_START(1, 3)
        Z_PARAM_STR(text)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(length)
        Z_PARAM_BOOL(ellipsis)
    ZEND_PARSE_PARAMETERS_END();

    if (length < 0) {
        length = 0;
    }

    if ((size_t) ZSTR_LEN(text) <= (size_t) length) {
        RETURN_STR(zend_string_copy(text));
    }

    if (ellipsis) {
        RETURN_STR(strpprintf(0, "%.*s...", (int) length, ZSTR_VAL(text)));
    }

    RETURN_STR(zend_string_init(ZSTR_VAL(text), length, 0));
}

PHP_MINFO_FUNCTION(chapter5)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "chapter5 support", "enabled");
    php_info_print_table_row(2, "Version", PHP_CHAPTER5_VERSION);
    php_info_print_table_end();
}

zend_module_entry chapter5_module_entry = {
    STANDARD_MODULE_HEADER,
    "chapter5",
    chapter5_functions,
    NULL,
    NULL,
    NULL,
    NULL,
    PHP_MINFO(chapter5),
    PHP_CHAPTER5_VERSION,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_CHAPTER5
ZEND_GET_MODULE(chapter5)
#endif
```

</details>

<!-- 本章代码 end:chapter5 -->
