--TEST--
chapter5: 默认参数值
--SKIPIF--
<?php if (!extension_loaded('chapter5')) die('skip chapter5 not loaded'); ?>
--FILE--
<?php
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
?>
--EXPECT--
Hello, World!
Hello, PHP!
Hi, PHP!
ab,ab
ab,ab,ab
ab-ab-ab
int(9)
int(1024)
Hello
Hello Wo...
Hi
