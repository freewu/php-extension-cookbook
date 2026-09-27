<?php
/*
 * Chapter 4 演示脚本
 *
 * 用法：
 *   php -d extension=$(pwd)/modules/chapter4.so demo.php
 */

var_dump(chapter4_sum([1, 2, 3, 4]));
var_dump(chapter4_sum(['1.5', 2, 2.5]));

print_r(chapter4_squares(5));

print_r(chapter4_keys_upper([
    'name' => 'php',
    'lang' => 'c',
    10     => 'numeric key',
]));

print_r(chapter4_filter_even([1, 2, 3, 4, 5, 6]));

try {
    chapter4_squares(-1);
} catch (ValueError $e) {
    echo '捕获异常: ', $e->getMessage(), PHP_EOL;
}
