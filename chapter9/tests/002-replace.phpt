--TEST--
chapter9: 替换命中的敏感词
--SKIPIF--
<?php if (!extension_loaded('chapter9')) die('skip chapter9 not loaded'); ?>
--FILE--
<?php
$txt = __DIR__ . '/../words.txt';
$bin = __DIR__ . '/replace.bin';

chapter9_build($txt, $bin);
chapter9_load($bin);

var_dump(chapter9_replace('今晚去赌场赌博'));
var_dump(chapter9_replace('今晚去赌场赌博', '好'));
var_dump(chapter9_replace('fuck this shit', ''));
var_dump(chapter9_replace('完全正常的一句话'));

unlink($bin);
?>
--EXPECT--
string(13) "今晚去****"
string(21) "今晚去好好好好"
string(6) " this "
string(24) "完全正常的一句话"