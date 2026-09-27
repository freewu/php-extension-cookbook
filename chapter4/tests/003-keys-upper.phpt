--TEST--
chapter4: 键转大写
--SKIPIF--
<?php if (!extension_loaded('chapter4')) die('skip chapter4 not loaded'); ?>
--FILE--
<?php
print_r(chapter4_keys_upper([
    'name' => 'php',
    'lang' => 'c',
    10     => 'numeric key',
]));
?>
--EXPECT--
Array
(
    [NAME] => php
    [LANG] => c
    [10] => numeric key
)
