--TEST--
chapter8: 创建、写入、读取资源
--SKIPIF--
<?php if (!extension_loaded('chapter8')) die('skip chapter8 not loaded'); ?>
--FILE--
<?php
$h = chapter8_open('Hello');
var_dump(get_resource_type($h));
var_dump(chapter8_read($h));
var_dump(chapter8_write($h, ', World'));
var_dump(chapter8_read($h));
var_dump(chapter8_length($h));

$e = chapter8_open();
var_dump(chapter8_read($e));
var_dump(chapter8_length($e));
?>
--EXPECT--
string(15) "chapter8_buffer"
string(5) "Hello"
int(12)
string(12) "Hello, World"
int(12)
string(0) ""
int(0)
