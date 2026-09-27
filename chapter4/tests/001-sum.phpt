--TEST--
chapter4: 数组求和
--SKIPIF--
<?php if (!extension_loaded('chapter4')) die('skip chapter4 not loaded'); ?>
--FILE--
<?php
var_dump(chapter4_sum([1, 2, 3, 4]));
var_dump(chapter4_sum(['1.5', 2, 2.5]));
var_dump(chapter4_sum([]));
?>
--EXPECT--
float(10)
float(6)
float(0)
