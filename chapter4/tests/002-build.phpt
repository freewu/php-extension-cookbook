--TEST--
chapter4: 构造数组返回值
--SKIPIF--
<?php if (!extension_loaded('chapter4')) die('skip chapter4 not loaded'); ?>
--FILE--
<?php
print_r(chapter4_squares(5));
print_r(chapter4_filter_even([1, 2, 3, 4, 5, 6]));
?>
--EXPECT--
Array
(
    [0] => 0
    [1] => 1
    [2] => 4
    [3] => 9
    [4] => 16
)
Array
(
    [0] => 2
    [1] => 4
    [2] => 6
)
