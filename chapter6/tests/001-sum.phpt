--TEST--
chapter6: 可变参数求和
--SKIPIF--
<?php if (!extension_loaded('chapter6')) die('skip chapter6 not loaded'); ?>
--FILE--
<?php
var_dump(chapter6_sum());
var_dump(chapter6_sum(1, 2, 3));
var_dump(chapter6_sum(1.5, 2, 3.5));
?>
--EXPECT--
float(0)
float(6)
float(7)
