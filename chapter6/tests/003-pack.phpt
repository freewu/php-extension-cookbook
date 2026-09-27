--TEST--
chapter6: 打包为数组
--SKIPIF--
<?php if (!extension_loaded('chapter6')) die('skip chapter6 not loaded'); ?>
--FILE--
<?php
var_export(chapter6_pack('a', 1, [2, 3]));
echo PHP_EOL;
var_export(chapter6_pack());
echo PHP_EOL;
?>
--EXPECT--
array (
  0 => 'a',
  1 => 1,
  2 => 
  array (
    0 => 2,
    1 => 3,
  ),
)
array (
)
