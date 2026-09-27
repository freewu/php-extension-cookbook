--TEST--
chapter3: 除零抛出 DivisionByZeroError
--SKIPIF--
<?php if (!extension_loaded('chapter3')) die('skip chapter3 not loaded'); ?>
--FILE--
<?php
try {
    chapter3_divide(1, 0);
} catch (DivisionByZeroError $e) {
    echo get_class($e), ': ', $e->getMessage(), PHP_EOL;
}
?>
--EXPECT--
DivisionByZeroError: Division by zero
