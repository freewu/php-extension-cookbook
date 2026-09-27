--TEST--
chapter8: 关闭后资源失效
--SKIPIF--
<?php if (!extension_loaded('chapter8')) die('skip chapter8 not loaded'); ?>
--FILE--
<?php
$h = chapter8_open('abc');
var_dump(chapter8_read($h));

chapter8_close($h);
var_dump(get_resource_type($h));

try {
    chapter8_read($h);
} catch (TypeError $e) {
    echo "TypeError\n";
}
?>
--EXPECT--
string(3) "abc"
string(7) "Unknown"
TypeError
