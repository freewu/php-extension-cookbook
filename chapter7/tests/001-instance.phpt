--TEST--
chapter7: 实例化与实例方法
--SKIPIF--
<?php if (!extension_loaded('chapter7')) die('skip chapter7 not loaded'); ?>
--FILE--
<?php
$c = new Chapter7Counter(10);
var_dump($c->getValue());
var_dump($c->increment());
var_dump($c->increment(5));
var_dump($c->decrement(3));
var_dump($c->getValue());

$c->setValue(100);
var_dump($c->getValue());
?>
--EXPECT--
int(10)
int(11)
int(16)
int(13)
int(13)
int(100)
