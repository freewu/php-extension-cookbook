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
