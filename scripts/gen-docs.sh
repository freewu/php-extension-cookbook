#!/usr/bin/env bash
#
# 生成 docsify 所需的导航文件：
#   _sidebar.md  侧边栏（按章节编号排序，取各章 README 的第一个 H1 作为标题）
#   ebook.md     单文件电子书（全部章节拼成一个大文档，可转 epub/pdf）
#
# 用法：
#   bash scripts/gen-docs.sh
#
# 生成后即可用任意静态服务器阅读：
#   python3 -m http.server 8000      # 打开 http://localhost:8000
#   npx docsify-cli serve .          # 或用 docsify 直接起服务

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

# ---- 收集 chapterN/README.md，按章节数字排序 ----
declare -a ENTRIES=()
for d in chapter*/; do
    n="${d#chapter}"
    n="${n%/}"
    [[ "$n" =~ ^[0-9]+$ ]] || continue
    [[ -f "${d}README.md" ]] || continue
    ENTRIES+=("$n:$d")
done
mapfile -t SORTED < <(printf '%s\n' "${ENTRIES[@]}" | sort -t: -k1,1n)

first_h1() { # $1 = file; 输出第一个 H1 去 # 后内容
    sed -n 's/^# \{1,\}//p' "$1" | head -1 | sed 's/[[:space:]]*$//'
}

# ---- _sidebar.md ----
{
    echo '* [简介](./)'
    for e in "${SORTED[@]}"; do
        dir="${e#*:}"
        title="$(first_h1 "${dir}README.md")"
        [[ -n "$title" ]] || title="${dir%/}"
        printf '* [%s](%s)\n' "$title" "$dir"
    done
} > _sidebar.md

echo "生成 _sidebar.md（$((${#SORTED[@]} + 1)) 项）"

# ---- ebook.md ----
{
    echo '# PHP 扩展开发手册（电子书版）'
    echo
    echo '> 由 `scripts/gen-docs.sh` 自动生成，可用 pandoc 转 epub/pdf：'
    echo '> `bash scripts/build-ebook.sh`'
    echo
    echo '## 目录'
    echo
    for e in "${SORTED[@]}"; do
        dir="${e#*:}"
        n="${e%%:*}"
        title="$(first_h1 "${dir}README.md")"
        [[ -n "$title" ]] || title="Chapter $n"
        printf '* [%s](#chapter-%s)\n' "$title" "$n"
    done
    echo
    for e in "${SORTED[@]}"; do
        dir="${e#*:}"
        n="${e%%:*}"
        echo "<a id=\"chapter-$n\"></a>"
        cat "${dir}README.md"
        echo
    done
} > ebook.md

echo "生成 ebook.md（$(wc -l < ebook.md) 行）"