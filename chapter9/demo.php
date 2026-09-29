<?php
/*
 * Chapter 9 演示脚本
 *
 * 用法：
 *   php -d extension=$(pwd)/modules/chapter9.so demo.php
 */

$txt = __DIR__ . '/words.txt';
$bin = __DIR__ . '/words.bin';

/* 1. 把 txt（每行一个词）构建成 .bin */
var_dump(chapter9_build($txt, $bin));

/* 2. 配置 chapter9.bin，之后无需手动 load */
ini_set('chapter9.bin', $bin);

$text = '今晚去赌场打麻将，还吸了点毒品，顺便给别人代购了东西。fuck this shit!';

echo "原文: {$text}\n";
echo "命中: ";
var_dump(chapter9_detect($text));

echo "替换: " . chapter9_replace($text) . "\n";
echo "替换(自定义): " . chapter9_replace($text, '口') . "\n";
echo "替换(去词): " . chapter9_replace($text, '') . "\n";

var_dump(chapter9_has('这里没有敏感词'));
var_dump(chapter9_count('fuck 和 gambling 都是敏感词'));

/* 3. 也可以显式 load 一个 .bin（覆盖自动加载） */
var_dump(chapter9_load($bin));

/* 查看生成的文件 */
printf("bin 大小: %d 字节\n", filesize($bin));