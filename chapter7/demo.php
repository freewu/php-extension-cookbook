<?php
/*
 * Chapter 7 演示脚本
 *
 * 用法：
 *   php -d extension=$(pwd)/modules/chapter7.so demo.php
 */

$c = new Chapter7Counter(10);
var_dump($c->getValue());
var_dump($c->increment());
var_dump($c->increment(5));
var_dump($c->decrement(3));
var_dump($c->getValue());

$c->setValue(100);
var_dump($c->getValue());

$c->setLabel('my counter');
var_dump($c->getLabel(), $c->label);

$d = clone $c;
$d->increment(1000);
var_dump($c->getValue(), $d->getValue());

$e = Chapter7Counter::create(3);
var_dump($e instanceof Chapter7Counter, $e->getValue());

var_dump(Chapter7Counter::DEFAULT_STEP);

$r = new ReflectionClass('Chapter7Counter');
echo 'class: ', $r->getName(), PHP_EOL;
foreach ($r->getMethods() as $m) {
    echo '  - ', $m->getName(), $m->isStatic() ? ' (static)' : '', PHP_EOL;
}
