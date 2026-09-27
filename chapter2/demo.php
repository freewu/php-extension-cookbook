<?php
/*
 * Chapter 2 演示脚本
 *
 * 用法：
 *   php -d extension=$(pwd)/modules/chapter2.so demo.php
 * 或在命令行覆盖配置：
 *   php -d extension=$(pwd)/modules/chapter2.so \
 *       -d chapter2.limit=1 -d chapter2.name=PHP demo.php
 */

echo '1) 默认配置: ', chapter2_greet(), PHP_EOL;
var_dump(chapter2_is_enabled());
echo '   ', chapter2_get_config(), PHP_EOL;

// 通过 ini_set 修改（PHP_INI_ALL 才允许）
ini_set('chapter2.name', 'PHP');
ini_set('chapter2.limit', '1');
echo '2) ini_set 之后: ', chapter2_greet(), PHP_EOL;
echo '   ', chapter2_get_config(), PHP_EOL;

// 通过扩展提供的函数修改
var_dump(chapter2_set_greeting('Hi'));
echo '3) 修改 greeting 之后: ', chapter2_greet(), PHP_EOL;
