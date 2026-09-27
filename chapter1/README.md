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

| 概念 | 说明 |
| --- | --- |
| `PHP_FUNCTION(name)` | 声明用户空间可调用的函数 |
| `ZEND_BEGIN_ARG_INFO_EX` | PHP 8 必需，描述函数签名（反射/类型检查） |
| `zend_function_entry` | 函数表，把函数名和 C 实现绑定 |
| `zend_module_entry` | 模块入口，定义生命周期回调与版本 |
| `PHP_MINFO_FUNCTION` | `phpinfo()` 输出的模块信息 |
| `ZEND_GET_MODULE` | 动态库方式加载的入口 |

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
