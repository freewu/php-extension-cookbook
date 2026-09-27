--TEST--
chapter7: 静态方法、类常量、克隆
--SKIPIF--
<?php if (!extension_loaded('chapter7')) die('skip chapter7 not loaded'); ?>
--FILE--
<?php
$e = Chapter7Counter::create(3);
var_dump($e instanceof Chapter7Counter);
var_dump($e->getValue());

var_dump(Chapter7Counter::DEFAULT_STEP);

$a = new Chapter7Counter(5);
$b = clone $a;
$b->increment(100);
var_dump($a->getValue());
var_dump($b->getValue());
?>
--EXPECT--
bool(true)
int(3)
int(1)
int(5)
int(105)
