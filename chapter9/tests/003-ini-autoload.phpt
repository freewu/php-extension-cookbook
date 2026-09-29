--TEST--
chapter9: 通过 chapter9.bin 配置自动加载
--SKIPIF--
<?php if (!extension_loaded('chapter9')) die('skip chapter9 not loaded'); ?>
--FILE--
<?php
$txt = __DIR__ . '/../words.txt';
$bin = __DIR__ . '/auto.bin';

chapter9_build($txt, $bin);
ini_set('chapter9.bin', $bin);

var_dump(chapter9_detect('这里没有敏感词'));
var_dump(chapter9_detect('交易毒品和私服外挂'));

unlink($bin);
?>
--EXPECT--
array(0) {
}
array(3) {
  [0]=>
  string(6) "毒品"
  [1]=>
  string(6) "私服"
  [2]=>
  string(6) "外挂"
}