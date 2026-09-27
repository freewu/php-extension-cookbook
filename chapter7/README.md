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
