#!/usr/bin/env bash
#
# 把每章的扩展源码（chapterN.c，已含中文注释）同步进对应 README，
# 方便在 GitHub / docsify / 电子书中直接阅读，无需打开源码文件。
#
# 用法：
#   bash scripts/sync-code-readme.sh
#
# 规则：
#   - 每个 README 只保留一个 "## 完整代码（chapterN.c）" 区域；
#   - 区域整体包裹在 "<!-- 本章代码 begin:chapterN -->" 与
#     "<!-- 本章代码 end:chapterN -->" 标记之间，可重复执行（幂等）；
#   - 会清理历史遗留的旧格式（标题在标记外导致的重复）。
#   - 推荐在修改过 .c 文件后运行一次，并提交 README 的同步结果。

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

python3 - "$ROOT" <<'PY'
import re
import sys
from pathlib import Path

root = Path(sys.argv[1])
done = 0

for d in sorted(root.iterdir()):
    m = re.fullmatch(r'chapter(\d+)', d.name)
    if not m:
        continue
    num    = m.group(1)
    cfile  = d / f'chapter{num}.c'
    readme = d / 'README.md'
    if not cfile.exists() or not readme.exists():
        continue

    code  = cfile.read_text(encoding='utf-8')
    lines = code.count('\n') + 1

    # 新格式：标题也放进标记区间，整块替换不会累积
    begin = f'<!-- 本章代码 begin:chapter{num} -->'
    end   = f'<!-- 本章代码 end:chapter{num} -->'
    section = (
        f'\n{begin}\n\n'
        f'## 完整代码（chapter{num}.c）\n\n'
        f'> 本章扩展的完整 C 源码，已带中文注释。'
        f'由 [`scripts/sync-code-readme.sh`](../scripts/sync-code-readme.sh) 自动同步；'
        f'以源码文件 [`chapter{num}.c`](chapter{num}.c) 为准。\n\n'
        f'<details>\n'
        f'<summary>展开 / 收起 chapter{num}.c（共 {lines} 行）</summary>\n\n'
        f'```c\n'
        f'{code}```\n\n'
        f'</details>\n\n'
        f'{end}\n'
    )

    text = readme.read_text(encoding='utf-8')

    # 清理旧区域：
    #   起点 = 第一个 "## 完整代码（" 标题（旧格式标题在标记外）；
    #   终点 = 最后一个 "本章代码 end" 标记；没有则一直删到文件尾。
    start = text.find('## 完整代码（')
    if start != -1:
        endpos = text.rfind('本章代码 end')
        if endpos != -1:
            endpos = text.find('\n', endpos)
            if endpos == -1:
                endpos = len(text)
            else:
                endpos += 1
        else:
            endpos = len(text)
        text = text[:start].rstrip() + '\n'

    text = text.rstrip() + section
    readme.write_text(text, encoding='utf-8')
    print(f'  {d.name}/README.md <-- chapter{num}.c ({lines} 行)')
    done += 1

print(f'共同步 {done} 章')
PY