# Chapter 9 - 敏感词检测（DFA）

## 目标

实现一个基于 **DFA（确定性有限自动机）** 的敏感词检测扩展：

- 把 `.txt` 词典（每行一个词）构建成 Trie，序列化为 `.bin` 二进制文件；
- 扩展加载 `.bin` 后，可以取出命中关键词，也可以替换处理（默认替换为 `*`）。

## 提供的函数

| 函数 | 签名 | 说明 |
| --- | --- | --- |
| `chapter9_build` | `(string $txt, string $bin): bool` | 把词典 txt 构建成 Trie 并写入 .bin |
| `chapter9_load` | `(string $bin): bool` | 加载 .bin（成功后替换当前字典） |
| `chapter9_detect` | `(string $text): array` | 返回命中的关键词（去重、按首次命中排序） |
| `chapter9_replace` | `(string $text, string $replacement = '*'): string` | 替换命中词 |
| `chapter9_has` | `(string $text): bool` | 是否命中 |
| `chapter9_count` | `(string $text): int` | 命中的词数（最左优先、不重叠） |

配置项：`chapter9.bin` —— 设置后首次调用自动加载，无需手动 `chapter9_load()`。

```php
ini_set('chapter9.bin', __DIR__ . '/words.bin');
chapter9_detect('今晚去赌场赌博');          // ['赌场', '赌博']
chapter9_replace('今晚去赌场赌博');          // '今晚去****'
chapter9_replace('今晚去赌场赌博', '好');    // '今晚去好好好好'
```

## 算法：Trie + Aho-Corasick

### 1. Trie 构建（build）

词典构建期是内存中的 Trie：每个节点是 `父节点`，边按 **Unicode 码点** 转移，
词结尾的节点记录词在字符串池中的偏移与码点长度。

```
root ─赌─> (赌) ─博─> (赌博)* ─场─> (赌场)*
```

### 2. 二进制格式（.bin，全部小端序）

```
[0]  magic     'C' '9' 'T' 'R'
[4]  version   u32 = 1
[8]  node_count u32
[12] edge_count u32
[16] pool_size  u32
[20] reserved   u32
[24] nodes     node_count × 20B
     { is_end:u32, word_index:u32, word_cp_len:u32, edge_off:u32, edge_count:u32 }
[..] edges     edge_count × 8B
     { ch:u32, target:u32 }   （每个节点的子边按 ch 升序）
[..] pool      字符串池（每个词 NUL 结尾）
```

- 子边按码点排序，查找子节点用**二分**；
- 序列化后真正的 DFA 由“节点表 + 边表”驱动，不依赖指针。

### 3. 加载 + 失败指针（Aho-Corasick）

加载 `.bin` 后，用 BFS 构建两个辅助数组：

```c
fail[v]  // 失配时回退到的状态
out[v]   // 状态 v 沿 fail 链能到达的最近词尾节点
```

扫描时失败指针让匹配不会回退文本指针，复杂度 **O(文本长度)**：

```c
for (i in text) {
    while (state && !trans(state, ch)) state = fail[state];   // 失配跳转
    state = trans(state, ch);                                  // 尽量前进
    if (out[state]) hits[] = { i - word_cp_len + 1, i + 1 };  // 命中
}
```

### 4. 命中选择

扫描得到的候选按「起点升序、同起点终点降序」排序后贪心选择：
**最左优先、互不重叠**。因此 `赌场赌博`（赌场 + 赌博）会选中两处，
`赌博` 与 `赌场` 重叠的 `赌` 不会叠加。

### 5. 替换

- 每个命中的**码点**替换为一个 `replacement`（默认 `*`），长度不变；
- `replacement=''` 相当于删除命中词；`replacement='好'` 用多字节字符替换。

## 内存与生命周期

- **词典（进程生命周期数据）**：`chapter9_load()` 用标准 C 的
  `fopen/fread/malloc` 读入并解析 `.bin`，整块分配、`free()` 释放。
  这是有意为之：词典跨请求常驻、仅在进程退出/重载时释放，
  让它在 Zend MM（`emalloc`）的临时小内存之外独立管理，
  不会影响每次请求的临时内存统计，也避免特定环境里
  `MSHUTDOWN` 阶段多块小内存归还与 Zend MM 交互的边缘问题。
- **请求内临时数据**（解码后的码点数组、命中列表、输出字符串等）
  仍用常规的 `emalloc/smart_str`，随请求结束自动回收。

热更新词典不影响常驻内存：`chapter9_load()` 会先释放旧字典再换新。

## 词典格式（words.txt）

每行一个词，UTF-8 编码，忽略空行、首尾空白与 BOM：

```text
赌博
赌场
色情
毒品
fuck
drug
```

## 编译与运行

```bash
cd chapter9
phpize && ./configure --enable-chapter9 && make
php -d extension=$(pwd)/modules/chapter9.so demo.php
make test
```

## 性能测试

```bash
php -d extension=$(pwd)/modules/chapter9.so bench.php
php -d extension=$(pwd)/modules/chapter9.so bench.php --words=200000 --text-mb=40
```

脚本会生成“大词典 + 大文本”，测量构建 / 加载 / 扫描的耗时与吞吐，
并和 PCRE 正则过滤做基线对比。常用参数：

| 参数 | 默认 | 说明 |
| --- | --- | --- |
| `--words=N` | 50000 | 词典词数 |
| `--text-mb=N` | 10 | 测试文本大小 (MiB) |
| `--runs=N` | 3 | 每操作重复次数，取最优 |
| `--re-words=N` | 1000 | PCRE 基线词数 |
| `--re-mb=N` | 1 | PCRE 基线切片 (MiB) |
| `--skip-re` | - | 跳过 PCRE 基线 |

