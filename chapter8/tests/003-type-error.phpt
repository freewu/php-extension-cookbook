--TEST--
chapter8: 资源类型不匹配抛 TypeError
--SKIPIF--
<?php if (!extension_loaded('chapter8')) die('skip chapter8 not loaded'); ?>
--FILE--
<?php
$fp = fopen('php://memory', 'r+');
try {
    chapter8_read($fp);
} catch (TypeError $e) {
    echo "TypeError\n";
}
fclose($fp);

try {
    chapter8_read('not a resource');
} catch (TypeError $e) {
    echo "TypeError\n";
}
?>
--EXPECT--
TypeError
TypeError
