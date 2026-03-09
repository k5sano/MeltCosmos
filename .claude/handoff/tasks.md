```markdown
Copy# EMSpace 実装タスク

## 優先順位

1. CMakeLists.txt 作成
2. Parameters.h (全11パラメータ)
3. DSP 実装
   a. InputDiffuser.h
   b. ToneFilter.h
   c. ModDelayLine.h
   d. ReverbTank.h / .cpp
   e. SpaceEngine.h / .cpp
4. LevelMeter.h
5. PluginProcessor.h / .cpp
6. PluginEditor.h / .cpp (レベルメーター付き)
7. PresetManager.h / .cpp
8. ビルド確認
9. Tests/SpaceEngineTests.cpp
10. scripts/run-pluginval.sh

## 注意事項

- processBlock 内で new/delete 禁止
- APVTS は PluginProcessor コンストラクタで初期化
- 1関数 50行以内、1ファイル 300行以内
- InputDiffuser と ReverbTank は DattorroReverb.cpp の
  コードを分割して再構成 (アルゴリズム変更ではない)
- cross_feed=0 で既存 EMverb と等価な音質を確認

## 完了条件

- `cmake --build build/` 成功
- VST3 + AU + Standalone 生成
- 全11パラメータが APVTS 登録
- pluginval --strictness-level 5 パス
Copy
```