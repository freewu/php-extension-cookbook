# php-extension-cookbook

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![PHP](https://img.shields.io/badge/PHP-8.0%2B-777BB4?logo=php&logoColor=white)](https://www.php.net/)
[![Zend Engine](https://img.shields.io/badge/Zend%20Engine-C-A8B9CC?logo=c&logoColor=white)](https://github.com/php/php-src)
[![phpize](https://img.shields.io/badge/build-phpize%20%2B%20make-success)](https://www.php.net/manual/zh/internals2.buildsys.php)
![Platform](https://img.shields.io/badge/platform-Linux-FCC624?logo=linux&logoColor=black)
![Tests](https://img.shields.io/badge/tests-28%20passed-brightgreen)

基于 PHP 8 的扩展开发教程。

从零开始，用 9 个循序渐进的章节，覆盖 PHP 扩展开发的核心知识点：
环境搭建、函数、参数与返回值、数组、默认参数、可变参数、类、资源、数据结构（DFA 敏感词检测）。

## Plan

| 章节 | 主题 | 内容 |
| --- | --- | --- |
| [chapter1](chapter1/) | Hello World | 开发环境搭建，实现返回 `"Hello World"` 的函数 |
| [chapter2](chapter2/) | 扩展配置 | 注册 INI 配置项，在函数里读取 / 修改 |
| [chapter3](chapter3/) | 参数与返回值 | 获取函数参数，返回标量，抛出异常 |
| [chapter4](chapter4/) | 数组 | 数组类型参数、遍历、数组类型返回值 |
| [chapter5](chapter5/) | 默认值 | 带默认值的可选参数，arginfo 默认值 |
| [chapter6](chapter6/) | 可变参数 | 无固定参数函数 `...$args`，具名参数 |
| [chapter7](chapter7/) | 类 | 自定义对象结构体、方法、属性、类常量 |
| [chapter8](chapter8/) | 资源 | 注册资源类型、创建 / 取回 / 析构资源 |
| [chapter9](chapter9/) | DFA 敏感词检测 | Trie + Aho-Corasick，txt 构建 .bin 词典，命中检测与替换 |

每个章节都是一个**独立可编译的扩展**，包含：

```
chapterN/
├── config.m4          # autoconf 配置
├── php_chapterN.h     # 头文件
├── chapterN.c         # 扩展实现（含详细中文注释）
├── demo.php           # 演示脚本
├── README.md          # 本章知识点讲解
└── tests/*.phpt       # phpt 测试用例
```

## 环境要求

- PHP 8.0+（本项目在 PHP 8.3 上开发、测试）
- PHP 开发头文件：`phpize`、`php-config`
- 编译工具链：`gcc`、`make`、`autoconf`、`pkg-config`

Ubuntu / Debian：

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

## 快速开始

编译并测试全部章节：

```bash
./build.sh
```

只编译某一章：

```bash
cd chapter1
phpize
./configure --enable-chapter1
make

php -d extension=$(pwd)/modules/chapter1.so demo.php
# Hello World

make test
```

清理编译产物：

```bash
./build.sh clean
```

## 运行说明

扩展编译产物位于 `chapterN/modules/chapterN.so`，运行时用 `-d extension=` 加载：

```bash
php -d extension=$(pwd)/modules/chapter1.so demo.php
```

也可以把 `.so` 复制到 `php-config --extension-dir` 指向的目录，并在 `php.ini` 中添加：

```ini
extension=chapter1.so
```

## 文档与电子书

项目内置 [docsify](https://docsify.js.org/) 支持：仓库根目录的 `index.html`
就是文档入口，任意静态服务器都可以直接打开为在线手册。

```bash
# 方式一：只要 Python 就能跑
python3 -m http.server 8000
# 打开 http://localhost:8000

# 方式二：用 docsify CLI
npx docsify-cli serve .
```

- `_sidebar.md`：侧边栏导航，由 `scripts/gen-docs.sh` 自动生成
  （按章节排序，标题取自各章 README）
- `ebook.md`：**单文件电子书**（全章节 + 目录锚点），同样由脚本生成
- `scripts/build-ebook.sh`：用 [pandoc](https://pandoc.org/) 把
  `ebook.md` 转成 `php-extension-cookbook.epub`（`bash scripts/build-ebook.sh`），
  直接导入微信读书 / Calibre / Kindle；`pdf` 亦可（需 LaTeX）

> 重新生成导航：`bash scripts/gen-docs.sh`（新增章节后执行一次即可）

### 部署到 GitHub Pages

仓库已内置 Pages 发布所需的一切：

- `.nojekyll`：关闭 Jekyll，避免 `_sidebar.md` 等下划线文件被忽略
- `.github/workflows/pages.yml`：push 到 `main` 后自动把**仓库根目录**
  （即 docsify 站点）发布到 Pages，并顺带构建 `epub` 电子书制品

开启方式（二选一）：

```text
1. 自动：推送 pages.yml 到 main 后，每次 push 自动部署（推荐）
2. 手动：Settings → Pages → Source: Deploy from a branch → main / (root)
```

发布后访问：

```text
https://<username>.github.io/php-extension-cookbook/
```

### 每章附完整代码

每个 `chapterN/README.md` 末尾都有 `<details>` 折叠块，内含该章扩展的
**完整 C 源码（带中文注释）**，可直接在 GitHub / docsify / 电子书中阅读：

```bash
bash scripts/sync-code-readme.sh   # 改过 .c 后重新同步一次，幂等
```

## 参考

- [PHP 官方文档 - 扩展开发](https://www.php.net/manual/zh/internals2.php)
- [Zend Engine 源码](https://github.com/php/php-src)

## License

本项目基于 [MIT License](LICENSE) 开源。

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
