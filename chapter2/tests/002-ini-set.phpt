--TEST--
chapter2: 运行时修改配置
--SKIPIF--
<?php if (!extension_loaded('chapter2')) die('skip chapter2 not loaded'); ?>
--INI--
chapter2.limit=1
--FILE--
<?php
ini_set('chapter2.greeting', 'Hi');
ini_set('chapter2.name', 'PHP');
echo chapter2_greet(), PHP_EOL;

var_dump(chapter2_set_greeting('Hey'));
echo chapter2_greet(), PHP_EOL;
?>
--EXPECT--
Hi PHP
bool(true)
Hey PHP
