--TEST--
chapter6: 固定参数 + 可变参数
--SKIPIF--
<?php if (!extension_loaded('chapter6')) die('skip chapter6 not loaded'); ?>
--FILE--
<?php
echo chapter6_join(','), PHP_EOL;
echo chapter6_join('-', 'a', 'b', 'c'), PHP_EOL;
echo chapter6_join(' | ', 'x', 1, 2.5, true), PHP_EOL;
?>
--EXPECT--
a-b-c
x | 1 | 2.5 | 1
