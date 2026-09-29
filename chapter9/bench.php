<?php
/**
 * chapter9 性能测试脚本
 *
 * 生成一份“大词典 + 大文本”，分别测量：
 *   - 词典构建 build / 加载 load 的耗时
 *   - detect（取出命中）、replace（替换）、has 的吞吐（MiB/s）
 *   - 与 PCRE 正则过滤做基线对比，直观体会 DFA 的优势
 *
 * 用法（在 chapter9 目录下）：
 *   php -d extension=$(pwd)/modules/chapter9.so bench.php
 *   php -d extension=$(pwd)/modules/chapter9.so bench.php --words=200000 --text-mb=40
 *
 * 选项：
 *   --words=N     词典词数          默认 50000
 *   --text-mb=N   测试文本大小(MiB) 默认 10
 *   --seed=N      RNG 种子         默认 12345
 *   --runs=N      每个操作重复次数  默认 3（取最优）
 *   --re-mb=N     PCRE 基线切片大小 默认 1 (MiB)
 *   --re-words=N  PCRE 基线词数    默认 min(words,1000)，越大差距越明显
 *   --skip-re     跳过 PCRE 基线
 */

if (!extension_loaded('chapter9')) {
    fwrite(STDERR, "chapter9 扩展未加载\n提示: php -d extension=" . __DIR__ . "/modules/chapter9.so bench.php\n");
    exit(2);
}

/* ---------------- 参数解析 ---------------- */
$opts = getopt('', ['words::', 'text-mb::', 'seed::', 'runs::', 're-mb::', 're-words::', 'skip-re']);
$wordsN  = max(10, (int) ($opts['words']   ?? 50000));
$textMb  = max(1,  (int) ($opts['text-mb'] ?? 10));
$seed    = (int) ($opts['seed']   ?? 12345);
$runs    = max(1,  (int) ($opts['runs']   ?? 3));
$reMb    = max(1,  (int) ($opts['re-mb']  ?? 1));
$reWords = isset($opts['re-words']) ? max(1, (int) $opts['re-words']) : min($wordsN, 1000);
$skipRe  = isset($opts['skip-re']);

const MIB = 1048576;

/* ---------------- 小工具 ---------------- */
/** 返回 [耗时毫秒, 回调返回值] */
function ms(callable $fn): array {
    $t0 = hrtime(true);
    $r  = $fn();
    return [(hrtime(true) - $t0) / 1e6, $r];
}

/** 重复 N 次取最优耗时 */
function best_ms(callable $fn, int $n): array {
    $best = PHP_FLOAT_MAX;
    $ret  = null;
    for ($i = 0; $i < $n; $i++) {
        [$t, $r] = ms($fn);
        if ($t < $best) { $best = $t; $ret = $r; }
    }
    return [$best, $ret];
}

function fmt_mbs(float $bytes, float $sec): string {
    return sprintf("%.2f MiB/s", $bytes / MIB / ($sec / 1000));
}

function fmt_ms(float $ms): string {
    if ($ms >= 1000) return sprintf("%.3f s", $ms / 1000);
    if ($ms >= 1)    return sprintf("%.2f ms", $ms);
    return sprintf("%.3f ms", $ms);
}

/* ---------------- 1. 生成词典 ---------------- */
function gen_words(int $n, int $seed): array {
    mt_srand($seed);
    /* 不依赖 mbstring：用 u 模式拆分 UTF-8 字符串 */
    $cn    = preg_split('//u', '赌博色情毒品诈骗私服外挂代购翻墙的是一不在了和有这为国上大个会都来我你他她对说而于子就那得着过下出里可没有用今明天地人年月日时分秒非常很比较好', -1, PREG_SPLIT_NO_EMPTY);
    $ascii = str_split('abcdefghijklmnopqrstuvwxyz');

    $words = [];
    while (count($words) < $n) {
        if (mt_rand(0, 1) === 0) {
            $len = mt_rand(2, 4);
            $w = '';
            for ($i = 0; $i < $len; $i++) $w .= $cn[mt_rand(0, count($cn) - 1)];
        } else {
            $len = mt_rand(4, 8);
            $w = '';
            for ($i = 0; $i < $len; $i++) $w .= $ascii[mt_rand(0, 25)];
        }
        $words[$w] = true;
    }
    return array_keys($words);
}

/* ---------------- 2. 生成测试文本 ---------------- */
function gen_text(array $words, int $mb, int $seed): string {
    mt_srand($seed);
    $base = str_repeat('天地玄黄宇宙洪荒日月盈昃辰宿列张寒来暑往秋收冬藏闰余成岁律吕调阳云腾致雨露结为霜金生丽水玉出昆冈剑号巨阙珠称夜光果珍李柰菜重芥姜海咸河淡鳞潜羽翔龙师火帝鸟官人皇始制文字乃服衣裳推位让国吊民伐罪坐朝问道垂拱平章爱育黎首臣伏戎羌遐迩壹体率宾归王', 4);

    $target = $mb * MIB;
    $chunks = [];
    $used   = 0;
    $count  = count($words);
    $blen   = strlen($base);

    while ($used < $target) {
        $take = min(8192, $target - $used);
        $p   = '';
        $cur = 0;
        while ($cur < $take) {
            /* 填充普通中文 */
            $off = 3 * mt_rand(0, intdiv($blen, 3) - 2);
            $sub = substr($base, $off, 3 * mt_rand(2, 20));
            $p  .= $sub;
            $cur += strlen($sub);
            /* 约 1/4 概率插入一个敏感词，保证有命中 */
            if (mt_rand(1, 4) === 1) {
                $w = $words[mt_rand(0, $count - 1)];
                $p .= $w;
                $cur += strlen($w);
            }
        }
        $chunks[] = $p;
        $used    += strlen($p);
    }
    return implode('', $chunks);
}

