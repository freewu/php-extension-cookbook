--TEST--
chapter3: 参数类型错误会抛 TypeError
--SKIPIF--
<?php if (!extension_loaded('chapter3')) die('skip chapter3 not loaded'); ?>
--FILE--
<?php
try {
    chapter3_add([], 1);
} catch (TypeError $e) {
    echo "TypeError\n";
}
?>
--EXPECT--
TypeError
