<?php
/*
 * Chapter 6 演示脚本
 *
 * 用法：
 *   php -d extension=$(pwd)/modules/chapter6.so demo.php
 */

var_dump(chapter6_sum());
var_dump(chapter6_sum(1, 2, 3));
var_dump(chapter6_sum(1.5, 2, 3.5));

echo chapter6_join(','), PHP_EOL;
echo chapter6_join('-', 'a', 'b', 'c'), PHP_EOL;
echo chapter6_join(' | ', 'x', 1, 2.5, true), PHP_EOL;

print_r(chapter6_pack('a', 1, [2, 3]));
print_r(chapter6_pack());

// 具名参数
print_r(chapter6_named(1, 2, a: 3, b: 4));

$r = new ReflectionFunction('chapter6_join');
var_dump($r->isVariadic());
foreach ($r->getParameters() as $p) {
    printf("%s variadic=%s\n", $p->getName(), $p->isVariadic() ? 'yes' : 'no');
}
