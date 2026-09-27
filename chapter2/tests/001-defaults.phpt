--TEST--
chapter2: 默认配置
--SKIPIF--
<?php if (!extension_loaded('chapter2')) die('skip chapter2 not loaded'); ?>
--FILE--
<?php
var_dump(chapter2_greet());
var_dump(chapter2_is_enabled());
echo chapter2_get_config(), PHP_EOL;
?>
--EXPECT--
string(37) "Hello World, Hello World, Hello World"
bool(true)
enabled=1; limit=3; greeting=Hello; name=World
