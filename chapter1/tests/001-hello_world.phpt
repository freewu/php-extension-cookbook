--TEST--
chapter1: hello_world() 返回 Hello World
--SKIPIF--
<?php if (!extension_loaded('chapter1')) die('skip chapter1 not loaded'); ?>
--FILE--
<?php
var_dump(hello_world());
echo hello_world(), PHP_EOL;
?>
--EXPECT--
string(11) "Hello World"
Hello World
