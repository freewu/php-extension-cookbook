<?php
/*
 * Chapter 3 演示脚本
 *
 * 用法：
 *   php -d extension=$(pwd)/modules/chapter3.so demo.php
 */

var_dump(chapter3_add(1, 2));
var_dump(chapter3_add('3', '4'));          // 弱类型模式下会转换

var_dump(chapter3_concat('Hello', 'World'));
var_dump(chapter3_upper('hello php'));

var_dump(chapter3_divide(10, 4));

try {
    chapter3_divide(1, 0);
} catch (DivisionByZeroError $e) {
    echo '捕获异常: ', $e->getMessage(), PHP_EOL;
}

foreach ([1, 1.5, 'x', true, null, [1, 2], new stdClass()] as $v) {
    echo chapter3_type_of($v), PHP_EOL;
}
