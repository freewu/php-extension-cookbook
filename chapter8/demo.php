<?php
/*
 * Chapter 8 演示脚本
 *
 * 用法：
 *   php -d extension=$(pwd)/modules/chapter8.so demo.php
 */

$h = chapter8_open('Hello');
var_dump(get_resource_type($h));
var_dump(chapter8_read($h));
var_dump(chapter8_write($h, ', World'));
var_dump(chapter8_read($h));
var_dump(chapter8_length($h));

chapter8_close($h);
try {
    chapter8_read($h);
} catch (TypeError $e) {
    echo '已关闭: ', $e->getMessage(), PHP_EOL;
}

// 资源类型不匹配
$fp = fopen('php://memory', 'r+');
try {
    chapter8_read($fp);
} catch (TypeError $e) {
    echo '类型错误: ', $e->getMessage(), PHP_EOL;
}
fclose($fp);

// 超出作用域后由 GC 自动释放（无需手动 close）
$tmp = chapter8_open(str_repeat('x', 1024));
unset($tmp);
echo "GC 释放完成\n";
