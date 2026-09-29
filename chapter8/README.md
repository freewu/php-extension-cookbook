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
## 完整代码（chapter8.c）

> 本章扩展的完整 C 源码，已带中文注释。由 [`scripts/sync-code-readme.sh`](../scripts/sync-code-readme.sh) 自动同步；以源码文件 [`chapter8.c`](chapter8.c) 为准。


## 完整代码（chapter8.c）

> 本章扩展的完整 C 源码，已带中文注释。由 [`scripts/sync-code-readme.sh`](../scripts/sync-code-readme.sh) 自动同步；以源码文件 [`chapter8.c`](chapter8.c) 为准。

<!-- 本章代码 begin -->

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

<!-- 本章代码 end -->

