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
