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
