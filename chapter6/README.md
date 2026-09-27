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
