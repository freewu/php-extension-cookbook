#!/usr/bin/env bash
#
# 一键编译并测试全部章节。
#
#   ./build.sh              # 编译并运行每个章节的 phpt 测试
#   ./build.sh build        # 只编译，不跑测试
#   ./build.sh clean        # 清理每个章节的编译产物
#   ./build.sh chapter3     # 只处理指定章节
#
# 依赖：php-cli、php-dev（phpize/php-config）、gcc、make、autoconf

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
JOBS="${JOBS:-$(nproc 2>/dev/null || echo 2)}"
ALL_CHAPTERS=(chapter1 chapter2 chapter3 chapter4 chapter5 chapter6 chapter7 chapter8)

ACTION="all"
TARGETS=()

for arg in "$@"; do
    case "$arg" in
        build|test|all|clean) ACTION="$arg" ;;
        chapter[1-8])         TARGETS+=("$arg") ;;
        *) echo "未知参数: $arg" >&2; exit 1 ;;
    esac
done

if [[ ${#TARGETS[@]} -eq 0 ]]; then
    TARGETS=("${ALL_CHAPTERS[@]}")
fi

need() {
    command -v "$1" >/dev/null 2>&1 || {
        echo "缺少命令: $1（请先安装 PHP 开发环境）" >&2
        exit 1
    }
}

if [[ "$ACTION" != "clean" ]]; then
    need php
    need phpize
    need php-config
    need make
fi

for ch in "${TARGETS[@]}"; do
    dir="$ROOT/$ch"
    [[ -d "$dir" ]] || { echo "跳过不存在的目录: $ch" >&2; continue; }

    echo "============================================================"
    echo ">>> $ch"
    echo "============================================================"

    cd "$dir"

    if [[ "$ACTION" == "clean" ]]; then
        if [[ -f Makefile ]]; then
            make clean >/dev/null 2>&1 || true
        fi
        rm -rf .libs modules build include autom4te.cache \
               Makefile Makefile.fragments Makefile.objects \
               configure configure.ac config.h config.h.in \
               config.log config.status config.nice libtool run-tests.php
        echo "已清理 $ch"
        continue
    fi

    phpize >/dev/null
    ./configure --enable-"$ch" >/dev/null
    make -j"$JOBS" >/dev/null

    echo "编译完成: $ch/modules/$ch.so"

    if [[ "$ACTION" != "build" ]]; then
        NO_INTERACTION=1 REPORT_EXIT_STATUS=1 make test
    fi
done
