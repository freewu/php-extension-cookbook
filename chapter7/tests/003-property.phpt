--TEST--
chapter7: 属性读写
--SKIPIF--
<?php if (!extension_loaded('chapter7')) die('skip chapter7 not loaded'); ?>
--FILE--
<?php
$c = new Chapter7Counter();
var_dump($c->getLabel());
$c->setLabel('my counter');
var_dump($c->getLabel());
var_dump($c->label);
?>
--EXPECT--
string(0) ""
string(10) "my counter"
string(10) "my counter"