参考结果（PHP 8.3.6，5 万词词典 / 10 MiB 文本，Virt 机器）：

```text
chapter9_build        170 ms   （bin 3.85 MiB）
chapter9_load          51 ms
chapter9_detect       344 ms   29 MiB/s（命中 39010 个不同词）
chapter9_replace      598 ms   17 MiB/s
chapter9_has          273 ms   37 MiB/s
preg_match_all  1 MiB 切片 + 1000 词基线：403 ms（约 15 倍）
```

> 说明：PCRE 基线只用了 1000 个词，词典越大正则的差距越大——
> DFA 的扫描耗时只与文本长度相关，与词典大小无关。

### 本章两处性能细节

1. **序列化合并写**：把 `.bin` 先在内存拼好、一次 `php_stream_write`，
   而不是按节点 / 按边逐个小块写入（每次都是系统调用）。
   5 万词构建从约 4 s 降到约 170 ms。
2. **detect 去重用 HashTable**：命中词非常多时，用“数组 + 线性查找”
   去重会退化成 O(n²)，换成 `zend_hash_str_add` 判重后
   detect 从约 970 ms 降到约 344 ms。

## 扩展练习

- 支持词库热更新：`chapter9_build` 或 `chapter9_load` 后无需重启即可生效（已实现：load 会替换旧字典）；
- 把示例里“最长命中”改成“所有命中”（输出链接 out[] 沿 fail 链多跳即可）；
- 增加 `chapter9_detect` 返回命中位置（把 `c9_hit` 的 start/end 返回给 PHP）。
<!-- 本章代码 begin:chapter9 -->

## 完整代码（chapter9.c）

> 本章扩展的完整 C 源码，已带中文注释。由 [`scripts/sync-code-readme.sh`](../scripts/sync-code-readme.sh) 自动同步；以源码文件 [`chapter9.c`](chapter9.c) 为准。

<details>
<summary>展开 / 收起 chapter9.c（共 1007 行）</summary>

