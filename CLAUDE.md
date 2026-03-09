# EMverb Avalanche — Claude Code プロジェクト設定

## プロジェクト概要

EarthQuaker Devices Avalanche Run の設計思想を参考にした ディレイ＋リバーブ融合空間系 VST3 プラグイン。 コアリバーブは Mutable Instruments Clouds の Dattorro plate reverb (既存 EMverb) を流用。 モジュレーションディレイ + クロスフィード + リバースモード + スウェルモードを追加。 JUCE フレームワークで開発。外部依存なしで自己完結する。

## JUCE ルール

-   コードは常に JUCE 公式ドキュメント ([https://docs.juce.com/](https://docs.juce.com/)) の最新仕様に準拠すること。
-   非推奨 (deprecated) の API は使用しないこと。代替 API が存在する場合は必ずそちらを使うこと。
    -   例: `juce::Font::Font(float size)` → `juce::Font(juce::FontOptions{}.withHeight(size))` を使う。
    -   例: `AudioProcessorValueTreeState` のパラメータ追加には `juce::ParameterID` を使う。
-   `JuceHeader.h` の利用には `juce_generate_juce_header(<target>)` を CMakeLists.txt に記述すること。
-   JUCE モジュールは `target_link_libraries` で `juce::juce_*` 形式で指定すること。
-   MSVC ビルドでは `_USE_MATH_DEFINES` を定義して `M_PI` を有効にすること。

## CMake ルール

-   `cmake_minimum_required` は 3.22 以上を指定すること。
-   プラグインターゲットには `juce_add_plugin()` を使い、`FORMATS` に必要なフォーマット（VST3, Standalone 等）をリストすること。

## 全般ルール

-   C++17 以上を前提としたコードを書くこと。
-   生ポインタよりスマートポインタ (`std::unique_ptr`, `std::shared_ptr`) を優先すること。
-   スレッドセーフが求められる箇所には `std::atomic` または JUCE の APVTS 経由の仕組みを使うこと。
-   processBlock 内で new/delete 禁止。
-   1関数 50行以内、1ファイル 300行以内を目安に。超えたら分割を検討し報告。

## ビルドコマンド

```
cmake -B build
cmake --build build/
ctest --test-dir build/
```

## コミット前チェック

`cmake --build build/` が成功すること。

## ハンドオフワークフロー

-   仕様・設計・タスク: `.claude/handoff/`
-   ビルドエラー: `.claude/build-errors.txt`
-   エスカレーション: `.claude/escalation-report.md`

## コード修正の原則

仕様書で定義されたコードを尊重する。 修正は根拠がある場合のみ、最小限で。 詳細は `.claude/commands/build-fix.md` の指示に従う。