--TEST--
chapter5: 反射能读取默认值
--SKIPIF--
<?php if (!extension_loaded('chapter5')) die('skip chapter5 not loaded'); ?>
--FILE--
<?php
foreach (['chapter5_greet', 'chapter5_pow', 'chapter5_slice'] as $fn) {
    $r = new ReflectionFunction($fn);
    echo $r->getName(), ': ';
    $parts = [];
    foreach ($r->getParameters() as $p) {
        $parts[] = $p->isOptional()
            ? sprintf('%s=%s', $p->getName(), var_export($p->getDefaultValue(), true))
            : $p->getName();
    }
    echo implode(', ', $parts), PHP_EOL;
}
?>
--EXPECT--
chapter5_greet: name='World', greeting='Hello'
chapter5_pow: base, exponent=2
chapter5_slice: text, length=5, ellipsis=false
