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

## 扩展练习

- 支持词库热更新：`chapter9_build` 或 `chapter9_load` 后无需重启即可生效（已实现：load 会替换旧字典）；
- 把示例里“最长命中”改成“所有命中”（输出链接 out[] 沿 fail 链多跳即可）；
- 增加 `chapter9_detect` 返回命中位置（把 `c9_hit` 的 start/end 返回给 PHP）。