```c
/*
 * Chapter 9 - 敏感词检测（DFA，Trie + Aho-Corasick 自动机）
 *
 * 功能：
 *   chapter9_build($txt, $bin)  把“每行一个词”的 txt 构建成 Trie，
 *                               序列化（小端序）写入 .bin 文件
 *   chapter9.load (INI)         配置项，指定自动加载的 .bin 路径
 *   chapter9_load($bin)         运行时加载 .bin 并构建 AC 失败指针
 *   chapter9_detect($text)      返回命中的关键词（去重、按首次命中排序）
 *   chapter9_replace($text, '*') 把命中词替换为 *（长度不变），返回新文本
 *   chapter9_has($text)         是否存在命中
 *   chapter9_count($text)       命中的词数（贪心、不重叠）
 *
 * 原理：
 *   1) 词典构建期为 Trie（可动态增删的构建结构 c9_builder）；
 *   2) 序列化为扁平二进制：节点表 + 边表（按码点排序，二分查找）+ 字符串池；
 *   3) 加载后在此基础上 BFS 构建失败指针 fail[] 与输出指针 out[]，
 *      形成真正的 DFA（Aho-Corasick），扫描匹配复杂度 O(文本长度)。
 *
 * 内存布局：
 *   加载后的字典（节点表/边表/字符串池/fail/out）从“一整块”emalloc
 *   内存里按偏移切分，避免多块小内存与 Zend MM 交互的边界问题；
 *   释放时只需要 efree 一次。
 *
 * 编译：phpize && ./configure --enable-chapter9 && make
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "main/php_streams.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_smart_str.h"
#include "php_chapter9.h"

ZEND_DECLARE_MODULE_GLOBALS(chapter9)

/* -------------------------------------------------------------------------
 * 1. 扁平数据结构
 *
 * 加载后的字典是“扁平”的：
 *   每个节点的子边区间 [edge_off, edge_off+edge_count)，按码点升序排列，
 *   因此查找子节点用二分；整个字典放在一块内存中（见 c9_trie.raw）。
 * ---------------------------------------------------------------------- */
typedef struct _c9_edge {
    uint32_t ch;      /* Unicode 码点 */
    uint32_t target;  /* 子节点下标 */
} c9_edge;

typedef struct _c9_node {
    uint32_t is_end;      /* 是否为一个词的结尾 */
    uint32_t word_index;  /* 是结尾时：词在 pool 中的偏移 */
    uint32_t word_cp_len; /* 是结尾时：词的码点个数（用于计算命中起点） */
    uint32_t edge_off;    /* 子边区间起点（edges[] 下标） */
    uint32_t edge_count;  /* 子边个数 */
} c9_node;

typedef struct _c9_trie {
    uint32_t node_count;
    uint32_t edge_count;
    uint32_t pool_size;

    size_t   off_nodes;   /* raw 内的各区域偏移 */
    size_t   off_edges;
    size_t   off_pool;
    size_t   off_fail;
    size_t   off_out;
    char    *raw;         /* 一整块内存 */
} c9_trie;

#define C9_TRIE_NODES(t) ((c9_node *)((t)->raw + (t)->off_nodes))
#define C9_TRIE_EDGES(t) ((c9_edge *)((t)->raw + (t)->off_edges))
#define C9_TRIE_POOL(t)  ((t)->raw + (t)->off_pool)
#define C9_TRIE_FAIL(t)  ((uint32_t *)((t)->raw + (t)->off_fail))
#define C9_TRIE_OUT(t)   ((uint32_t *)((t)->raw + (t)->off_out))

/* 构建期结构（动态数组，可扩展；用完即弃） */
typedef struct _c9_bnode {
    zend_bool  is_end;
    uint32_t   word_index;
    uint32_t   word_cp_len;
    c9_edge   *edges;
    uint32_t   edge_count;
    uint32_t   edge_cap;
    uint32_t   edge_off;
} c9_bnode;

typedef struct _c9_builder {
    c9_bnode  *nodes;
    uint32_t   node_count;
    uint32_t   node_cap;
    char      *pool;
    uint32_t   pool_len;
    uint32_t   pool_cap;
} c9_builder;

/* -------------------------------------------------------------------------
 * 2. 二进制格式（全部小端序）
 *
 *   [0]  magic: 'C' '9' 'T' 'R'(4B)
 *   [4]  version: u32 = 1
 *   [8]  node_count: u32
 *   [12] edge_count: u32
 *   [16] pool_size:  u32
 *   [20] reserved:   u32
 *   [24] nodes:  node_count * 20B
 *        { is_end:u32, word_index:u32, word_cp_len:u32, edge_off:u32, edge_count:u32 }
 *   [..] edges:  edge_count * 8B
 *        { ch:u32, target:u32 }
 *   [..] pool:   字符串池（每个词 NUL 结尾）
 * ---------------------------------------------------------------------- */
#define C9_MAGIC0 'C'
#define C9_MAGIC1 '9'
#define C9_MAGIC2 'T'
#define C9_MAGIC3 'R'
#define C9_VERSION 1

static void c9_w32(unsigned char *p, uint32_t v)
{
    p[0] = (unsigned char)(v & 0xff);
    p[1] = (unsigned char)((v >> 8)  & 0xff);
    p[2] = (unsigned char)((v >> 16) & 0xff);
    p[3] = (unsigned char)((v >> 24) & 0xff);
}

static uint32_t c9_r32(const unsigned char *p)
{
    return ((uint32_t) p[0])
         | ((uint32_t) p[1] << 8)
         | ((uint32_t) p[2] << 16)
         | ((uint32_t) p[3] << 24);
}

/* -------------------------------------------------------------------------
 * 3. UTF-8 解码
 * ---------------------------------------------------------------------- */
static void c9_utf8_decode(const char *str, size_t len,
                           uint32_t **out_cps, uint32_t **out_offs, size_t *out_n)
{
    size_t cap = len ? len : 1;
    uint32_t *cps  = emalloc(sizeof(uint32_t) * cap);
    uint32_t *offs = out_offs ? emalloc(sizeof(uint32_t) * (cap + 1)) : NULL;
    size_t    n    = 0;

    const unsigned char *p   = (const unsigned char *) str;
    const unsigned char *end = p + len;

    if (offs) offs[0] = 0;

    while (p < end) {
        unsigned char b = *p;
        uint32_t cp;
        size_t adv = 1;

        if (b < 0x80) {
            cp = b;
        } else if ((b & 0xE0) == 0xC0 && end - p >= 2 && (p[1] & 0xC0) == 0x80) {
            cp  = ((uint32_t)(b & 0x1F) << 6) | (p[1] & 0x3F);
            adv = 2;
        } else if ((b & 0xF0) == 0xE0 && end - p >= 3
                   && (p[1] & 0xC0) == 0x80 && (p[2] & 0xC0) == 0x80) {
            uint32_t c = ((uint32_t)(b & 0x0F) << 12)
                       | ((uint32_t)(p[1] & 0x3F) << 6)
                       | (p[2] & 0x3F);
            if (c >= 0xD800 && c <= 0xDFFF) { cp = b; adv = 1; }
            else { cp = c; adv = 3; }
        } else if ((b & 0xF8) == 0xF0 && end - p >= 4
                   && (p[1] & 0xC0) == 0x80 && (p[2] & 0xC0) == 0x80
                   && (p[3] & 0xC0) == 0x80) {
            uint32_t c = ((uint32_t)(b & 0x07) << 18)
                       | ((uint32_t)(p[1] & 0x3F) << 12)
                       | ((uint32_t)(p[2] & 0x3F) << 6)
                       | (p[3] & 0x3F);
            if (c < 0x10000 || c > 0x10FFFF) { cp = b; adv = 1; }
            else { cp = c; adv = 4; }
        } else {
            cp = b;
        }

        if (n == cap) {
            cap *= 2;
            cps  = erealloc(cps,  sizeof(uint32_t) * cap);
            if (offs) offs = erealloc(offs, sizeof(uint32_t) * (cap + 1));
        }
        cps[n] = cp;
        if (offs) offs[n + 1] = offs[n] + adv;
        n++;
        p += adv;
    }

    *out_cps = cps;
    *out_n   = n;
    if (out_offs) *out_offs = offs;
}

/* -------------------------------------------------------------------------
 * 4. Trie 构建（c9_builder）
 * ---------------------------------------------------------------------- */
static void c9_builder_init(c9_builder *b)
{
    memset(b, 0, sizeof(*b));
    b->nodes = emalloc(sizeof(c9_bnode) * 16);
    b->node_cap = 16;
    b->node_count = 1;   /* 根节点 = 0 */
    memset(&b->nodes[0], 0, sizeof(c9_bnode));
}

static void c9_builder_free(c9_builder *b)
{
    for (uint32_t i = 0; i < b->node_count; i++) {
        if (b->nodes[i].edges) efree(b->nodes[i].edges);
    }
    efree(b->nodes);
    if (b->pool) efree(b->pool);
    memset(b, 0, sizeof(*b));
}

static uint32_t c9_builder_add_node(c9_builder *b)
{
    if (b->node_count == b->node_cap) {
        uint32_t ncap = b->node_cap * 2;
        b->nodes = erealloc(b->nodes, sizeof(c9_bnode) * ncap);
        b->node_cap = ncap;
    }
    memset(&b->nodes[b->node_count], 0, sizeof(c9_bnode));
    return b->node_count++;
}

static c9_edge *c9_builder_find_edge(c9_builder *b, uint32_t node, uint32_t ch)
{
    c9_bnode *n = &b->nodes[node];
    for (uint32_t i = 0; i < n->edge_count; i++) {
        if (n->edges[i].ch == ch) return &n->edges[i];
    }
    return NULL;
}

static uint32_t c9_builder_add_edge(c9_builder *b, uint32_t from, uint32_t ch)
{
    c9_bnode *n = &b->nodes[from];
    if (n->edge_count == n->edge_cap) {
        uint32_t ncap = n->edge_cap ? n->edge_cap * 2 : 4;
        n->edges = erealloc(n->edges, sizeof(c9_edge) * ncap);
        n->edge_cap = ncap;
    }
    n->edges[n->edge_count].ch = ch;
    uint32_t child = c9_builder_add_node(b);
    n = &b->nodes[from];
    n->edges[n->edge_count].target = child;
    return n->edges[n->edge_count++].target;
}

static uint32_t c9_pool_append(c9_builder *b, const char *s, size_t len)
{
    if (b->pool_len + len + 1 > b->pool_cap) {
        size_t ncap = b->pool_cap ? b->pool_cap : 64;
        while (ncap < b->pool_len + len + 1) ncap *= 2;
        b->pool = erealloc(b->pool, ncap);
        b->pool_cap = ncap;
    }
    uint32_t off = b->pool_len;
    memcpy(b->pool + off, s, len);
    b->pool[off + len] = '\0';
    b->pool_len += len + 1;
    return off;
}

static void c9_builder_insert(c9_builder *b, const char *word, size_t len)
{
    uint32_t *cps;
    size_t    n;

    c9_utf8_decode(word, len, &cps, NULL, &n);
    if (n == 0) {
        return;
    }

    uint32_t node = 0;
    for (size_t i = 0; i < n; i++) {
        c9_edge *e = c9_builder_find_edge(b, node, cps[i]);
        if (e) {
            node = e->target;
        } else {
            node = c9_builder_add_edge(b, node, cps[i]);
        }
    }
    if (!b->nodes[node].is_end) {
        b->nodes[node].is_end      = 1;
        b->nodes[node].word_cp_len = n;
        b->nodes[node].word_index  = c9_pool_append(b, word, len);
    }
    efree(cps);
}

static int c9_edge_cmp(const void *pa, const void *pb)
{
    const c9_edge *a = pa, *b = pb;
    return (a->ch < b->ch) ? -1 : ((a->ch > b->ch) ? 1 : 0);
}

/* -------------------------------------------------------------------------
 * 5. 序列化（Trie -> .bin）
 * ---------------------------------------------------------------------- */
static zend_bool c9_serialize(c9_builder *b, php_stream *out)
{
    uint32_t nc = b->node_count;
    uint32_t ec = 0;
    for (uint32_t i = 0; i < nc; i++) ec += b->nodes[i].edge_count;

    /* 展平边表：每个节点的子边按 ch 排序后连续存放 */
    c9_edge *flat = emalloc(sizeof(c9_edge) * (ec ? ec : 1));
    uint32_t off = 0;
    for (uint32_t i = 0; i < nc; i++) {
        c9_bnode *n = &b->nodes[i];
        if (n->edge_count > 1) {
            qsort(n->edges, n->edge_count, sizeof(c9_edge), c9_edge_cmp);
        }
        n->edge_off = off;
        if (n->edge_count) {
            memcpy(flat + off, n->edges, sizeof(c9_edge) * n->edge_count);
        }
        off += n->edge_count;
    }

    /* 在内存里拼好整个 .bin，再一次性写入，
     * 避免按节点 / 按边逐个小块 php_stream_write（每次都是系统调用）。 */
    size_t total = 24 + (size_t)nc * 20 + (size_t)ec * 8 + b->pool_len;
    unsigned char *buf = emalloc(total ? total : 1);
    unsigned char *p   = buf;

    p[0] = C9_MAGIC0; p[1] = C9_MAGIC1; p[2] = C9_MAGIC2; p[3] = C9_MAGIC3;
    c9_w32(p + 4,  C9_VERSION);
    c9_w32(p + 8,  nc);
    c9_w32(p + 12, ec);
    c9_w32(p + 16, b->pool_len);
    c9_w32(p + 20, 0);
    p += 24;

    for (uint32_t i = 0; i < nc; i++) {
        c9_bnode *n = &b->nodes[i];
        c9_w32(p,      n->is_end);
        c9_w32(p + 4,  n->word_index);
        c9_w32(p + 8,  n->word_cp_len);
        c9_w32(p + 12, n->edge_off);
        c9_w32(p + 16, n->edge_count);
        p += 20;
    }

    for (uint32_t i = 0; i < ec; i++) {
        c9_w32(p,     flat[i].ch);
        c9_w32(p + 4, flat[i].target);
        p += 8;
    }

    if (b->pool_len) {
        memcpy(p, b->pool, b->pool_len);
        p += b->pool_len;
    }

    zend_bool ok = (php_stream_write(out, buf, total) == (ssize_t) total)
                 ? SUCCESS : FAILURE;

    efree(flat);
    efree(buf);
    return ok;
}

/* -------------------------------------------------------------------------
 * 6. 加载 + 构建 AC 自动机
 * ---------------------------------------------------------------------- */

/* 二分查找子边；找不到返回 UINT32_MAX */
static uint32_t c9_get_trans(const c9_trie *t, uint32_t node, uint32_t ch)
{
    uint32_t lo = C9_TRIE_NODES(t)[node].edge_off;
    uint32_t hi = lo + C9_TRIE_NODES(t)[node].edge_count;
    while (lo < hi) {
        uint32_t mid = lo + (hi - lo) / 2;
        uint32_t c = C9_TRIE_EDGES(t)[mid].ch;
        if (c == ch) return C9_TRIE_EDGES(t)[mid].target;
        if (c < ch) lo = mid + 1;
        else hi = mid;
    }
    return UINT32_MAX;
}

static void c9_trie_free(c9_trie *t)
{
    if (!t) return;
    if (t->raw) free(t->raw);
    free(t);
}

/* BFS 构建失败指针 fail[] 与输出指针 out[] */
static void c9_build_ac(c9_trie *t)
{
    c9_node  *nodes = C9_TRIE_NODES(t);
    c9_edge  *edges = C9_TRIE_EDGES(t);
    uint32_t *fail  = C9_TRIE_FAIL(t);
    uint32_t *out   = C9_TRIE_OUT(t);
    uint32_t  nc    = t->node_count;

    /* 词典数据进程生命周期：队列也走 malloc，与词典一致 */
    uint32_t *queue = malloc(sizeof(uint32_t) * nc);
    uint32_t head = 0, tail = 0;

    fail[0] = 0;
    out[0]  = 0;

    /* 根的直接子节点 fail = 0 */
    for (uint32_t e = nodes[0].edge_off;
         e < nodes[0].edge_off + nodes[0].edge_count; e++) {
        uint32_t v = edges[e].target;
        fail[v] = 0;
        out[v]  = nodes[v].is_end ? v : 0;
        queue[tail++] = v;
    }

    while (head < tail) {
        uint32_t u = queue[head++];
        for (uint32_t e = nodes[u].edge_off;
             e < nodes[u].edge_off + nodes[u].edge_count; e++) {
            uint32_t ch = edges[e].ch;
            uint32_t v  = edges[e].target;

            /* f = fail[u] 沿链上溯，找第一个能转移 ch 的状态 */
            uint32_t f = fail[u];
            while (f != 0 && c9_get_trans(t, f, ch) == UINT32_MAX) {
                f = fail[f];
            }
            fail[v] = (f != 0) ? c9_get_trans(t, f, ch) : 0;

            /* out[v]：自己或 fail 链上最近的词尾节点 */
            out[v] = nodes[v].is_end ? v : out[fail[v]];

            queue[tail++] = v;
        }
    }
    free(queue);
}

/* 校验并加载 .bin，成功后把 trie 写入 *out */
static zend_bool c9_load_trie(c9_trie **out, const char *path)
{
    /* 词典是进程生命周期数据：加载用标准 C 的 stdio + malloc，
     * 不经过 Zend MM，释放时用 free()，避免与大块小内存分配
     * 以及 Zend MM 在进程收尾阶段的交互出现意外。 */
    FILE *fp = fopen(path, "rb");
    if (!fp) return FAILURE;

    fseek(fp, 0, SEEK_END);
    long fsz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (fsz < 24) {
        fclose(fp);
        return FAILURE;
    }

    unsigned char *buf = malloc((size_t) fsz + 1);
    if (!buf) {
        fclose(fp);
        return FAILURE;
    }
    size_t got = fread(buf, 1, (size_t) fsz, fp);
    fclose(fp);
    if (got != (size_t) fsz) {
        free(buf);
        return FAILURE;
    }

    zend_bool ok = FAILURE;
    const unsigned char *p = buf;
    if (p[0] == C9_MAGIC0 && p[1] == C9_MAGIC1
     && p[2] == C9_MAGIC2 && p[3] == C9_MAGIC3
     && c9_r32(p + 4) == C9_VERSION) {
        uint32_t nc = c9_r32(p + 8);
        uint32_t ec = c9_r32(p + 12);
        uint32_t ps = c9_r32(p + 16);
        uint64_t need = 24ULL + 20ULL * nc + 8ULL * ec + ps;
        if (nc != 0 && need == (uint64_t) fsz) {
            c9_trie *t = calloc(1, sizeof(c9_trie));
            t->node_count = nc;
            t->edge_count = ec;
            t->pool_size  = ps;

            /* 一整块内存：nodes | edges | pool | fail | out */
            size_t nb = (size_t)nc * sizeof(c9_node);
            size_t eb = (size_t)ec * sizeof(c9_edge);
            t->off_nodes = 0;
            t->off_edges = nb;
            t->off_pool  = nb + eb;
            t->off_fail  = nb + eb + ps;
            t->off_out   = nb + eb + ps + (size_t)nc * sizeof(uint32_t);
            t->raw = malloc(nb + eb + ps + 2 * (size_t)nc * sizeof(uint32_t) + 32);
            if (t->raw) {
                const unsigned char *np = p + 24;
                c9_node *nn = C9_TRIE_NODES(t);
                for (uint32_t i = 0; i < nc; i++) {
                    nn[i].is_end      = c9_r32(np);
                    nn[i].word_index  = c9_r32(np + 4);
                    nn[i].word_cp_len = c9_r32(np + 8);
                    nn[i].edge_off    = c9_r32(np + 12);
                    nn[i].edge_count  = c9_r32(np + 16);
                    np += 20;
                }
                const unsigned char *ep = np;
                c9_edge *ee = C9_TRIE_EDGES(t);
                for (uint32_t i = 0; i < ec; i++) {
                    ee[i].ch     = c9_r32(ep);
                    ee[i].target = c9_r32(ep + 4);
                    ep += 8;
                }
                memcpy(C9_TRIE_POOL(t), ep, ps);

                c9_build_ac(t);
                *out = t;
                ok = SUCCESS;
            } else {
                free(t);
            }
        }
    }
    free(buf);
    return ok;
}

/* -------------------------------------------------------------------------
 * 7. 匹配
 * ---------------------------------------------------------------------- */
typedef struct _c9_hit {
    uint32_t start;
    uint32_t end;
    uint32_t word_off;
} c9_hit;

/* Aho-Corasick 扫描：找出所有“以某位置结尾”的最长命中 */
static c9_hit *c9_scan(const c9_trie *t, const uint32_t *cps, size_t n, size_t *out_cnt)
{
    size_t cap = 16, cnt = 0;
    c9_hit *hits = emalloc(sizeof(c9_hit) * cap);

    uint32_t state = 0;
    for (size_t i = 0; i < n; i++) {
        uint32_t ch = cps[i];

        while (state != 0 && c9_get_trans(t, state, ch) == UINT32_MAX) {
            state = C9_TRIE_FAIL(t)[state];
        }
        uint32_t next = c9_get_trans(t, state, ch);
        if (next != UINT32_MAX) {
            state = next;
        } else {
            state = 0;
        }

        if (state != 0 && C9_TRIE_OUT(t)[state] != 0) {
            uint32_t node = C9_TRIE_OUT(t)[state];
            uint32_t wlen = C9_TRIE_NODES(t)[node].word_cp_len;
            if (wlen > 0 && wlen <= i + 1) {
                if (cnt == cap) {
                    cap *= 2;
                    hits = erealloc(hits, sizeof(c9_hit) * cap);
                }
                hits[cnt].start    = (uint32_t) (i + 1 - wlen);
                hits[cnt].end      = (uint32_t) (i + 1);
                hits[cnt].word_off = C9_TRIE_NODES(t)[node].word_index;
                cnt++;
            }
        }
    }

    *out_cnt = cnt;
    return hits;
}

/* 排序：起点升序，同起点取最长（终点降序） */
static int c9_hit_cmp(const void *pa, const void *pb)
{
    const c9_hit *a = pa, *b = pb;
    if (a->start != b->start) return (a->start < b->start) ? -1 : 1;
    if (a->end   != b->end)   return (a->end   > b->end)   ? -1 : 1;
    return 0;
}

/* 贪心选择：最左优先、不重叠、同起点取最长；就地压缩 */
static size_t c9_select(c9_hit *hits, size_t cnt)
{
    qsort(hits, cnt, sizeof(c9_hit), c9_hit_cmp);
    size_t sel = 0;
    uint32_t last_end = 0;
    for (size_t i = 0; i < cnt; i++) {
        if (hits[i].start >= last_end) {
            hits[sel++] = hits[i];
            last_end = hits[i].end;
        }
    }
    return sel;
}

/* -------------------------------------------------------------------------
 * 8. 函数声明 / arginfo / 函数表
 * ---------------------------------------------------------------------- */
PHP_FUNCTION(chapter9_build);
PHP_FUNCTION(chapter9_load);
PHP_FUNCTION(chapter9_detect);
PHP_FUNCTION(chapter9_replace);
PHP_FUNCTION(chapter9_has);
PHP_FUNCTION(chapter9_count);

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chapter9_build, 0, 2, _IS_BOOL, 0)
    ZEND_ARG_TYPE_INFO(0, txt, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, bin, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chapter9_load, 0, 1, _IS_BOOL, 0)
    ZEND_ARG_TYPE_INFO(0, bin, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chapter9_detect, 0, 1, IS_ARRAY, 0)
    ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chapter9_replace, 0, 1, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, replacement, IS_STRING, 0, "\"*\"")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chapter9_has, 0, 1, _IS_BOOL, 0)
    ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chapter9_count, 0, 1, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry chapter9_functions[] = {
    PHP_FE(chapter9_build,   arginfo_chapter9_build)
    PHP_FE(chapter9_load,    arginfo_chapter9_load)
    PHP_FE(chapter9_detect,  arginfo_chapter9_detect)
    PHP_FE(chapter9_replace, arginfo_chapter9_replace)
    PHP_FE(chapter9_has,     arginfo_chapter9_has)
    PHP_FE(chapter9_count,   arginfo_chapter9_count)
    PHP_FE_END
};

/* -------------------------------------------------------------------------
 * 9. 配置项与全局变量
 * ---------------------------------------------------------------------- */
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("chapter9.bin", "", PHP_INI_ALL,
                      OnUpdateString, bin_path, zend_chapter9_globals, chapter9_globals)
PHP_INI_END()

static PHP_GINIT_FUNCTION(chapter9)
{
#if defined(COMPILE_DL_CHAPTER9) && defined(ZTS)
    ZEND_TSRMLS_CACHE_UPDATE();
#endif
    memset(chapter9_globals, 0, sizeof(*chapter9_globals));
}

/* 惰性加载：首次使用时按 chapter9.bin 自动加载 */
static c9_trie *c9_ensure_load(void)
{
    if (CHAPTER9_G(trie) != NULL) {
        return (c9_trie *) CHAPTER9_G(trie);
    }
    if (CHAPTER9_G(bin_path) != NULL && *CHAPTER9_G(bin_path)) {
        c9_trie *t = NULL;
        if (c9_load_trie(&t, CHAPTER9_G(bin_path)) == SUCCESS) {
            CHAPTER9_G(trie) = t;
            return t;
        }
    }
    return NULL;
}

static void c9_throw_not_loaded(void)
{
    zend_throw_exception(zend_exception_get_default(),
        "chapter9: dictionary not loaded; set ini chapter9.bin, "
        "call chapter9_load(), or build one with chapter9_build()", 0);
}

PHP_MINIT_FUNCTION(chapter9)
{
    REGISTER_INI_ENTRIES();
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(chapter9)
{
    if (CHAPTER9_G(trie) != NULL) {
        c9_trie_free((c9_trie *) CHAPTER9_G(trie));
        CHAPTER9_G(trie) = NULL;
    }
    UNREGISTER_INI_ENTRIES();
    return SUCCESS;
}

/* -------------------------------------------------------------------------
 * 10. 函数实现
 * ---------------------------------------------------------------------- */

/* bool chapter9_build(string $txt, string $bin) */
PHP_FUNCTION(chapter9_build)
{
    zend_string *txt, *bin;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STR(txt)
        Z_PARAM_STR(bin)
    ZEND_PARSE_PARAMETERS_END();

    php_stream *s = php_stream_open_wrapper(ZSTR_VAL(txt), "rb", 0, NULL);
    if (!s) {
        char *msg;
        spprintf(&msg, 0, "chapter9: cannot open dictionary file: %s", ZSTR_VAL(txt));
        zend_throw_exception(zend_exception_get_default(), msg, 0);
        efree(msg);
        RETURN_THROWS();
    }
    zend_string *content = php_stream_copy_to_mem(s, PHP_STREAM_COPY_ALL, 0);
    php_stream_close(s);
    if (!content) {
        zend_throw_exception(zend_exception_get_default(),
            "chapter9: cannot read dictionary file", 0);
        RETURN_THROWS();
    }

    c9_builder b;
    c9_builder_init(&b);

    const char *data = ZSTR_VAL(content);
    size_t left = ZSTR_LEN(content);

    /* 跳过 UTF-8 BOM */
    if (left >= 3 && memcmp(data, "\xEF\xBB\xBF", 3) == 0) {
        data += 3;
        left -= 3;
    }

    /* 逐行：每行一个词，忽略空白行与首尾空白 */
    while (left > 0) {
        const char *nl = memchr(data, '\n', left);
        size_t llen = nl ? (size_t) (nl - data) : left;

        size_t st = 0, en = llen;
        while (st < en && (data[st] == ' ' || data[st] == '\t' || data[st] == '\r')) st++;
        while (en > st && (data[en-1] == ' ' || data[en-1] == '\t' || data[en-1] == '\r')) en--;

        if (en > st) {
            c9_builder_insert(&b, data + st, en - st);
        }

        if (!nl) break;
        data += llen + 1;
        left -= llen + 1;
    }

    zend_string_release(content);

    php_stream *out = php_stream_open_wrapper(ZSTR_VAL(bin), "wb", 0, NULL);
    if (!out) {
        char *msg;
        spprintf(&msg, 0, "chapter9: cannot open output file: %s", ZSTR_VAL(bin));
        zend_throw_exception(zend_exception_get_default(), msg, 0);
        efree(msg);
        c9_builder_free(&b);
        RETURN_THROWS();
    }

    zend_bool ok = c9_serialize(&b, out);
    php_stream_close(out);
    c9_builder_free(&b);

    if (ok != SUCCESS) {
        zend_throw_exception(zend_exception_get_default(),
            "chapter9: failed to write dictionary file", 0);
        RETURN_THROWS();
    }

    RETURN_TRUE;
}

/* bool chapter9_load(string $bin) */
PHP_FUNCTION(chapter9_load)
{
    zend_string *bin;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(bin)
    ZEND_PARSE_PARAMETERS_END();

    c9_trie *t = NULL;
    if (c9_load_trie(&t, ZSTR_VAL(bin)) != SUCCESS) {
        zend_throw_exception(zend_exception_get_default(),
            "chapter9: invalid dictionary file", 0);
        RETURN_THROWS();
    }

    if (CHAPTER9_G(trie) != NULL) {
        c9_trie_free((c9_trie *) CHAPTER9_G(trie));
    }
    CHAPTER9_G(trie) = t;

    RETURN_TRUE;
}

/* array chapter9_detect(string $text) */
PHP_FUNCTION(chapter9_detect)
{
    zend_string *text;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(text)
    ZEND_PARSE_PARAMETERS_END();

    c9_trie *t = c9_ensure_load();
    if (!t) {
        c9_throw_not_loaded();
        RETURN_THROWS();
    }

    uint32_t *cps;
    size_t n;
    c9_utf8_decode(ZSTR_VAL(text), ZSTR_LEN(text), &cps, NULL, &n);

    size_t hcnt;
    c9_hit *hits = c9_scan(t, cps, n, &hcnt);
    size_t scnt = c9_select(hits, hcnt);
    efree(cps);

    array_init(return_value);
    if (scnt == 0) {
        efree(hits);
        return;
    }

    /* 去重，保持首次命中顺序。
     * 命中词可能非常多（大文本 + 大词典），用线性表会退化成 O(n^2)，
     * 这里用 HashTable 做 O(1) 判重。 */
    HashTable seen;
    zend_hash_init(&seen, scnt, NULL, NULL, 0);
    zval marker;
    ZVAL_TRUE(&marker);
    for (size_t i = 0; i < scnt; i++) {
        const char *w = C9_TRIE_POOL(t) + hits[i].word_off;
        size_t wlen = strlen(w);
        /* zend_hash_str_add 的 pData 不能为 NULL（内部会解引用），
         * 传入一个真实 zval 即可；键已存在时返回 NULL，达到判重效果 */
        if (zend_hash_str_add(&seen, w, wlen, &marker) != NULL) {
            add_next_index_stringl(return_value, w, wlen);
        }
    }
    zend_hash_destroy(&seen);
    efree(hits);
}

/* string chapter9_replace(string $text, string $replacement = '*') */
PHP_FUNCTION(chapter9_replace)
{
    zend_string *text;
    zend_string *repl = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_STR(text)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR(repl)
    ZEND_PARSE_PARAMETERS_END();

    const char *rep_str = repl ? ZSTR_VAL(repl) : "*";
    size_t      rep_len = repl ? ZSTR_LEN(repl) : 1;

    c9_trie *t = c9_ensure_load();
    if (!t) {
        c9_throw_not_loaded();
        RETURN_THROWS();
    }

    uint32_t *cps, *offs;
    size_t n;
    c9_utf8_decode(ZSTR_VAL(text), ZSTR_LEN(text), &cps, &offs, &n);

    size_t hcnt;
    c9_hit *hits = c9_scan(t, cps, n, &hcnt);
    size_t scnt  = c9_select(hits, hcnt);

    /* 拼接输出：命中的码点换成 replacement（每个码点一个 replacement） */
    smart_str buf = {0};
    size_t s = 0;
    for (size_t i = 0; i < n; i++) {
        if (s < scnt && i >= hits[s].end) s++;
        if (s < scnt && i >= hits[s].start && i < hits[s].end) {
            smart_str_appendl(&buf, rep_str, rep_len);
        } else {
            smart_str_appendl(&buf, ZSTR_VAL(text) + offs[i], offs[i + 1] - offs[i]);
        }
    }
    smart_str_0(&buf);

    efree(cps);
    efree(offs);
    efree(hits);

    if (buf.s) {
        RETURN_STR(buf.s);
    }
    RETURN_EMPTY_STRING();
}

/* bool chapter9_has(string $text) */
PHP_FUNCTION(chapter9_has)
{
    zend_string *text;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(text)
    ZEND_PARSE_PARAMETERS_END();

    c9_trie *t = c9_ensure_load();
    if (!t) {
        c9_throw_not_loaded();
        RETURN_THROWS();
    }

    uint32_t *cps;
    size_t n;
    c9_utf8_decode(ZSTR_VAL(text), ZSTR_LEN(text), &cps, NULL, &n);

    size_t hcnt;
    c9_hit *hits = c9_scan(t, cps, n, &hcnt);
    size_t scnt  = c9_select(hits, hcnt);

    efree(cps);
    efree(hits);

    RETURN_BOOL(scnt > 0);
}

/* int chapter9_count(string $text) */
PHP_FUNCTION(chapter9_count)
{
    zend_string *text;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(text)
    ZEND_PARSE_PARAMETERS_END();

    c9_trie *t = c9_ensure_load();
    if (!t) {
        c9_throw_not_loaded();
        RETURN_THROWS();
    }

    uint32_t *cps;
    size_t n;
    c9_utf8_decode(ZSTR_VAL(text), ZSTR_LEN(text), &cps, NULL, &n);

    size_t hcnt;
    c9_hit *hits = c9_scan(t, cps, n, &hcnt);
    size_t scnt  = c9_select(hits, hcnt);

    efree(cps);
    efree(hits);

    RETURN_LONG((zend_long) scnt);
}

/* -------------------------------------------------------------------------
 * 11. MINFO / 模块入口
 * ---------------------------------------------------------------------- */
PHP_MINFO_FUNCTION(chapter9)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "chapter9 support", "enabled");
    php_info_print_table_row(2, "Version", PHP_CHAPTER9_VERSION);
    php_info_print_table_row(2, "Algorithm", "Trie + Aho-Corasick (DFA)");
    php_info_print_table_end();

    DISPLAY_INI_ENTRIES();
}

zend_module_entry chapter9_module_entry = {
    STANDARD_MODULE_HEADER,
    "chapter9",
    chapter9_functions,
    PHP_MINIT(chapter9),
    PHP_MSHUTDOWN(chapter9),
    NULL,
    NULL,
    PHP_MINFO(chapter9),
    PHP_CHAPTER9_VERSION,
    PHP_MODULE_GLOBALS(chapter9),
    PHP_GINIT(chapter9),
    NULL,
    NULL,
    STANDARD_MODULE_PROPERTIES_EX
};

#ifdef COMPILE_DL_CHAPTER9
ZEND_GET_MODULE(chapter9)
#endif```

</details>

<!-- 本章代码 end:chapter9 -->
