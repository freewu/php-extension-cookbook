--TEST--
chapter6: 具名参数
--SKIPIF--
<?php if (!extension_loaded('chapter6')) die('skip chapter6 not loaded'); ?>
--FILE--
<?php
var_export(chapter6_named(1, 2, a: 3, b: 4));
echo PHP_EOL;

$r = new ReflectionFunction('chapter6_join');
var_dump($r->isVariadic());
?>
--EXPECT--
array (
  0 => 1,
  1 => 2,
  'a' => 3,
  'b' => 4,
)
bool(true)
