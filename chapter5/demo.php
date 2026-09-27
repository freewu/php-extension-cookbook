<?php
/*
 * Chapter 5 演示脚本
 *
 * 用法：
 *   php -d extension=$(pwd)/modules/chapter5.so demo.php
 */

echo chapter5_greet(), PHP_EOL;
echo chapter5_greet('PHP'), PHP_EOL;
echo chapter5_greet('PHP', 'Hi'), PHP_EOL;

echo chapter5_repeat('ab'), PHP_EOL;
echo chapter5_repeat('ab', 3), PHP_EOL;
echo chapter5_repeat('ab', 3, '-'), PHP_EOL;

var_dump(chapter5_pow(3));
var_dump(chapter5_pow(2, 10));

echo chapter5_slice('Hello World'), PHP_EOL;
echo chapter5_slice('Hello World', 8, true), PHP_EOL;
echo chapter5_slice('Hi'), PHP_EOL;

// 通过反射查看 arginfo 中声明的默认值
$r = new ReflectionFunction('chapter5_greet');
foreach ($r->getParameters() as $p) {
    printf("%s = %s\n", $p->getName(), var_export($p->getDefaultValue(), true));
}
