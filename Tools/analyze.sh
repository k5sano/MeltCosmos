#!/bin/bash

# \============================================================

# Plugin Analyser — 信号特性の定量計測スクリプト

# \============================================================

# 事前準備:

# git clone [https://github.com/Conceptual-Machines/plugin-analyser.git](https://github.com/Conceptual-Machines/plugin-analyser.git)

# cd plugin-analyser

# mkdir build && cd build

# cmake .. -G Ninja -DCMAKE\_BUILD\_TYPE=Release

# cmake --build . --parallel

# 使い方:

# ./Tools/analyze.sh \[path-to-plugin-analyser-cli\]

# \============================================================

set -euo pipefail

SCRIPT\_DIR="(cd"(dirname "$0")" && pwd)" PROJECT\_DIR="(cd"SCRIPT\_DIR/.." && pwd)"

# \--- Plugin Analyser CLI パスの解決 ---

CLI\_PATH="${1:-""}"

if \[ -z "$CLI\_PATH" \]; then # 一般的な場所を探索 SEARCH\_PATHS=( "$PROJECT\_DIR/../plugin-analyser/build/plugin\_measure\_grid\_cli" "$HOME/plugin-analyser/build/plugin\_measure\_grid\_cli" "/usr/local/bin/plugin\_measure\_grid\_cli" ) for p in "${SEARCH\_PATHS\[@\]}"; do if \[ -x "$p" \]; then CLI\_PATH="$p" break fi done fi

if \[ -z "CLIP​ATH"\]∣∣\[!−x"CLI\_PATH" \]; then echo "ERROR: plugin\_measure\_grid\_cli not found." echo "" echo "Build from source:" echo " git clone [https://github.com/Conceptual-Machines/plugin-analyser.git](https://github.com/Conceptual-Machines/plugin-analyser.git)" echo " cd plugin-analyser && mkdir build && cd build" echo " cmake .. -G Ninja -DCMAKE\_BUILD\_TYPE=Release" echo " cmake --build . --parallel" echo "" echo "Then re-run:" echo " $0 /path/to/plugin\_measure\_grid\_cli" exit 1 fi

# \--- VST3 パスの自動検出 ---

VST3\_PATH="" CANDIDATES=( "$PROJECT\_DIR/build/EMverbAvalanche\_artefacts/VST3/EMverb Avalanche.vst3" "$PROJECT\_DIR/build/EMverbAvalanche\_artefacts/Debug/VST3/EMverb Avalanche.vst3" "$PROJECT\_DIR/build/EMverbAvalanche\_artefacts/Release/VST3/EMverb Avalanche.vst3" )

for candidate in "${CANDIDATES\[@\]}"; do if \[ -e "$candidate" \]; then VST3\_PATH="$candidate" break fi done

if \[ -z "$VST3\_PATH" \]; then echo "ERROR: VST3 plugin not found. Build first." exit 1 fi

# \--- 出力ディレクトリ ---

OUTPUT\_DIR="PROJECTD​IR/Tools/analysisr​esults/(date +%Y%m%d\_%H%M%S)" mkdir -p "$OUTPUT\_DIR"

echo "" echo " Plugin Analyser — EMverb Avalanche" echo " CLI: $CLI\_PATH" echo " Plugin: $VST3\_PATH" echo " Config: $SCRIPT\_DIR/emverb\_avalanche\_config.json" echo " Output: $OUTPUT\_DIR" echo ""

# \--- 実行 ---

"$CLI\_PATH"  
\--config "$SCRIPT\_DIR/emverb\_avalanche\_config.json"  
\--out "$OUTPUT\_DIR"  
\--plugin "$VST3\_PATH"

RESULT=$?

echo "" if \[ $RESULT -eq 0 \]; then echo "DONE: Results saved to $OUTPUT\_DIR" echo "" echo "Generated files:" ls -la "$OUTPUT\_DIR/" echo "" echo "Next steps:" echo " - Check grid\_rms\_peak.csv for level anomalies" echo " - Check grid\_thd.csv for distortion at high cross\_feed" echo " - Check grid\_linear\_response.csv for frequency buildup" else echo "FAIL: Plugin Analyser reported errors (exit code $RESULT)" fi

exit $RESULT