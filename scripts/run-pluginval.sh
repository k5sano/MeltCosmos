```bash
Copy#!/usr/bin/env bash
# Run pluginval against the built VST3 plugin.
# Usage: bash scripts/run-pluginval.sh [strictness]
#
# Prerequisites:
#   - pluginval binary on PATH, or set PLUGINVAL_BIN env var
#   - Build completed: cmake --build build/
#
# Optional: set PLUGIN_ANALYZER_BIN for Plugin Analyzer integration.

set -euo pipefail

STRICTNESS="${1:-5}"

# Locate pluginval
PLUGINVAL="${PLUGINVAL_BIN:-$(command -v pluginval 2>/dev/null || true)}"
if [[ -z "$PLUGINVAL" ]]; then
    echo "ERROR: pluginval not found."
    echo "  Install from: https://github.com/Tracktion/pluginval/releases"
    echo "  Or set PLUGINVAL_BIN=/path/to/pluginval"
    exit 1
fi

# Locate plugin
VST3_DIR="build/EMSpace_artefacts"
VST3=""

# macOS
if [[ -d "$VST3_DIR/VST3/EMSpace.vst3" ]]; then
    VST3="$VST3_DIR/VST3/EMSpace.vst3"
# Linux
elif [[ -d "$VST3_DIR/Debug/VST3/EMSpace.vst3" ]]; then
    VST3="$VST3_DIR/Debug/VST3/EMSpace.vst3"
elif [[ -d "$VST3_DIR/Release/VST3/EMSpace.vst3" ]]; then
    VST3="$VST3_DIR/Release/VST3/EMSpace.vst3"
fi

if [[ -z "$VST3" ]]; then
    echo "ERROR: EMSpace.vst3 not found in $VST3_DIR"
    echo "  Run: cmake --build build/"
    exit 1
fi

echo "=== pluginval ==="
echo "Plugin: $VST3"
echo "Strictness: $STRICTNESS"
echo ""

"$PLUGINVAL" --validate "$VST3" --strictness-level "$STRICTNESS" \
    --timeout-ms 120000 --verbose 2>&1 | tee .claude/pluginval-output.log

RESULT=$?
if [[ $RESULT -eq 0 ]]; then
    echo ""
    echo "✅ pluginval PASSED (strictness $STRICTNESS)"
else
    echo ""
    echo "❌ pluginval FAILED (exit code $RESULT)"
fi

# Optional: Plugin Analyzer
ANALYZER="${PLUGIN_ANALYZER_BIN:-}"
if [[ -n "$ANALYZER" && -x "$ANALYZER" ]]; then
    echo ""
    echo "=== Plugin Analyzer ==="
    "$ANALYZER" "$VST3" 2>&1 | tee .claude/analyzer-output.log
fi

exit $RESULT
Copy
```