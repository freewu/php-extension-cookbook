#!/usr/bin/env bash
#
# 把项目制作为 epub / pdf 电子书（依赖 pandoc）。
#
#   1. 先由 scripts/gen-docs.sh 生成暂存的 ebook.md；
#   2. 再用 pandoc 生成 epub（自动目录，首章为封面）。
#
# 用法：
#   bash scripts/build-ebook.sh            # 生成 epub
#   bash scripts/build-ebook.sh pdf        # 生成 pdf（需要 LaTeX，较慢）
#
# 只生成 markdown：直接执行 scripts/gen-docs.sh 即可。

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

FMT="${1:-epub}"

if ! command -v pandoc >/dev/null 2>&1; then
    echo "未检测到 pandoc。请先安装：" >&2
    echo "  sudo apt-get install -y pandoc            # Debian/Ubuntu" >&2
    echo "或直接使用在线转换（https://pandoc.org/try）上传 ebook.md。" >&2
    exit 1
fi

OUT="php-extension-cookbook.${FMT}"

bash scripts/gen-docs.sh

case "$FMT" in
    epub)
        pandoc ebook.md -o "$OUT" \
            --metadata title="PHP 扩展开发手册" \
            --metadata lang=zh-CN \
            --toc --toc-depth=2
        ;;
    pdf)
        # 需要 xelatex 之类的 LaTeX 引擎，中文需指定字体
        pandoc ebook.md -o "$OUT" \
            --pdf-engine=xelatex \
            -V CJKmainfont="Noto Serif CJK SC" \
            -V geometry:margin=2cm \
            --toc --toc-depth=2
        ;;
    *)
        echo "不支持的目标格式: $FMT（可选 epub / pdf）" >&2
        exit 1
        ;;
esac

echo "生成 $OUT"
ls -lh "$OUT"