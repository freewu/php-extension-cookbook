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
## 完整代码（chapter7.c）

> 本章扩展的完整 C 源码，已带中文注释。由 [`scripts/sync-code-readme.sh`](../scripts/sync-code-readme.sh) 自动同步；以源码文件 [`chapter7.c`](chapter7.c) 为准。


## 完整代码（chapter7.c）

> 本章扩展的完整 C 源码，已带中文注释。由 [`scripts/sync-code-readme.sh`](../scripts/sync-code-readme.sh) 自动同步；以源码文件 [`chapter7.c`](chapter7.c) 为准。

<!-- 本章代码 begin -->

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

<!-- 本章代码 end -->

