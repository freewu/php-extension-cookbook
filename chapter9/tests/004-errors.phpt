--TEST--
chapter9: 错误处理
--SKIPIF--
<?php if (!extension_loaded('chapter9')) die('skip chapter9 not loaded'); ?>
--FILE--
<?php
try {
    chapter9_detect('x');
    echo "no error\n";
} catch (Exception $e) {
    echo get_class($e), "\n";
}

try {
    chapter9_build('/no/such/words.txt', '/tmp/c9x.bin');
    echo "no error\n";
} catch (Exception $e) {
    echo 'build: ', $e->getMessage(), "\n";
}

file_put_contents('/tmp/c9bad.bin', 'not a dictionary');
try {
    chapter9_load('/tmp/c9bad.bin');
    echo "no error\n";
} catch (Exception $e) {
    echo 'load: ', $e->getMessage(), "\n";
}
@unlink('/tmp/c9bad.bin');
?>
--EXPECT--
Exception
build: chapter9: cannot open dictionary file: /no/such/words.txt
load: chapter9: invalid dictionary file