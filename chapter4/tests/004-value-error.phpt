--TEST--
chapter4: 非法参数抛 ValueError
--SKIPIF--
<?php if (!extension_loaded('chapter4')) die('skip chapter4 not loaded'); ?>
--FILE--
<?php
try {
    chapter4_squares(-1);
} catch (ValueError $e) {
    echo 'ValueError: ', $e->getMessage(), PHP_EOL;
}
?>
--EXPECT--
ValueError: chapter4_squares(): Argument #1 ($n) must be greater than or equal to 0
