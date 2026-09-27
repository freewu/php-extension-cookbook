--TEST--
chapter3: 标量参数与返回值
--SKIPIF--
<?php if (!extension_loaded('chapter3')) die('skip chapter3 not loaded'); ?>
--FILE--
<?php
var_dump(chapter3_add(1, 2));
var_dump(chapter3_add('3', '4'));
var_dump(chapter3_concat('Hello', 'World'));
var_dump(chapter3_upper('hello php'));
var_dump(chapter3_divide(10, 4));
?>
--EXPECT--
int(3)
int(7)
string(10) "HelloWorld"
string(9) "HELLO PHP"
float(2.5)
