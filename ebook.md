# PHP 扩展开发手册（电子书版）

> 由 `scripts/gen-docs.sh` 自动生成，可用 pandoc 转 epub/pdf：
> `bash scripts/build-ebook.sh`

## 目录

* [Chapter 1 - Hello World](#chapter-1)
* [Chapter 2 - 扩展配置（INI）](#chapter-2)
* [Chapter 3 - 函数参数与返回值](#chapter-3)
* [Chapter 4 - 数组参数与数组返回值](#chapter-4)
* [Chapter 5 - 带默认值的函数](#chapter-5)
* [Chapter 6 - 无固定参数（可变参数）函数](#chapter-6)
* [Chapter 7 - 实现类（Class）](#chapter-7)
* [Chapter 8 - 资源（Resource）](#chapter-8)
* [Chapter 9 - 敏感词检测（DFA）](#chapter-9)

<a id="chapter-1"></a>
# Chapter 1 - Hello World

## 目标

搭建 PHP 8 扩展开发环境，实现第一个扩展函数 `hello_world()`，返回 `"Hello World"`。

## 环境准备（Ubuntu / Debian）

```bash
sudo apt-get update
sudo apt-get install -y php8.3-cli php8.3-dev build-essential autoconf pkg-config
```

验证：

```bash
php -v
phpize --version
php-config --version
```

## 目录结构

```
chapter1/
├── config.m4          # autoconf 配置：声明扩展名、源文件、编译开关
├── php_chapter1.h     # 扩展头文件
├── chapter1.c         # 扩展实现
├── demo.php           # 演示脚本
└── tests/
    └── 001-hello_world.phpt
```

## 关键知识点

| 概念                      | 说明                                 |
| ------------------------ | ------------------------------------ |
| `PHP_FUNCTION(name)`     | 声明用户空间可调用的函数                 |
| `ZEND_BEGIN_ARG_INFO_EX` | PHP 8 必需，描述函数签名（反射/类型检查） |
| `zend_function_entry`    | 函数表，把函数名和 C 实现绑定            |
| `zend_module_entry`      | 模块入口，定义生命周期回调与版本          |
| `PHP_MINFO_FUNCTION`     | `phpinfo()` 输出的模块信息             |
| `ZEND_GET_MODULE`        | 动态库方式加载的入口                    |

## 编译与运行

```bash
cd chapter1
phpize
./configure --enable-chapter1
make

php -d extension=$(pwd)/modules/chapter1.so demo.php
# 输出：Hello World
```

查看扩展信息：

```bash
php -d extension=$(pwd)/modules/chapter1.so --ri chapter1
```

运行测试：

```bash
make test
```
<!-- 本章代码 begin:chapter1 -->

## 完整代码（chapter1.c）

> 本章扩展的完整 C 源码，已带中文注释。由 [`scripts/sync-code-readme.sh`](../scripts/sync-code-readme.sh) 自动同步；以源码文件 [`chapter1.c`](chapter1.c) 为准。

<details>
<summary>展开 / 收起 chapter1.c（共 97 行）</summary>

```c
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
```

</details>

<!-- 本章代码 end:chapter1 -->

<a id="chapter-2"></a>
# Chapter 2 - 扩展配置（INI）

## 目标

让扩展拥有可在 `php.ini` / 命令行 / `ini_set()` 中配置的选项，并在函数里读取这些配置。

## 目录结构

```
chapter2/
├── config.m4
├── php_chapter2.h     # 模块全局变量声明
├── chapter2.c         # INI 注册 + 函数实现
├── demo.php
└── tests/
```

## 配置项

| 名称 | 默认值 | 说明 |
| --- | --- | --- |
| `chapter2.enabled` | `1` | 是否启用 |
| `chapter2.limit` | `3` | 重复次数 |
| `chapter2.greeting` | `Hello` | 问候语 |
| `chapter2.name` | `World` | 问候对象 |

## 关键知识点

### 1. 模块全局变量（module globals）

```c
ZEND_BEGIN_MODULE_GLOBALS(chapter2)
    zend_bool  enabled;
    zend_long  limit;
    char      *greeting;
    char      *name;
ZEND_END_MODULE_GLOBALS(chapter2)

#define CHAPTER2_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(chapter2, v)
```

- 全局变量按进程/线程隔离（ZTS 下每个线程一份）。
- 在 `.c` 中用 `ZEND_DECLARE_MODULE_GLOBALS(chapter2)` 定义实例。
- 用 `PHP_GINIT_FUNCTION` 完成零初始化。

### 2. 注册 INI

```c
PHP_INI_BEGIN()
    STD_PHP_INI_BOOLEAN("chapter2.enabled", "1", PHP_INI_ALL,
                        OnUpdateBool, enabled, zend_chapter2_globals, chapter2_globals)
    ...
PHP_INI_END()
```

`OnUpdateBool/Long/String` 会把解析结果直接写入全局变量对应字段，
所以函数里直接读 `CHAPTER2_G(...)` 即可，无需再查表。

### 3. 可修改范围

`PHP_INI_ALL` 表示任何阶段都能改；还有 `PHP_INI_SYSTEM`（只能 php.ini）、
`PHP_INI_PERDIR` 等。只有 `PHP_INI_ALL` / `PHP_INI_USER` 才允许 `ini_set()`。

### 4. 运行时修改

```c
zend_alter_ini_entry_chars(key, value, value_len,
                           PHP_INI_USER, PHP_INI_STAGE_RUNTIME);
```

等价于用户空间的 `ini_set()`。

### 5. 在 phpinfo() 中显示

`MINFO` 里调用 `DISPLAY_INI_ENTRIES()` 会输出所有该扩展的 INI 项。

## 编译与运行

```bash
cd chapter2
phpize
./configure --enable-chapter2
make
php -d extension=$(pwd)/modules/chapter2.so demo.php
php -d extension=$(pwd)/modules/chapter2.so -d chapter2.limit=1 demo.php
make test
```
<!-- 本章代码 begin:chapter2 -->

## 完整代码（chapter2.c）

> 本章扩展的完整 C 源码，已带中文注释。由 [`scripts/sync-code-readme.sh`](../scripts/sync-code-readme.sh) 自动同步；以源码文件 [`chapter2.c`](chapter2.c) 为准。

<details>
<summary>展开 / 收起 chapter2.c（共 216 行）</summary>

```c
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
```

</details>

<!-- 本章代码 end:chapter2 -->

<a id="chapter-3"></a>
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

<a id="chapter-4"></a>
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

<a id="chapter-5"></a>
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

<a id="chapter-6"></a>
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

<a id="chapter-7"></a>
# Chapter 7 - 实现类（Class）

## 目标

用 C 实现一个内部类 `Chapter7Counter`，包含实例方法、静态方法、
类常量、属性和自定义对象存储。

## 类定义

```php
class Chapter7Counter
{
    public string $label = '';
    const DEFAULT_STEP = 1;

    public function __construct(int $start = 0);
    public function increment(int $step = 1): int;
    public function decrement(int $step = 1): int;
    public function getValue(): int;
    public function setValue(int $value): void;
    public function getLabel(): string;
    public function setLabel(string $label): void;
    public static function create(int $start = 0): Chapter7Counter;
}
```

## 关键知识点

### 1. 自定义对象结构体

```c
typedef struct _chapter7_counter_object {
    zend_long   value;   // 自定义字段
    zend_object std;     // 必须是最后一个成员
} chapter7_counter_object;
```

对象内存布局为：`[自定义结构体][属性表]`，所以分配时：

```c
ecalloc(1, sizeof(chapter7_counter_object) + zend_object_properties_size(ce));
```

用 `XtOffsetOf` 从 `zend_object *` 反推：

```c
(chapter7_counter_object *)((char *)obj - XtOffsetOf(chapter7_counter_object, std))
```

### 2. 对象处理器（handlers）

| 处理器 | 作用 |
| --- | --- |
| `create_object` | `new` 时分配对象、初始化自定义字段 |
| `free_obj` | 对象回收时释放自定义资源 |
| `clone_obj` | `clone` 时拷贝自定义字段 |

注册：

```c
memcpy(&handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
handlers.offset    = XtOffsetOf(chapter7_counter_object, std);
handlers.free_obj  = chapter7_counter_free;
handlers.clone_obj = chapter7_counter_clone;
```

> `free_obj` 中不要 `efree` 对象本身，引擎会在调用后释放对象内存。

### 3. 注册类

```c
INIT_CLASS_ENTRY(ce, "Chapter7Counter", chapter7_counter_methods);
chapter7_counter_ce = zend_register_internal_class(&ce);
chapter7_counter_ce->create_object = chapter7_counter_create;
```

### 4. 方法

```c
PHP_METHOD(Chapter7Counter, increment) { ... }
```

方法表用 `PHP_ME(类名, 方法名, arginfo, 标志)`，
静态方法加 `ZEND_ACC_STATIC`。

在方法内：

```c
zend_long v = Z_CHAPTER7_COUNTER_P(ZEND_THIS)->value;  // $this
object_init_ex(return_value, chapter7_counter_ce);     // 返回新对象
```

### 5. 类常量与属性

```c
zend_declare_class_constant_long(ce, "DEFAULT_STEP", sizeof("DEFAULT_STEP") - 1, 1);
zend_declare_property_string(ce, "label", sizeof("label") - 1, "", ZEND_ACC_PUBLIC);
```

### 6. 读写属性

```c
zval rv;
zval *label = zend_read_property(ce, Z_OBJ_P(ZEND_THIS), "label", sizeof("label") - 1, 1, &rv);
zend_update_property_string(ce, Z_OBJ_P(ZEND_THIS), "label", sizeof("label") - 1, "x");
```

## 编译与运行

```bash
cd chapter7
phpize && ./configure --enable-chapter7 && make
php -d extension=$(pwd)/modules/chapter7.so demo.php
make test
```
<!-- 本章代码 begin:chapter7 -->

## 完整代码（chapter7.c）

> 本章扩展的完整 C 源码，已带中文注释。由 [`scripts/sync-code-readme.sh`](../scripts/sync-code-readme.sh) 自动同步；以源码文件 [`chapter7.c`](chapter7.c) 为准。

<details>
<summary>展开 / 收起 chapter7.c（共 306 行）</summary>

```c
/*
 * Chapter 7 - 实现类（Class）
 *
 * 演示：
 *   - 注册内部类 Chapter7Counter
 *   - 自定义对象结构体 + create_object / free_obj / clone_obj
 *   - 实例方法、静态方法、类常量、属性
 *
 * 编译：phpize && ./configure --enable-chapter7 && make
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_operators.h"
#include "php_chapter7.h"

/* -------------------------------------------------------------------------
 * 1. 自定义对象结构体
 *
 * zend_object std 必须是最后一个成员，对象内存布局：
 *   [ chapter7_counter_object ][ 属性表 HashTable ]
 * ---------------------------------------------------------------------- */
typedef struct _chapter7_counter_object {
    zend_long    value;   /* 自定义字段：计数器当前值 */
    zend_object  std;
} chapter7_counter_object;

static zend_class_entry *chapter7_counter_ce;
static zend_object_handlers chapter7_counter_handlers;

/* 从 zend_object * 反推回自定义结构体指针 */
static zend_always_inline chapter7_counter_object *
chapter7_counter_from_obj(zend_object *obj)
{
    return (chapter7_counter_object *)((char *) obj
            - XtOffsetOf(chapter7_counter_object, std));
}

#define Z_CHAPTER7_COUNTER_P(zv) \
    chapter7_counter_from_obj(Z_OBJ_P(zv))

/* -------------------------------------------------------------------------
 * 2. 对象处理器
 * ---------------------------------------------------------------------- */

/* new Chapter7Counter() 时调用：分配并初始化对象 */
static zend_object *chapter7_counter_create(zend_class_entry *ce)
{
    chapter7_counter_object *intern = ecalloc(1,
        sizeof(chapter7_counter_object) + zend_object_properties_size(ce));

    zend_object_std_init(&intern->std, ce);
    object_properties_init(&intern->std, ce);

    intern->value = 0;
    intern->std.handlers = &chapter7_counter_handlers;

    return &intern->std;
}

/* 对象被回收时调用：释放自定义资源（这里只有值类型，无需额外释放） */
static void chapter7_counter_free(zend_object *obj)
{
    chapter7_counter_object *intern = chapter7_counter_from_obj(obj);

    /* 注意：对象内存由引擎释放，这里不要 efree(intern) */
    zend_object_std_dtor(&intern->std);
}

/* clone $obj 时调用：拷贝自定义字段 */
static zend_object *chapter7_counter_clone(zend_object *old_obj)
{
    chapter7_counter_object *old_intern = chapter7_counter_from_obj(old_obj);
    zend_object             *new_obj    = chapter7_counter_create(old_obj->ce);
    chapter7_counter_object *new_intern = chapter7_counter_from_obj(new_obj);

    new_intern->value = old_intern->value;
    zend_objects_clone_members(new_obj, old_obj);

    return new_obj;
}

/* -------------------------------------------------------------------------
 * 3. 方法声明
 * ---------------------------------------------------------------------- */
PHP_METHOD(Chapter7Counter, __construct);
PHP_METHOD(Chapter7Counter, increment);
PHP_METHOD(Chapter7Counter, decrement);
PHP_METHOD(Chapter7Counter, getValue);
PHP_METHOD(Chapter7Counter, setValue);
PHP_METHOD(Chapter7Counter, getLabel);
PHP_METHOD(Chapter7Counter, setLabel);
PHP_METHOD(Chapter7Counter, create);

/* -------------------------------------------------------------------------
 * 4. 方法 arginfo
 * ---------------------------------------------------------------------- */
ZEND_BEGIN_ARG_INFO_EX(arginfo_chapter7_construct, 0, 0, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, start, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chapter7_step, 0, 0, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, step, IS_LONG, 0, "1")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chapter7_get_value, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chapter7_set_value, 0, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, value, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chapter7_get_label, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chapter7_set_label, 0, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, label, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_chapter7_create, 0, 0, Chapter7Counter, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, start, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()

/* -------------------------------------------------------------------------
 * 5. 方法表
 * ---------------------------------------------------------------------- */
static const zend_function_entry chapter7_counter_methods[] = {
    PHP_ME(Chapter7Counter, __construct, arginfo_chapter7_construct,  ZEND_ACC_PUBLIC)
    PHP_ME(Chapter7Counter, increment,   arginfo_chapter7_step,       ZEND_ACC_PUBLIC)
    PHP_ME(Chapter7Counter, decrement,   arginfo_chapter7_step,       ZEND_ACC_PUBLIC)
    PHP_ME(Chapter7Counter, getValue,    arginfo_chapter7_get_value,  ZEND_ACC_PUBLIC)
    PHP_ME(Chapter7Counter, setValue,    arginfo_chapter7_set_value,  ZEND_ACC_PUBLIC)
    PHP_ME(Chapter7Counter, getLabel,    arginfo_chapter7_get_label,  ZEND_ACC_PUBLIC)
    PHP_ME(Chapter7Counter, setLabel,    arginfo_chapter7_set_label,  ZEND_ACC_PUBLIC)
    PHP_ME(Chapter7Counter, create,      arginfo_chapter7_create,     ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_FE_END
};

/* -------------------------------------------------------------------------
 * 6. 方法实现
 * ---------------------------------------------------------------------- */

/* __construct(int $start = 0) */
PHP_METHOD(Chapter7Counter, __construct)
{
    zend_long start = 0;

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(start)
    ZEND_PARSE_PARAMETERS_END();

    Z_CHAPTER7_COUNTER_P(ZEND_THIS)->value = start;
}

/* increment(int $step = 1): int */
PHP_METHOD(Chapter7Counter, increment)
{
    zend_long step = 1;

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(step)
    ZEND_PARSE_PARAMETERS_END();

    chapter7_counter_object *intern = Z_CHAPTER7_COUNTER_P(ZEND_THIS);
    intern->value += step;

    RETURN_LONG(intern->value);
}

/* decrement(int $step = 1): int */
PHP_METHOD(Chapter7Counter, decrement)
{
    zend_long step = 1;

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(step)
    ZEND_PARSE_PARAMETERS_END();

    chapter7_counter_object *intern = Z_CHAPTER7_COUNTER_P(ZEND_THIS);
    intern->value -= step;

    RETURN_LONG(intern->value);
}

/* getValue(): int */
PHP_METHOD(Chapter7Counter, getValue)
{
    ZEND_PARSE_PARAMETERS_NONE();
    RETURN_LONG(Z_CHAPTER7_COUNTER_P(ZEND_THIS)->value);
}

/* setValue(int $value): void */
PHP_METHOD(Chapter7Counter, setValue)
{
    zend_long value;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(value)
    ZEND_PARSE_PARAMETERS_END();

    Z_CHAPTER7_COUNTER_P(ZEND_THIS)->value = value;
}

/* getLabel(): string */
PHP_METHOD(Chapter7Counter, getLabel)
{
    ZEND_PARSE_PARAMETERS_NONE();

    zval rv;
    zval *label = zend_read_property(chapter7_counter_ce, Z_OBJ_P(ZEND_THIS),
                                     "label", sizeof("label") - 1, 1, &rv);

    RETURN_STR(zval_get_string(label));
}

/* setLabel(string $label): void */
PHP_METHOD(Chapter7Counter, setLabel)
{
    zend_string *label;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(label)
    ZEND_PARSE_PARAMETERS_END();

    zend_update_property_string(chapter7_counter_ce, Z_OBJ_P(ZEND_THIS),
                                "label", sizeof("label") - 1,
                                ZSTR_VAL(label));
}

/* static create(int $start = 0): Chapter7Counter */
PHP_METHOD(Chapter7Counter, create)
{
    zend_long start = 0;

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(start)
    ZEND_PARSE_PARAMETERS_END();

    object_init_ex(return_value, chapter7_counter_ce);
    Z_CHAPTER7_COUNTER_P(return_value)->value = start;
}

/* -------------------------------------------------------------------------
 * 7. 模块初始化：注册类
 * ---------------------------------------------------------------------- */
PHP_MINIT_FUNCTION(chapter7)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "Chapter7Counter", chapter7_counter_methods);
    chapter7_counter_ce = zend_register_internal_class(&ce);
    chapter7_counter_ce->create_object = chapter7_counter_create;

    memcpy(&chapter7_counter_handlers,
           zend_get_std_object_handlers(),
           sizeof(zend_object_handlers));
    chapter7_counter_handlers.offset    = XtOffsetOf(chapter7_counter_object, std);
    chapter7_counter_handlers.free_obj  = chapter7_counter_free;
    chapter7_counter_handlers.clone_obj = chapter7_counter_clone;

    /* 类常量 */
    zend_declare_class_constant_long(chapter7_counter_ce,
        "DEFAULT_STEP", sizeof("DEFAULT_STEP") - 1, 1);

    /* 属性（带默认值） */
    zend_declare_property_string(chapter7_counter_ce,
        "label", sizeof("label") - 1, "", ZEND_ACC_PUBLIC);

    return SUCCESS;
}

PHP_MINFO_FUNCTION(chapter7)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "chapter7 support", "enabled");
    php_info_print_table_row(2, "Version", PHP_CHAPTER7_VERSION);
    php_info_print_table_end();
}

zend_module_entry chapter7_module_entry = {
    STANDARD_MODULE_HEADER,
    "chapter7",
    NULL,                 /* 本章只提供类，没有全局函数 */
    PHP_MINIT(chapter7),
    NULL,
    NULL,
    NULL,
    PHP_MINFO(chapter7),
    PHP_CHAPTER7_VERSION,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_CHAPTER7
ZEND_GET_MODULE(chapter7)
#endif
```

</details>

<!-- 本章代码 end:chapter7 -->

<a id="chapter-8"></a>
# Chapter 8 - 资源（Resource）

## 目标

实现一个自定义资源类型 `chapter8_buffer`（内存缓冲区），
理解资源的注册、创建、取回和释放。

## 提供的函数

| 函数 | 签名 | 说明 |
| --- | --- | --- |
| `chapter8_open` | `(string $name = ''): resource` | 创建缓冲区资源 |
| `chapter8_write` | `(resource $handle, string $data): int` | 追加写入，返回新长度 |
| `chapter8_read` | `(resource $handle): string` | 读取全部内容 |
| `chapter8_length` | `(resource $handle): int` | 当前长度 |
| `chapter8_close` | `(resource $handle): void` | 显式关闭资源 |

`get_resource_type($h)` 返回 `chapter8_buffer`。

## 关键知识点

### 1. 注册资源类型

```c
static int le_chapter8_buffer;

PHP_MINIT_FUNCTION(chapter8)
{
    le_chapter8_buffer = zend_register_list_destructors_ex(
        chapter8_buffer_dtor,   // 普通资源析构函数
        NULL,                   // 持久资源析构函数（pmem 分配时用）
        "chapter8_buffer",      // 类型名
        module_number);
    return SUCCESS;
}
```

### 2. 析构函数

```c
static void chapter8_buffer_dtor(zend_resource *rsrc)
{
    chapter8_buffer *buf = (chapter8_buffer *) rsrc->ptr;
    if (buf == NULL) return;
    if (buf->data != NULL) efree(buf->data);
    efree(buf);
}
```

资源被 GC 回收或被显式关闭时都会调用它，因此只需释放一次。

### 3. 创建资源

```c
chapter8_buffer *buf = ecalloc(1, sizeof(chapter8_buffer));
...
RETURN_RES(zend_register_resource(buf, le_chapter8_buffer));
```

`zend_register_resource` 返回 `zend_resource *`，`RETURN_RES` 把它放进返回值。

### 4. 取回底层结构体

```c
chapter8_buffer *buf = zend_fetch_resource_ex(handle, NULL, le_chapter8_buffer);
if (buf == NULL) {
    zend_argument_type_error(1, "must be a valid chapter8_buffer resource");
    RETURN_THROWS();
}
```

- 参数用 `Z_PARAM_RESOURCE(handle)`（得到 `zval *`）解析。
- 类型不匹配时抛 `TypeError`，比内部 warning 更规范。

### 5. 关闭资源

```c
zend_list_close(Z_RES_P(handle));
```

- 触发析构函数并把资源标记为失效（type = -1）。
- 之后再把它当参数传入，`get_resource_type()` 返回 `Unknown`，
  取回结构体失败并抛 `TypeError`。

### 6. 生命周期

- 不手动 `close` 也没关系：变量离开作用域后引用计数归零，
  资源存储会调用析构函数，避免内存泄漏。

## 编译与运行

```bash
cd chapter8
phpize && ./configure --enable-chapter8 && make
php -d extension=$(pwd)/modules/chapter8.so demo.php
make test
```
<!-- 本章代码 begin:chapter8 -->

## 完整代码（chapter8.c）

> 本章扩展的完整 C 源码，已带中文注释。由 [`scripts/sync-code-readme.sh`](../scripts/sync-code-readme.sh) 自动同步；以源码文件 [`chapter8.c`](chapter8.c) 为准。

<details>
<summary>展开 / 收起 chapter8.c（共 264 行）</summary>

```c
/*
 * Chapter 8 - 资源（Resource）
 *
 * 演示：
 *   - zend_register_list_destructors_ex 注册资源类型
 *   - zend_register_resource 创建资源并返回给 PHP
 *   - zend_fetch_resource_ex 取回底层 C 结构体
 *   - 资源析构函数 / 显式 close
 *
 * 这里实现一个“内存缓冲区”资源：
 *   $h = chapter8_open('Hello');
 *   chapter8_write($h, ' World');
 *   echo chapter8_read($h);   // Hello World
 *
 * 编译：phpize && ./configure --enable-chapter8 && make
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "Zend/zend_list.h"
#include "php_chapter8.h"

/* -------------------------------------------------------------------------
 * 1. 资源底层结构体
 * ---------------------------------------------------------------------- */
typedef struct _chapter8_buffer {
    char   *data;     /* 堆上缓冲区（可能为 NULL） */
    size_t  length;   /* 当前长度 */
} chapter8_buffer;

/* 资源类型 id，在 MINIT 中注册 */
static int le_chapter8_buffer;

/* -------------------------------------------------------------------------
 * 2. 资源析构函数
 *
 * 当资源被 GC 回收，或显式 close 时调用。
 * zend_resource *rsrc 的 ptr 指向我们创建的结构体。
 * ---------------------------------------------------------------------- */
static void chapter8_buffer_dtor(zend_resource *rsrc)
{
    chapter8_buffer *buf = (chapter8_buffer *) rsrc->ptr;
    if (buf == NULL) {
        return;
    }
    if (buf->data != NULL) {
        efree(buf->data);
    }
    efree(buf);
}

/* -------------------------------------------------------------------------
 * 3. 取回资源对应的结构体
 *
 * zend_fetch_resource_ex 会校验资源类型；传 NULL 作为名字可以
 * 抑制它内部的 warning，改由我们抛出规范的 TypeError。
 * ---------------------------------------------------------------------- */
static chapter8_buffer *chapter8_fetch(zval *handle, uint32_t arg_num)
{
    chapter8_buffer *buf = (chapter8_buffer *) zend_fetch_resource_ex(
        handle, NULL, le_chapter8_buffer);

    if (buf == NULL) {
        zend_argument_type_error(arg_num,
            "must be a valid chapter8_buffer resource");
        return NULL;
    }
    return buf;
}

/* 函数声明 */
PHP_FUNCTION(chapter8_open);
PHP_FUNCTION(chapter8_write);
PHP_FUNCTION(chapter8_read);
PHP_FUNCTION(chapter8_length);
PHP_FUNCTION(chapter8_close);

/* -------------------------------------------------------------------------
 * 4. arginfo
 * ---------------------------------------------------------------------- */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chapter8_open, 0, 0, IS_RESOURCE, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, name, IS_STRING, 0, "\"\"")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chapter8_write, 0, 2, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, handle, IS_RESOURCE, 0)
    ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chapter8_read, 0, 1, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, handle, IS_RESOURCE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chapter8_length, 0, 1, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, handle, IS_RESOURCE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chapter8_close, 0, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, handle, IS_RESOURCE, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry chapter8_functions[] = {
    PHP_FE(chapter8_open,   arginfo_chapter8_open)
    PHP_FE(chapter8_write,  arginfo_chapter8_write)
    PHP_FE(chapter8_read,   arginfo_chapter8_read)
    PHP_FE(chapter8_length, arginfo_chapter8_length)
    PHP_FE(chapter8_close,  arginfo_chapter8_close)
    PHP_FE_END
};

/* -------------------------------------------------------------------------
 * 5. 函数实现
 * ---------------------------------------------------------------------- */

/* resource chapter8_open(string $name = '') */
PHP_FUNCTION(chapter8_open)
{
    zend_string *name = NULL;

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_OR_NULL(name)
    ZEND_PARSE_PARAMETERS_END();

    chapter8_buffer *buf = ecalloc(1, sizeof(chapter8_buffer));

    if (name != NULL) {
        buf->length = ZSTR_LEN(name);
        buf->data   = emalloc(buf->length + 1);
        memcpy(buf->data, ZSTR_VAL(name), buf->length);
        buf->data[buf->length] = '\0';
    } else {
        buf->length = 0;
        buf->data   = NULL;
    }

    /* 把 C 结构体注册为资源并返回给 PHP */
    RETURN_RES(zend_register_resource(buf, le_chapter8_buffer));
}

/* int chapter8_write(resource $handle, string $data) */
PHP_FUNCTION(chapter8_write)
{
    zval        *handle;
    zend_string *data;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(handle)
        Z_PARAM_STR(data)
    ZEND_PARSE_PARAMETERS_END();

    chapter8_buffer *buf = chapter8_fetch(handle, 1);
    if (buf == NULL) {
        RETURN_THROWS();
    }

    size_t new_len = buf->length + ZSTR_LEN(data);
    buf->data = erealloc(buf->data, new_len + 1);
    memcpy(buf->data + buf->length, ZSTR_VAL(data), ZSTR_LEN(data));
    buf->data[new_len] = '\0';
    buf->length = new_len;

    RETURN_LONG((zend_long) new_len);
}

/* string chapter8_read(resource $handle) */
PHP_FUNCTION(chapter8_read)
{
    zval *handle;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(handle)
    ZEND_PARSE_PARAMETERS_END();

    chapter8_buffer *buf = chapter8_fetch(handle, 1);
    if (buf == NULL) {
        RETURN_THROWS();
    }

    if (buf->data == NULL) {
        RETURN_EMPTY_STRING();
    }
    RETURN_STRINGL(buf->data, buf->length);
}

/* int chapter8_length(resource $handle) */
PHP_FUNCTION(chapter8_length)
{
    zval *handle;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(handle)
    ZEND_PARSE_PARAMETERS_END();

    chapter8_buffer *buf = chapter8_fetch(handle, 1);
    if (buf == NULL) {
        RETURN_THROWS();
    }

    RETURN_LONG((zend_long) buf->length);
}

/* void chapter8_close(resource $handle) */
PHP_FUNCTION(chapter8_close)
{
    zval *handle;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(handle)
    ZEND_PARSE_PARAMETERS_END();

    chapter8_buffer *buf = chapter8_fetch(handle, 1);
    if (buf == NULL) {
        RETURN_THROWS();
    }

    /* 关闭资源：触发析构函数并把资源标记为失效 */
    zend_list_close(Z_RES_P(handle));
}

/* -------------------------------------------------------------------------
 * 6. 生命周期
 * ---------------------------------------------------------------------- */
PHP_MINIT_FUNCTION(chapter8)
{
    le_chapter8_buffer = zend_register_list_destructors_ex(
        chapter8_buffer_dtor,   /* 普通资源析构 */
        NULL,                   /* 持久资源析构 */
        "chapter8_buffer",      /* 类型名，get_resource_type() 返回它 */
        module_number);

    return SUCCESS;
}

PHP_MINFO_FUNCTION(chapter8)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "chapter8 support", "enabled");
    php_info_print_table_row(2, "Version", PHP_CHAPTER8_VERSION);
    php_info_print_table_end();
}

zend_module_entry chapter8_module_entry = {
    STANDARD_MODULE_HEADER,
    "chapter8",
    chapter8_functions,
    PHP_MINIT(chapter8),
    NULL,
    NULL,
    NULL,
    PHP_MINFO(chapter8),
    PHP_CHAPTER8_VERSION,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_CHAPTER8
ZEND_GET_MODULE(chapter8)
#endif
```

</details>

<!-- 本章代码 end:chapter8 -->

<a id="chapter-9"></a>
# Chapter 9 - 敏感词检测（DFA）

## 目标

实现一个基于 **DFA（确定性有限自动机）** 的敏感词检测扩展：

- 把 `.txt` 词典（每行一个词）构建成 Trie，序列化为 `.bin` 二进制文件；
- 扩展加载 `.bin` 后，可以取出命中关键词，也可以替换处理（默认替换为 `*`）。

## 提供的函数

| 函数 | 签名 | 说明 |
| --- | --- | --- |
| `chapter9_build` | `(string $txt, string $bin): bool` | 把词典 txt 构建成 Trie 并写入 .bin |
| `chapter9_load` | `(string $bin): bool` | 加载 .bin（成功后替换当前字典） |
| `chapter9_detect` | `(string $text): array` | 返回命中的关键词（去重、按首次命中排序） |
| `chapter9_replace` | `(string $text, string $replacement = '*'): string` | 替换命中词 |
| `chapter9_has` | `(string $text): bool` | 是否命中 |
| `chapter9_count` | `(string $text): int` | 命中的词数（最左优先、不重叠） |

配置项：`chapter9.bin` —— 设置后首次调用自动加载，无需手动 `chapter9_load()`。

```php
ini_set('chapter9.bin', __DIR__ . '/words.bin');
chapter9_detect('今晚去赌场赌博');          // ['赌场', '赌博']
chapter9_replace('今晚去赌场赌博');          // '今晚去****'
chapter9_replace('今晚去赌场赌博', '好');    // '今晚去好好好好'
```

## 算法：Trie + Aho-Corasick

### 1. Trie 构建（build）

词典构建期是内存中的 Trie：每个节点是 `父节点`，边按 **Unicode 码点** 转移，
词结尾的节点记录词在字符串池中的偏移与码点长度。

```
root ─赌─> (赌) ─博─> (赌博)* ─场─> (赌场)*
```

### 2. 二进制格式（.bin，全部小端序）

```
[0]  magic     'C' '9' 'T' 'R'
[4]  version   u32 = 1
[8]  node_count u32
[12] edge_count u32
[16] pool_size  u32
[20] reserved   u32
[24] nodes     node_count × 20B
     { is_end:u32, word_index:u32, word_cp_len:u32, edge_off:u32, edge_count:u32 }
[..] edges     edge_count × 8B
     { ch:u32, target:u32 }   （每个节点的子边按 ch 升序）
[..] pool      字符串池（每个词 NUL 结尾）
```

- 子边按码点排序，查找子节点用**二分**；
- 序列化后真正的 DFA 由“节点表 + 边表”驱动，不依赖指针。

### 3. 加载 + 失败指针（Aho-Corasick）

加载 `.bin` 后，用 BFS 构建两个辅助数组：

```c
fail[v]  // 失配时回退到的状态
out[v]   // 状态 v 沿 fail 链能到达的最近词尾节点
```

扫描时失败指针让匹配不会回退文本指针，复杂度 **O(文本长度)**：

```c
for (i in text) {
    while (state && !trans(state, ch)) state = fail[state];   // 失配跳转
    state = trans(state, ch);                                  // 尽量前进
    if (out[state]) hits[] = { i - word_cp_len + 1, i + 1 };  // 命中
}
```

### 4. 命中选择

扫描得到的候选按「起点升序、同起点终点降序」排序后贪心选择：
**最左优先、互不重叠**。因此 `赌场赌博`（赌场 + 赌博）会选中两处，
`赌博` 与 `赌场` 重叠的 `赌` 不会叠加。

### 5. 替换

- 每个命中的**码点**替换为一个 `replacement`（默认 `*`），长度不变；
- `replacement=''` 相当于删除命中词；`replacement='好'` 用多字节字符替换。

## 内存与生命周期

- **词典（进程生命周期数据）**：`chapter9_load()` 用标准 C 的
  `fopen/fread/malloc` 读入并解析 `.bin`，整块分配、`free()` 释放。
  这是有意为之：词典跨请求常驻、仅在进程退出/重载时释放，
  让它在 Zend MM（`emalloc`）的临时小内存之外独立管理，
  不会影响每次请求的临时内存统计，也避免特定环境里
  `MSHUTDOWN` 阶段多块小内存归还与 Zend MM 交互的边缘问题。
- **请求内临时数据**（解码后的码点数组、命中列表、输出字符串等）
  仍用常规的 `emalloc/smart_str`，随请求结束自动回收。

热更新词典不影响常驻内存：`chapter9_load()` 会先释放旧字典再换新。

## 词典格式（words.txt）

每行一个词，UTF-8 编码，忽略空行、首尾空白与 BOM：

```text
赌博
赌场
色情
毒品
fuck
drug
```

## 编译与运行

```bash
cd chapter9
phpize && ./configure --enable-chapter9 && make
php -d extension=$(pwd)/modules/chapter9.so demo.php
make test
```

## 性能测试

```bash
php -d extension=$(pwd)/modules/chapter9.so bench.php
php -d extension=$(pwd)/modules/chapter9.so bench.php --words=200000 --text-mb=40
```

脚本会生成“大词典 + 大文本”，测量构建 / 加载 / 扫描的耗时与吞吐，
并和 PCRE 正则过滤做基线对比。常用参数：

| 参数 | 默认 | 说明 |
| --- | --- | --- |
| `--words=N` | 50000 | 词典词数 |
| `--text-mb=N` | 10 | 测试文本大小 (MiB) |
| `--runs=N` | 3 | 每操作重复次数，取最优 |
| `--re-words=N` | 1000 | PCRE 基线词数 |
| `--re-mb=N` | 1 | PCRE 基线切片 (MiB) |
| `--skip-re` | - | 跳过 PCRE 基线 |

参考结果（PHP 8.3.6，5 万词词典 / 10 MiB 文本，Virt 机器）：

```text
chapter9_build        170 ms   （bin 3.85 MiB）
chapter9_load          51 ms
chapter9_detect       344 ms   29 MiB/s（命中 39010 个不同词）
chapter9_replace      598 ms   17 MiB/s
chapter9_has          273 ms   37 MiB/s
preg_match_all  1 MiB 切片 + 1000 词基线：403 ms（约 15 倍）
```

> 说明：PCRE 基线只用了 1000 个词，词典越大正则的差距越大——
> DFA 的扫描耗时只与文本长度相关，与词典大小无关。

### 本章两处性能细节

1. **序列化合并写**：把 `.bin` 先在内存拼好、一次 `php_stream_write`，
   而不是按节点 / 按边逐个小块写入（每次都是系统调用）。
   5 万词构建从约 4 s 降到约 170 ms。
2. **detect 去重用 HashTable**：命中词非常多时，用“数组 + 线性查找”
   去重会退化成 O(n²)，换成 `zend_hash_str_add` 判重后
   detect 从约 970 ms 降到约 344 ms。

## 扩展练习

- 支持词库热更新：`chapter9_build` 或 `chapter9_load` 后无需重启即可生效（已实现：load 会替换旧字典）；
- 把示例里“最长命中”改成“所有命中”（输出链接 out[] 沿 fail 链多跳即可）；
- 增加 `chapter9_detect` 返回命中位置（把 `c9_hit` 的 start/end 返回给 PHP）。
<!-- 本章代码 begin:chapter9 -->

## 完整代码（chapter9.c）

> 本章扩展的完整 C 源码，已带中文注释。由 [`scripts/sync-code-readme.sh`](../scripts/sync-code-readme.sh) 自动同步；以源码文件 [`chapter9.c`](chapter9.c) 为准。

<details>
<summary>展开 / 收起 chapter9.c（共 1007 行）</summary>

```c
/*
 * Chapter 9 - 敏感词检测（DFA，Trie + Aho-Corasick 自动机）
 *
 * 功能：
 *   chapter9_build($txt, $bin)  把“每行一个词”的 txt 构建成 Trie，
 *                               序列化（小端序）写入 .bin 文件
 *   chapter9.load (INI)         配置项，指定自动加载的 .bin 路径
 *   chapter9_load($bin)         运行时加载 .bin 并构建 AC 失败指针
 *   chapter9_detect($text)      返回命中的关键词（去重、按首次命中排序）
 *   chapter9_replace($text, '*') 把命中词替换为 *（长度不变），返回新文本
 *   chapter9_has($text)         是否存在命中
 *   chapter9_count($text)       命中的词数（贪心、不重叠）
 *
 * 原理：
 *   1) 词典构建期为 Trie（可动态增删的构建结构 c9_builder）；
 *   2) 序列化为扁平二进制：节点表 + 边表（按码点排序，二分查找）+ 字符串池；
 *   3) 加载后在此基础上 BFS 构建失败指针 fail[] 与输出指针 out[]，
 *      形成真正的 DFA（Aho-Corasick），扫描匹配复杂度 O(文本长度)。
 *
 * 内存布局：
 *   加载后的字典（节点表/边表/字符串池/fail/out）从“一整块”emalloc
 *   内存里按偏移切分，避免多块小内存与 Zend MM 交互的边界问题；
 *   释放时只需要 efree 一次。
 *
 * 编译：phpize && ./configure --enable-chapter9 && make
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "main/php_streams.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_smart_str.h"
#include "php_chapter9.h"

ZEND_DECLARE_MODULE_GLOBALS(chapter9)

/* -------------------------------------------------------------------------
 * 1. 扁平数据结构
 *
 * 加载后的字典是“扁平”的：
 *   每个节点的子边区间 [edge_off, edge_off+edge_count)，按码点升序排列，
 *   因此查找子节点用二分；整个字典放在一块内存中（见 c9_trie.raw）。
 * ---------------------------------------------------------------------- */
typedef struct _c9_edge {
    uint32_t ch;      /* Unicode 码点 */
    uint32_t target;  /* 子节点下标 */
} c9_edge;

typedef struct _c9_node {
    uint32_t is_end;      /* 是否为一个词的结尾 */
    uint32_t word_index;  /* 是结尾时：词在 pool 中的偏移 */
    uint32_t word_cp_len; /* 是结尾时：词的码点个数（用于计算命中起点） */
    uint32_t edge_off;    /* 子边区间起点（edges[] 下标） */
    uint32_t edge_count;  /* 子边个数 */
} c9_node;

typedef struct _c9_trie {
    uint32_t node_count;
    uint32_t edge_count;
    uint32_t pool_size;

    size_t   off_nodes;   /* raw 内的各区域偏移 */
    size_t   off_edges;
    size_t   off_pool;
    size_t   off_fail;
    size_t   off_out;
    char    *raw;         /* 一整块内存 */
} c9_trie;

#define C9_TRIE_NODES(t) ((c9_node *)((t)->raw + (t)->off_nodes))
#define C9_TRIE_EDGES(t) ((c9_edge *)((t)->raw + (t)->off_edges))
#define C9_TRIE_POOL(t)  ((t)->raw + (t)->off_pool)
#define C9_TRIE_FAIL(t)  ((uint32_t *)((t)->raw + (t)->off_fail))
#define C9_TRIE_OUT(t)   ((uint32_t *)((t)->raw + (t)->off_out))

/* 构建期结构（动态数组，可扩展；用完即弃） */
typedef struct _c9_bnode {
    zend_bool  is_end;
    uint32_t   word_index;
    uint32_t   word_cp_len;
    c9_edge   *edges;
    uint32_t   edge_count;
    uint32_t   edge_cap;
    uint32_t   edge_off;
} c9_bnode;

typedef struct _c9_builder {
    c9_bnode  *nodes;
    uint32_t   node_count;
    uint32_t   node_cap;
    char      *pool;
    uint32_t   pool_len;
    uint32_t   pool_cap;
} c9_builder;

/* -------------------------------------------------------------------------
 * 2. 二进制格式（全部小端序）
 *
 *   [0]  magic: 'C' '9' 'T' 'R'(4B)
 *   [4]  version: u32 = 1
 *   [8]  node_count: u32
 *   [12] edge_count: u32
 *   [16] pool_size:  u32
 *   [20] reserved:   u32
 *   [24] nodes:  node_count * 20B
 *        { is_end:u32, word_index:u32, word_cp_len:u32, edge_off:u32, edge_count:u32 }
 *   [..] edges:  edge_count * 8B
 *        { ch:u32, target:u32 }
 *   [..] pool:   字符串池（每个词 NUL 结尾）
 * ---------------------------------------------------------------------- */
#define C9_MAGIC0 'C'
#define C9_MAGIC1 '9'
#define C9_MAGIC2 'T'
#define C9_MAGIC3 'R'
#define C9_VERSION 1

static void c9_w32(unsigned char *p, uint32_t v)
{
    p[0] = (unsigned char)(v & 0xff);
    p[1] = (unsigned char)((v >> 8)  & 0xff);
    p[2] = (unsigned char)((v >> 16) & 0xff);
    p[3] = (unsigned char)((v >> 24) & 0xff);
}

static uint32_t c9_r32(const unsigned char *p)
{
    return ((uint32_t) p[0])
         | ((uint32_t) p[1] << 8)
         | ((uint32_t) p[2] << 16)
         | ((uint32_t) p[3] << 24);
}

/* -------------------------------------------------------------------------
 * 3. UTF-8 解码
 * ---------------------------------------------------------------------- */
static void c9_utf8_decode(const char *str, size_t len,
                           uint32_t **out_cps, uint32_t **out_offs, size_t *out_n)
{
    size_t cap = len ? len : 1;
    uint32_t *cps  = emalloc(sizeof(uint32_t) * cap);
    uint32_t *offs = out_offs ? emalloc(sizeof(uint32_t) * (cap + 1)) : NULL;
    size_t    n    = 0;

    const unsigned char *p   = (const unsigned char *) str;
    const unsigned char *end = p + len;

    if (offs) offs[0] = 0;

    while (p < end) {
        unsigned char b = *p;
        uint32_t cp;
        size_t adv = 1;

        if (b < 0x80) {
            cp = b;
        } else if ((b & 0xE0) == 0xC0 && end - p >= 2 && (p[1] & 0xC0) == 0x80) {
            cp  = ((uint32_t)(b & 0x1F) << 6) | (p[1] & 0x3F);
            adv = 2;
        } else if ((b & 0xF0) == 0xE0 && end - p >= 3
                   && (p[1] & 0xC0) == 0x80 && (p[2] & 0xC0) == 0x80) {
            uint32_t c = ((uint32_t)(b & 0x0F) << 12)
                       | ((uint32_t)(p[1] & 0x3F) << 6)
                       | (p[2] & 0x3F);
            if (c >= 0xD800 && c <= 0xDFFF) { cp = b; adv = 1; }
            else { cp = c; adv = 3; }
        } else if ((b & 0xF8) == 0xF0 && end - p >= 4
                   && (p[1] & 0xC0) == 0x80 && (p[2] & 0xC0) == 0x80
                   && (p[3] & 0xC0) == 0x80) {
            uint32_t c = ((uint32_t)(b & 0x07) << 18)
                       | ((uint32_t)(p[1] & 0x3F) << 12)
                       | ((uint32_t)(p[2] & 0x3F) << 6)
                       | (p[3] & 0x3F);
            if (c < 0x10000 || c > 0x10FFFF) { cp = b; adv = 1; }
            else { cp = c; adv = 4; }
        } else {
            cp = b;
        }

        if (n == cap) {
            cap *= 2;
            cps  = erealloc(cps,  sizeof(uint32_t) * cap);
            if (offs) offs = erealloc(offs, sizeof(uint32_t) * (cap + 1));
        }
        cps[n] = cp;
        if (offs) offs[n + 1] = offs[n] + adv;
        n++;
        p += adv;
    }

    *out_cps = cps;
    *out_n   = n;
    if (out_offs) *out_offs = offs;
}

/* -------------------------------------------------------------------------
 * 4. Trie 构建（c9_builder）
 * ---------------------------------------------------------------------- */
static void c9_builder_init(c9_builder *b)
{
    memset(b, 0, sizeof(*b));
    b->nodes = emalloc(sizeof(c9_bnode) * 16);
    b->node_cap = 16;
    b->node_count = 1;   /* 根节点 = 0 */
    memset(&b->nodes[0], 0, sizeof(c9_bnode));
}

static void c9_builder_free(c9_builder *b)
{
    for (uint32_t i = 0; i < b->node_count; i++) {
        if (b->nodes[i].edges) efree(b->nodes[i].edges);
    }
    efree(b->nodes);
    if (b->pool) efree(b->pool);
    memset(b, 0, sizeof(*b));
}

static uint32_t c9_builder_add_node(c9_builder *b)
{
    if (b->node_count == b->node_cap) {
        uint32_t ncap = b->node_cap * 2;
        b->nodes = erealloc(b->nodes, sizeof(c9_bnode) * ncap);
        b->node_cap = ncap;
    }
    memset(&b->nodes[b->node_count], 0, sizeof(c9_bnode));
    return b->node_count++;
}

static c9_edge *c9_builder_find_edge(c9_builder *b, uint32_t node, uint32_t ch)
{
    c9_bnode *n = &b->nodes[node];
    for (uint32_t i = 0; i < n->edge_count; i++) {
        if (n->edges[i].ch == ch) return &n->edges[i];
    }
    return NULL;
}

static uint32_t c9_builder_add_edge(c9_builder *b, uint32_t from, uint32_t ch)
{
    c9_bnode *n = &b->nodes[from];
    if (n->edge_count == n->edge_cap) {
        uint32_t ncap = n->edge_cap ? n->edge_cap * 2 : 4;
        n->edges = erealloc(n->edges, sizeof(c9_edge) * ncap);
        n->edge_cap = ncap;
    }
    n->edges[n->edge_count].ch = ch;
    uint32_t child = c9_builder_add_node(b);
    n = &b->nodes[from];
    n->edges[n->edge_count].target = child;
    return n->edges[n->edge_count++].target;
}

static uint32_t c9_pool_append(c9_builder *b, const char *s, size_t len)
{
    if (b->pool_len + len + 1 > b->pool_cap) {
        size_t ncap = b->pool_cap ? b->pool_cap : 64;
        while (ncap < b->pool_len + len + 1) ncap *= 2;
        b->pool = erealloc(b->pool, ncap);
        b->pool_cap = ncap;
    }
    uint32_t off = b->pool_len;
    memcpy(b->pool + off, s, len);
    b->pool[off + len] = '\0';
    b->pool_len += len + 1;
    return off;
}

static void c9_builder_insert(c9_builder *b, const char *word, size_t len)
{
    uint32_t *cps;
    size_t    n;

    c9_utf8_decode(word, len, &cps, NULL, &n);
    if (n == 0) {
        return;
    }

    uint32_t node = 0;
    for (size_t i = 0; i < n; i++) {
        c9_edge *e = c9_builder_find_edge(b, node, cps[i]);
        if (e) {
            node = e->target;
        } else {
            node = c9_builder_add_edge(b, node, cps[i]);
        }
    }
    if (!b->nodes[node].is_end) {
        b->nodes[node].is_end      = 1;
        b->nodes[node].word_cp_len = n;
        b->nodes[node].word_index  = c9_pool_append(b, word, len);
    }
    efree(cps);
}

static int c9_edge_cmp(const void *pa, const void *pb)
{
    const c9_edge *a = pa, *b = pb;
    return (a->ch < b->ch) ? -1 : ((a->ch > b->ch) ? 1 : 0);
}

/* -------------------------------------------------------------------------
 * 5. 序列化（Trie -> .bin）
 * ---------------------------------------------------------------------- */
static zend_bool c9_serialize(c9_builder *b, php_stream *out)
{
    uint32_t nc = b->node_count;
    uint32_t ec = 0;
    for (uint32_t i = 0; i < nc; i++) ec += b->nodes[i].edge_count;

    /* 展平边表：每个节点的子边按 ch 排序后连续存放 */
    c9_edge *flat = emalloc(sizeof(c9_edge) * (ec ? ec : 1));
    uint32_t off = 0;
    for (uint32_t i = 0; i < nc; i++) {
        c9_bnode *n = &b->nodes[i];
        if (n->edge_count > 1) {
            qsort(n->edges, n->edge_count, sizeof(c9_edge), c9_edge_cmp);
        }
        n->edge_off = off;
        if (n->edge_count) {
            memcpy(flat + off, n->edges, sizeof(c9_edge) * n->edge_count);
        }
        off += n->edge_count;
    }

    /* 在内存里拼好整个 .bin，再一次性写入，
     * 避免按节点 / 按边逐个小块 php_stream_write（每次都是系统调用）。 */
    size_t total = 24 + (size_t)nc * 20 + (size_t)ec * 8 + b->pool_len;
    unsigned char *buf = emalloc(total ? total : 1);
    unsigned char *p   = buf;

    p[0] = C9_MAGIC0; p[1] = C9_MAGIC1; p[2] = C9_MAGIC2; p[3] = C9_MAGIC3;
    c9_w32(p + 4,  C9_VERSION);
    c9_w32(p + 8,  nc);
    c9_w32(p + 12, ec);
    c9_w32(p + 16, b->pool_len);
    c9_w32(p + 20, 0);
    p += 24;

    for (uint32_t i = 0; i < nc; i++) {
        c9_bnode *n = &b->nodes[i];
        c9_w32(p,      n->is_end);
        c9_w32(p + 4,  n->word_index);
        c9_w32(p + 8,  n->word_cp_len);
        c9_w32(p + 12, n->edge_off);
        c9_w32(p + 16, n->edge_count);
        p += 20;
    }

    for (uint32_t i = 0; i < ec; i++) {
        c9_w32(p,     flat[i].ch);
        c9_w32(p + 4, flat[i].target);
        p += 8;
    }

    if (b->pool_len) {
        memcpy(p, b->pool, b->pool_len);
        p += b->pool_len;
    }

    zend_bool ok = (php_stream_write(out, buf, total) == (ssize_t) total)
                 ? SUCCESS : FAILURE;

    efree(flat);
    efree(buf);
    return ok;
}

/* -------------------------------------------------------------------------
 * 6. 加载 + 构建 AC 自动机
 * ---------------------------------------------------------------------- */

/* 二分查找子边；找不到返回 UINT32_MAX */
static uint32_t c9_get_trans(const c9_trie *t, uint32_t node, uint32_t ch)
{
    uint32_t lo = C9_TRIE_NODES(t)[node].edge_off;
    uint32_t hi = lo + C9_TRIE_NODES(t)[node].edge_count;
    while (lo < hi) {
        uint32_t mid = lo + (hi - lo) / 2;
        uint32_t c = C9_TRIE_EDGES(t)[mid].ch;
        if (c == ch) return C9_TRIE_EDGES(t)[mid].target;
        if (c < ch) lo = mid + 1;
        else hi = mid;
    }
    return UINT32_MAX;
}

static void c9_trie_free(c9_trie *t)
{
    if (!t) return;
    if (t->raw) free(t->raw);
    free(t);
}

/* BFS 构建失败指针 fail[] 与输出指针 out[] */
static void c9_build_ac(c9_trie *t)
{
    c9_node  *nodes = C9_TRIE_NODES(t);
    c9_edge  *edges = C9_TRIE_EDGES(t);
    uint32_t *fail  = C9_TRIE_FAIL(t);
    uint32_t *out   = C9_TRIE_OUT(t);
    uint32_t  nc    = t->node_count;

    /* 词典数据进程生命周期：队列也走 malloc，与词典一致 */
    uint32_t *queue = malloc(sizeof(uint32_t) * nc);
    uint32_t head = 0, tail = 0;

    fail[0] = 0;
    out[0]  = 0;

    /* 根的直接子节点 fail = 0 */
    for (uint32_t e = nodes[0].edge_off;
         e < nodes[0].edge_off + nodes[0].edge_count; e++) {
        uint32_t v = edges[e].target;
        fail[v] = 0;
        out[v]  = nodes[v].is_end ? v : 0;
        queue[tail++] = v;
    }

    while (head < tail) {
        uint32_t u = queue[head++];
        for (uint32_t e = nodes[u].edge_off;
             e < nodes[u].edge_off + nodes[u].edge_count; e++) {
            uint32_t ch = edges[e].ch;
            uint32_t v  = edges[e].target;

            /* f = fail[u] 沿链上溯，找第一个能转移 ch 的状态 */
            uint32_t f = fail[u];
            while (f != 0 && c9_get_trans(t, f, ch) == UINT32_MAX) {
                f = fail[f];
            }
            fail[v] = (f != 0) ? c9_get_trans(t, f, ch) : 0;

            /* out[v]：自己或 fail 链上最近的词尾节点 */
            out[v] = nodes[v].is_end ? v : out[fail[v]];

            queue[tail++] = v;
        }
    }
    free(queue);
}

/* 校验并加载 .bin，成功后把 trie 写入 *out */
static zend_bool c9_load_trie(c9_trie **out, const char *path)
{
    /* 词典是进程生命周期数据：加载用标准 C 的 stdio + malloc，
     * 不经过 Zend MM，释放时用 free()，避免与大块小内存分配
     * 以及 Zend MM 在进程收尾阶段的交互出现意外。 */
    FILE *fp = fopen(path, "rb");
    if (!fp) return FAILURE;

    fseek(fp, 0, SEEK_END);
    long fsz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (fsz < 24) {
        fclose(fp);
        return FAILURE;
    }

    unsigned char *buf = malloc((size_t) fsz + 1);
    if (!buf) {
        fclose(fp);
        return FAILURE;
    }
    size_t got = fread(buf, 1, (size_t) fsz, fp);
    fclose(fp);
    if (got != (size_t) fsz) {
        free(buf);
        return FAILURE;
    }

    zend_bool ok = FAILURE;
    const unsigned char *p = buf;
    if (p[0] == C9_MAGIC0 && p[1] == C9_MAGIC1
     && p[2] == C9_MAGIC2 && p[3] == C9_MAGIC3
     && c9_r32(p + 4) == C9_VERSION) {
        uint32_t nc = c9_r32(p + 8);
        uint32_t ec = c9_r32(p + 12);
        uint32_t ps = c9_r32(p + 16);
        uint64_t need = 24ULL + 20ULL * nc + 8ULL * ec + ps;
        if (nc != 0 && need == (uint64_t) fsz) {
            c9_trie *t = calloc(1, sizeof(c9_trie));
            t->node_count = nc;
            t->edge_count = ec;
            t->pool_size  = ps;

            /* 一整块内存：nodes | edges | pool | fail | out */
            size_t nb = (size_t)nc * sizeof(c9_node);
            size_t eb = (size_t)ec * sizeof(c9_edge);
            t->off_nodes = 0;
            t->off_edges = nb;
            t->off_pool  = nb + eb;
            t->off_fail  = nb + eb + ps;
            t->off_out   = nb + eb + ps + (size_t)nc * sizeof(uint32_t);
            t->raw = malloc(nb + eb + ps + 2 * (size_t)nc * sizeof(uint32_t) + 32);
            if (t->raw) {
                const unsigned char *np = p + 24;
                c9_node *nn = C9_TRIE_NODES(t);
                for (uint32_t i = 0; i < nc; i++) {
                    nn[i].is_end      = c9_r32(np);
                    nn[i].word_index  = c9_r32(np + 4);
                    nn[i].word_cp_len = c9_r32(np + 8);
                    nn[i].edge_off    = c9_r32(np + 12);
                    nn[i].edge_count  = c9_r32(np + 16);
                    np += 20;
                }
                const unsigned char *ep = np;
                c9_edge *ee = C9_TRIE_EDGES(t);
                for (uint32_t i = 0; i < ec; i++) {
                    ee[i].ch     = c9_r32(ep);
                    ee[i].target = c9_r32(ep + 4);
                    ep += 8;
                }
                memcpy(C9_TRIE_POOL(t), ep, ps);

                c9_build_ac(t);
                *out = t;
                ok = SUCCESS;
            } else {
                free(t);
            }
        }
    }
    free(buf);
    return ok;
}

/* -------------------------------------------------------------------------
 * 7. 匹配
 * ---------------------------------------------------------------------- */
typedef struct _c9_hit {
    uint32_t start;
    uint32_t end;
    uint32_t word_off;
} c9_hit;

/* Aho-Corasick 扫描：找出所有“以某位置结尾”的最长命中 */
static c9_hit *c9_scan(const c9_trie *t, const uint32_t *cps, size_t n, size_t *out_cnt)
{
    size_t cap = 16, cnt = 0;
    c9_hit *hits = emalloc(sizeof(c9_hit) * cap);

    uint32_t state = 0;
    for (size_t i = 0; i < n; i++) {
        uint32_t ch = cps[i];

        while (state != 0 && c9_get_trans(t, state, ch) == UINT32_MAX) {
            state = C9_TRIE_FAIL(t)[state];
        }
        uint32_t next = c9_get_trans(t, state, ch);
        if (next != UINT32_MAX) {
            state = next;
        } else {
            state = 0;
        }

        if (state != 0 && C9_TRIE_OUT(t)[state] != 0) {
            uint32_t node = C9_TRIE_OUT(t)[state];
            uint32_t wlen = C9_TRIE_NODES(t)[node].word_cp_len;
            if (wlen > 0 && wlen <= i + 1) {
                if (cnt == cap) {
                    cap *= 2;
                    hits = erealloc(hits, sizeof(c9_hit) * cap);
                }
                hits[cnt].start    = (uint32_t) (i + 1 - wlen);
                hits[cnt].end      = (uint32_t) (i + 1);
                hits[cnt].word_off = C9_TRIE_NODES(t)[node].word_index;
                cnt++;
            }
        }
    }

    *out_cnt = cnt;
    return hits;
}

/* 排序：起点升序，同起点取最长（终点降序） */
static int c9_hit_cmp(const void *pa, const void *pb)
{
    const c9_hit *a = pa, *b = pb;
    if (a->start != b->start) return (a->start < b->start) ? -1 : 1;
    if (a->end   != b->end)   return (a->end   > b->end)   ? -1 : 1;
    return 0;
}

/* 贪心选择：最左优先、不重叠、同起点取最长；就地压缩 */
static size_t c9_select(c9_hit *hits, size_t cnt)
{
    qsort(hits, cnt, sizeof(c9_hit), c9_hit_cmp);
    size_t sel = 0;
    uint32_t last_end = 0;
    for (size_t i = 0; i < cnt; i++) {
        if (hits[i].start >= last_end) {
            hits[sel++] = hits[i];
            last_end = hits[i].end;
        }
    }
    return sel;
}

/* -------------------------------------------------------------------------
 * 8. 函数声明 / arginfo / 函数表
 * ---------------------------------------------------------------------- */
PHP_FUNCTION(chapter9_build);
PHP_FUNCTION(chapter9_load);
PHP_FUNCTION(chapter9_detect);
PHP_FUNCTION(chapter9_replace);
PHP_FUNCTION(chapter9_has);
PHP_FUNCTION(chapter9_count);

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chapter9_build, 0, 2, _IS_BOOL, 0)
    ZEND_ARG_TYPE_INFO(0, txt, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, bin, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chapter9_load, 0, 1, _IS_BOOL, 0)
    ZEND_ARG_TYPE_INFO(0, bin, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chapter9_detect, 0, 1, IS_ARRAY, 0)
    ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chapter9_replace, 0, 1, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, replacement, IS_STRING, 0, "\"*\"")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chapter9_has, 0, 1, _IS_BOOL, 0)
    ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chapter9_count, 0, 1, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry chapter9_functions[] = {
    PHP_FE(chapter9_build,   arginfo_chapter9_build)
    PHP_FE(chapter9_load,    arginfo_chapter9_load)
    PHP_FE(chapter9_detect,  arginfo_chapter9_detect)
    PHP_FE(chapter9_replace, arginfo_chapter9_replace)
    PHP_FE(chapter9_has,     arginfo_chapter9_has)
    PHP_FE(chapter9_count,   arginfo_chapter9_count)
    PHP_FE_END
};

/* -------------------------------------------------------------------------
 * 9. 配置项与全局变量
 * ---------------------------------------------------------------------- */
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("chapter9.bin", "", PHP_INI_ALL,
                      OnUpdateString, bin_path, zend_chapter9_globals, chapter9_globals)
PHP_INI_END()

static PHP_GINIT_FUNCTION(chapter9)
{
#if defined(COMPILE_DL_CHAPTER9) && defined(ZTS)
    ZEND_TSRMLS_CACHE_UPDATE();
#endif
    memset(chapter9_globals, 0, sizeof(*chapter9_globals));
}

/* 惰性加载：首次使用时按 chapter9.bin 自动加载 */
static c9_trie *c9_ensure_load(void)
{
    if (CHAPTER9_G(trie) != NULL) {
        return (c9_trie *) CHAPTER9_G(trie);
    }
    if (CHAPTER9_G(bin_path) != NULL && *CHAPTER9_G(bin_path)) {
        c9_trie *t = NULL;
        if (c9_load_trie(&t, CHAPTER9_G(bin_path)) == SUCCESS) {
            CHAPTER9_G(trie) = t;
            return t;
        }
    }
    return NULL;
}

static void c9_throw_not_loaded(void)
{
    zend_throw_exception(zend_exception_get_default(),
        "chapter9: dictionary not loaded; set ini chapter9.bin, "
        "call chapter9_load(), or build one with chapter9_build()", 0);
}

PHP_MINIT_FUNCTION(chapter9)
{
    REGISTER_INI_ENTRIES();
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(chapter9)
{
    if (CHAPTER9_G(trie) != NULL) {
        c9_trie_free((c9_trie *) CHAPTER9_G(trie));
        CHAPTER9_G(trie) = NULL;
    }
    UNREGISTER_INI_ENTRIES();
    return SUCCESS;
}

/* -------------------------------------------------------------------------
 * 10. 函数实现
 * ---------------------------------------------------------------------- */

/* bool chapter9_build(string $txt, string $bin) */
PHP_FUNCTION(chapter9_build)
{
    zend_string *txt, *bin;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STR(txt)
        Z_PARAM_STR(bin)
    ZEND_PARSE_PARAMETERS_END();

    php_stream *s = php_stream_open_wrapper(ZSTR_VAL(txt), "rb", 0, NULL);
    if (!s) {
        char *msg;
        spprintf(&msg, 0, "chapter9: cannot open dictionary file: %s", ZSTR_VAL(txt));
        zend_throw_exception(zend_exception_get_default(), msg, 0);
        efree(msg);
        RETURN_THROWS();
    }
    zend_string *content = php_stream_copy_to_mem(s, PHP_STREAM_COPY_ALL, 0);
    php_stream_close(s);
    if (!content) {
        zend_throw_exception(zend_exception_get_default(),
            "chapter9: cannot read dictionary file", 0);
        RETURN_THROWS();
    }

    c9_builder b;
    c9_builder_init(&b);

    const char *data = ZSTR_VAL(content);
    size_t left = ZSTR_LEN(content);

    /* 跳过 UTF-8 BOM */
    if (left >= 3 && memcmp(data, "\xEF\xBB\xBF", 3) == 0) {
        data += 3;
        left -= 3;
    }

    /* 逐行：每行一个词，忽略空白行与首尾空白 */
    while (left > 0) {
        const char *nl = memchr(data, '\n', left);
        size_t llen = nl ? (size_t) (nl - data) : left;

        size_t st = 0, en = llen;
        while (st < en && (data[st] == ' ' || data[st] == '\t' || data[st] == '\r')) st++;
        while (en > st && (data[en-1] == ' ' || data[en-1] == '\t' || data[en-1] == '\r')) en--;

        if (en > st) {
            c9_builder_insert(&b, data + st, en - st);
        }

        if (!nl) break;
        data += llen + 1;
        left -= llen + 1;
    }

    zend_string_release(content);

    php_stream *out = php_stream_open_wrapper(ZSTR_VAL(bin), "wb", 0, NULL);
    if (!out) {
        char *msg;
        spprintf(&msg, 0, "chapter9: cannot open output file: %s", ZSTR_VAL(bin));
        zend_throw_exception(zend_exception_get_default(), msg, 0);
        efree(msg);
        c9_builder_free(&b);
        RETURN_THROWS();
    }

    zend_bool ok = c9_serialize(&b, out);
    php_stream_close(out);
    c9_builder_free(&b);

    if (ok != SUCCESS) {
        zend_throw_exception(zend_exception_get_default(),
            "chapter9: failed to write dictionary file", 0);
        RETURN_THROWS();
    }

    RETURN_TRUE;
}

/* bool chapter9_load(string $bin) */
PHP_FUNCTION(chapter9_load)
{
    zend_string *bin;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(bin)
    ZEND_PARSE_PARAMETERS_END();

    c9_trie *t = NULL;
    if (c9_load_trie(&t, ZSTR_VAL(bin)) != SUCCESS) {
        zend_throw_exception(zend_exception_get_default(),
            "chapter9: invalid dictionary file", 0);
        RETURN_THROWS();
    }

    if (CHAPTER9_G(trie) != NULL) {
        c9_trie_free((c9_trie *) CHAPTER9_G(trie));
    }
    CHAPTER9_G(trie) = t;

    RETURN_TRUE;
}

/* array chapter9_detect(string $text) */
PHP_FUNCTION(chapter9_detect)
{
    zend_string *text;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(text)
    ZEND_PARSE_PARAMETERS_END();

    c9_trie *t = c9_ensure_load();
    if (!t) {
        c9_throw_not_loaded();
        RETURN_THROWS();
    }

    uint32_t *cps;
    size_t n;
    c9_utf8_decode(ZSTR_VAL(text), ZSTR_LEN(text), &cps, NULL, &n);

    size_t hcnt;
    c9_hit *hits = c9_scan(t, cps, n, &hcnt);
    size_t scnt = c9_select(hits, hcnt);
    efree(cps);

    array_init(return_value);
    if (scnt == 0) {
        efree(hits);
        return;
    }

    /* 去重，保持首次命中顺序。
     * 命中词可能非常多（大文本 + 大词典），用线性表会退化成 O(n^2)，
     * 这里用 HashTable 做 O(1) 判重。 */
    HashTable seen;
    zend_hash_init(&seen, scnt, NULL, NULL, 0);
    zval marker;
    ZVAL_TRUE(&marker);
    for (size_t i = 0; i < scnt; i++) {
        const char *w = C9_TRIE_POOL(t) + hits[i].word_off;
        size_t wlen = strlen(w);
        /* zend_hash_str_add 的 pData 不能为 NULL（内部会解引用），
         * 传入一个真实 zval 即可；键已存在时返回 NULL，达到判重效果 */
        if (zend_hash_str_add(&seen, w, wlen, &marker) != NULL) {
            add_next_index_stringl(return_value, w, wlen);
        }
    }
    zend_hash_destroy(&seen);
    efree(hits);
}

/* string chapter9_replace(string $text, string $replacement = '*') */
PHP_FUNCTION(chapter9_replace)
{
    zend_string *text;
    zend_string *repl = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_STR(text)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR(repl)
    ZEND_PARSE_PARAMETERS_END();

    const char *rep_str = repl ? ZSTR_VAL(repl) : "*";
    size_t      rep_len = repl ? ZSTR_LEN(repl) : 1;

    c9_trie *t = c9_ensure_load();
    if (!t) {
        c9_throw_not_loaded();
        RETURN_THROWS();
    }

    uint32_t *cps, *offs;
    size_t n;
    c9_utf8_decode(ZSTR_VAL(text), ZSTR_LEN(text), &cps, &offs, &n);

    size_t hcnt;
    c9_hit *hits = c9_scan(t, cps, n, &hcnt);
    size_t scnt  = c9_select(hits, hcnt);

    /* 拼接输出：命中的码点换成 replacement（每个码点一个 replacement） */
    smart_str buf = {0};
    size_t s = 0;
    for (size_t i = 0; i < n; i++) {
        if (s < scnt && i >= hits[s].end) s++;
        if (s < scnt && i >= hits[s].start && i < hits[s].end) {
            smart_str_appendl(&buf, rep_str, rep_len);
        } else {
            smart_str_appendl(&buf, ZSTR_VAL(text) + offs[i], offs[i + 1] - offs[i]);
        }
    }
    smart_str_0(&buf);

    efree(cps);
    efree(offs);
    efree(hits);

    if (buf.s) {
        RETURN_STR(buf.s);
    }
    RETURN_EMPTY_STRING();
}

/* bool chapter9_has(string $text) */
PHP_FUNCTION(chapter9_has)
{
    zend_string *text;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(text)
    ZEND_PARSE_PARAMETERS_END();

    c9_trie *t = c9_ensure_load();
    if (!t) {
        c9_throw_not_loaded();
        RETURN_THROWS();
    }

    uint32_t *cps;
    size_t n;
    c9_utf8_decode(ZSTR_VAL(text), ZSTR_LEN(text), &cps, NULL, &n);

    size_t hcnt;
    c9_hit *hits = c9_scan(t, cps, n, &hcnt);
    size_t scnt  = c9_select(hits, hcnt);

    efree(cps);
    efree(hits);

    RETURN_BOOL(scnt > 0);
}

/* int chapter9_count(string $text) */
PHP_FUNCTION(chapter9_count)
{
    zend_string *text;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(text)
    ZEND_PARSE_PARAMETERS_END();

    c9_trie *t = c9_ensure_load();
    if (!t) {
        c9_throw_not_loaded();
        RETURN_THROWS();
    }

    uint32_t *cps;
    size_t n;
    c9_utf8_decode(ZSTR_VAL(text), ZSTR_LEN(text), &cps, NULL, &n);

    size_t hcnt;
    c9_hit *hits = c9_scan(t, cps, n, &hcnt);
    size_t scnt  = c9_select(hits, hcnt);

    efree(cps);
    efree(hits);

    RETURN_LONG((zend_long) scnt);
}

/* -------------------------------------------------------------------------
 * 11. MINFO / 模块入口
 * ---------------------------------------------------------------------- */
PHP_MINFO_FUNCTION(chapter9)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "chapter9 support", "enabled");
    php_info_print_table_row(2, "Version", PHP_CHAPTER9_VERSION);
    php_info_print_table_row(2, "Algorithm", "Trie + Aho-Corasick (DFA)");
    php_info_print_table_end();

    DISPLAY_INI_ENTRIES();
}

zend_module_entry chapter9_module_entry = {
    STANDARD_MODULE_HEADER,
    "chapter9",
    chapter9_functions,
    PHP_MINIT(chapter9),
    PHP_MSHUTDOWN(chapter9),
    NULL,
    NULL,
    PHP_MINFO(chapter9),
    PHP_CHAPTER9_VERSION,
    PHP_MODULE_GLOBALS(chapter9),
    PHP_GINIT(chapter9),
    NULL,
    NULL,
    STANDARD_MODULE_PROPERTIES_EX
};

#ifdef COMPILE_DL_CHAPTER9
ZEND_GET_MODULE(chapter9)
#endif```

</details>

<!-- 本章代码 end:chapter9 -->

