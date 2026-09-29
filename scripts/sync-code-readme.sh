#!/usr/bin/env bash
#
# 把每章的扩展源码（chapterN.c，已含中文注释）同步进对应 README，
# 方便在 GitHub / docsify / 电子书中直接阅读，无需打开源码文件。
#
# 用法：
#   bash scripts/sync-code-readme.sh
#
# 规则：
#   - 每章 README 末尾维护 "<!-- 本章代码 begin --> ... end -->" 区间；
#   - 已存在则整体替换，否则追加；可重复执行（幂等）；
#   - 推荐在修改过 .c 文件后运行一次，并提交 README 的同步结果。

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

python3 - "$ROOT" <<'PY'
import re
import sys
from pathlib import Path

root = Path(sys.argv[1])
BEGIN = '<!-- 本章代码 begin -->'
END   = '<!-- 本章代码 end -->'

done = 0
for d in sorted(root.iterdir()):
    m = re.fullmatch(r'chapter(\d+)', d.name)
    if not m:
        continue
    num      = m.group(1)
    cfile    = d / f'chapter{num}.c'
    readme   = d / 'README.md'
    if not cfile.exists() or not readme.exists():
        continue

    code = cfile.read_text(encoding='utf-8')
    lines = code.count('\n') + 1

    section = (
        f'\n## 完整代码（chapter{num}.c）\n\n'
        f'> 本章扩展的完整 C 源码，已带中文注释。'
        f'由 [`scripts/sync-code-readme.sh`](../scripts/sync-code-readme.sh) 自动同步；'
        f'以源码文件 [`chapter{num}.c`](chapter{num}.c) 为准。\n\n'
        f'{BEGIN}\n\n'
        f'<details>\n'
        f'<summary>展开 / 收起 chapter{num}.c（共 {lines} 行）</summary>\n\n'
        f'```c\n'
        f'{code}```\n\n'
        f'</details>\n\n'
        f'{END}\n'
    )

    text = readme.read_text(encoding='utf-8')
    if BEGIN in text:
        # 整体替换旧区间（用 lambda 避免 re.sub 对替换串做转义）
        text = re.sub(
            re.escape(BEGIN) + r'.*?' + re.escape(END),
            lambda _m: section,
            text,
            flags=re.S,
        )
    else:
        text = text.rstrip() + section

    readme.write_text(text, encoding='utf-8')
    print(f'  {d.name}/README.md <-- chapter{num}.c ({lines} 行)')
    done += 1

print(f'共同步 {done} 章')
PY