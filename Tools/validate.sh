#!/bin/bash

# \============================================================

# pluginval — プラグイン正常性検証スクリプト

# \============================================================

# 事前準備:

# macOS: brew install pluginval

# Linux: GitHub Releases からダウンロード

# [https://github.com/Tracktion/pluginval/releases](https://github.com/Tracktion/pluginval/releases)

# Windows: 同上

# 使い方:

# ./Tools/validate.sh \[strictness\]

# strictness: 1-10 (デフォルト 5)

# \============================================================

set -euo pipefail

SCRIPT\_DIR="(cd"(dirname "$0")" && pwd)" PROJECT\_DIR="(cd"SCRIPT\_DIR/.." && pwd)"

STRICTNESS="${1:-5}"

# \--- プラグインパスの自動検出 ---

VST3\_PATH="" CANDIDATES=( "$PROJECT\_DIR/build/EMverbAvalanche\_artefacts/VST3/EMverb Avalanche.vst3" "$PROJECT\_DIR/build/EMverbAvalanche\_artefacts/Debug/VST3/EMverb Avalanche.vst3" "$PROJECT\_DIR/build/EMverbAvalanche\_artefacts/Release/VST3/EMverb Avalanche.vst3" )

for candidate in "${CANDIDATES\[@\]}"; do if \[ -e "$candidate" \]; then VST3\_PATH="$candidate" break fi done

if \[ -z "$VST3\_PATH" \]; then echo "ERROR: VST3 plugin not found. Build first:" echo " cmake -B build && cmake --build build/" exit 1 fi

echo "" echo " pluginval — EMverb Avalanche" echo " Plugin: $VST3\_PATH" echo " Strictness: $STRICTNESS" echo ""

# \--- pluginval の存在確認 ---

if ! command -v pluginval &> /dev/null; then echo "" echo "ERROR: pluginval not found." echo "" echo "Install:" echo " macOS: brew install pluginval" echo " Other: [https://github.com/Tracktion/pluginval/releases](https://github.com/Tracktion/pluginval/releases)" exit 1 fi

# \--- 実行 ---

echo "" echo "\[Phase 1\] Strictness level $STRICTNESS validation..." pluginval  
\--validate "$VST3\_PATH"  
\--strictness-level "$STRICTNESS"  
\--timeout-ms 120000  
\--repeat 2  
\--randomise  
\--verbose

RESULT=$?

echo "" if \[ $RESULT -eq 0 \]; then echo "PASS: All pluginval tests passed (strictness $STRICTNESS)" else echo "FAIL: pluginval reported errors (exit code $RESULT)" echo " Re-run with --verbose for details" fi

exit $RESULT