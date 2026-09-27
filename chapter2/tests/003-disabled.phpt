--TEST--
chapter2: 禁用后返回空字符串
--SKIPIF--
<?php if (!extension_loaded('chapter2')) die('skip chapter2 not loaded'); ?>
--INI--
chapter2.enabled=0
--FILE--
<?php
var_dump(chapter2_is_enabled());
var_dump(chapter2_greet());
?>
--EXPECT--
bool(false)
string(0) ""