/* 把切片尾部可能被截断的多字节字符去掉，保证字节安全 */
function byte_safe_slice(string $s, int $bytes): string {
    $s = substr($s, 0, $bytes);
    while ($s !== '' && (ord($s[strlen($s) - 1]) & 0xC0) === 0x80) {
        $s = substr($s, 0, -1);
    }
    return $s;
}

/* ---------------- 3. 输出 ---------------- */
function hr(string $s = ''): void {
    echo str_pad('', 72, '='), PHP_EOL;
    if ($s !== '') echo $s, PHP_EOL;
}

function row(string $label, string $value): void {
    printf("%-34s %s\n", $label, $value);
}

/* ---------------- main ---------------- */
ini_set('memory_limit', '-1');
echo PHP_EOL;
hr('chapter9 性能测试');
row('PHP', phpversion());
row('扩展版本', phpversion('chapter9') ?: 'chapter9');
row('词典词数', number_format($wordsN));
row('测试文本', sprintf('%d MiB', $textMb));
row('重复次数', $runs);
row('RNG 种子', $seed);

/* -------- 词典构建 -------- */
hr('1. 词典构建 build');
$txtPath = tempnam(sys_get_temp_dir(), 'c9dict') . '.txt';
$binPath = tempnam(sys_get_temp_dir(), 'c9dict') . '.bin';

[$genT, $words] = ms(fn() => gen_words($wordsN, $seed));
row('生成词典            ', fmt_ms($genT));

[$writeT] = ms(function () use ($txtPath, $words) {
    file_put_contents($txtPath, implode("\n", $words) . "\n");
});
row('写入 words.txt       ', fmt_ms($writeT));

[$buildT] = ms(fn() => chapter9_build($txtPath, $binPath));
row('chapter9_build      ', fmt_ms($buildT));
row('bin 文件大小         ', sprintf('%.2f MiB (%d 字节)', filesize($binPath) / MIB, filesize($binPath)));

/* -------- 加载 -------- */
hr('2. 加载 load');
[$loadT] = ms(fn() => chapter9_load($binPath));
row('chapter9_load       ', fmt_ms($loadT));

/* -------- 扫描类操作 -------- */
hr('3. 文本扫描（文本全部注入命中词）');
echo '生成测试文本...', PHP_EOL;
[$genTextT, $text] = ms(fn() => gen_text($words, $textMb, $seed));
row('生成测试文本        ', sprintf('%s + %.2f MiB', fmt_ms($genTextT), strlen($text) / MIB));
$tlen = strlen($text);
echo PHP_EOL;

[$detT, $hits] = best_ms(fn() => chapter9_detect($text), $runs);
row('chapter9_detect     ',
    sprintf('%s  (%s, 命中 %d 个词)', fmt_ms($detT), fmt_mbs($tlen, $detT), count($hits)));

[$repT, $out] = best_ms(fn() => chapter9_replace($text), $runs);
row('chapter9_replace    ',
    sprintf('%s  (%s)', fmt_ms($repT), fmt_mbs($tlen, $repT)));

[$hasT] = best_ms(fn() => chapter9_has($text), $runs);
row('chapter9_has        ', sprintf('%s  (%s)', fmt_ms($hasT), fmt_mbs($tlen, $hasT)));

/* -------- PCRE 基线对比 -------- */
if (!$skipRe) {
    hr('4. PCRE 基线对比（同一文本切片）');
    $slice = byte_safe_slice($text, $reMb * MIB);
    $slen  = strlen($slice);
    row('切片大小            ', sprintf('%.2f MiB', $slen / MIB));

    $reList  = array_slice($words, 0, $reWords);
    $pattern = '/' . implode('|', array_map(fn($w) => preg_quote($w, '/'), $reList)) . '/';
    row('基线词数            ', number_format(count($reList)));

    [$p1] = best_ms(fn() => preg_match_all($pattern, $slice, $m), $runs);
    row('preg_match_all      ', sprintf('%s  (%s)', fmt_ms($p1), fmt_mbs($slen, $p1)));

    [$p2] = best_ms(fn() => preg_replace($pattern, '*', $slice), $runs);
    row('preg_replace        ', sprintf('%s  (%s)', fmt_ms($p2), fmt_mbs($slen, $p2)));

    [$d1] = best_ms(fn() => chapter9_detect($slice), $runs);
    row('chapter9_detect(同切片)', sprintf('%s  (%s)', fmt_ms($d1), fmt_mbs($slen, $d1)));

    if ($p1 > 0) {
        printf("\n对比：preg_match_all 是 chapter9_detect 的 %.0f 倍耗时（词数越多差距越大）\n", $p1 / $d1);
    }
    printf("提示：PCRE 基线只用了 %s 个词、%.2f MiB 切片；\n", number_format(count($reList)), $slen / MIB);
    echo "      可调大 --re-words / --re-mb 体验差距, 或 --skip-re 跳过。\n";
}

/* -------- 汇总 -------- */
hr('5. 汇总');
row('detect 吞吐          ', fmt_mbs($tlen, $detT));
row('replace 吞吐         ', fmt_mbs($tlen, $repT));
row('峰值内存             ', sprintf('%.1f MiB', memory_get_peak_usage(true) / MIB));

@unlink($txtPath);
@unlink($binPath);
echo PHP_EOL;
echo '完成', PHP_EOL;