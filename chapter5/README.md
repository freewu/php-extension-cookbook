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
