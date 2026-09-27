--TEST--
chapter7: 反射
--SKIPIF--
<?php if (!extension_loaded('chapter7')) die('skip chapter7 not loaded'); ?>
--FILE--
<?php
$r = new ReflectionClass('Chapter7Counter');
echo $r->getName(), PHP_EOL;
echo implode(',', array_map(fn($m) => $m->getName(), $r->getMethods())), PHP_EOL;
var_dump($r->hasConstant('DEFAULT_STEP'));
var_dump($r->hasProperty('label'));
?>
--EXPECT--
Chapter7Counter
__construct,increment,decrement,getValue,setValue,getLabel,setLabel,create
bool(true)
bool(true)
