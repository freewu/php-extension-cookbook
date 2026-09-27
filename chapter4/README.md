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
