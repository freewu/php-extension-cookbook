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
