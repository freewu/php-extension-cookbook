--TEST--
chapter9: 构建字典并检测命中
--SKIPIF--
<?php if (!extension_loaded('chapter9')) die('skip chapter9 not loaded'); ?>
--FILE--
<?php
$txt = __DIR__ . '/../words.txt';
$bin = __DIR__ . '/build.bin';

if (!file_exists($txt)) { echo "words.txt missing\n"; exit; }

var_dump(chapter9_build($txt, $bin));
var_dump(chapter9_load($bin));

var_dump(chapter9_detect('今晚赌场赌博，还吸了点毒品'));
var_dump(chapter9_detect('今天天气真不错'));
var_dump(chapter9_has('不要用私服外挂'));
var_dump(chapter9_has('完全干净'));
var_dump(chapter9_count('fuck, this is nonsense!'));
var_dump(chapter9_count('nothing at all'));

unlink($bin);
?>
--EXPECT--
bool(true)
bool(true)
array(3) {
  [0]=>
  string(6) "赌场"
  [1]=>
  string(6) "赌博"
  [2]=>
  string(6) "毒品"
}
array(0) {
}
bool(true)
bool(false)
int(1)
int(0)