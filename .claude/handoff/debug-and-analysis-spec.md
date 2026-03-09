# EMverb Avalanche — デバッグ・解析ツール仕様書

## 目的

複雑なクロスフィードルーティングのゲイン管理を可視化し、 「音が来ているか」「どこで爆発しているか」「どこで消えているか」を リアルタイムで確認できるようにする。

## 1\. 内部レベルメーター (PluginEditor 内蔵)

### 計測ポイント (6箇所)

| ID | 位置 | 説明 |
| --- | --- | --- |
| `input` | processBlock 入口 | 入力信号 L/R の Peak+RMS |
| `delay_out` | ModDelay 出力 | ディレイ単体の出力レベル |
| `reverb_in` | DattorroReverb 入口 | リバーブに入る信号 (ディレイ後) |
| `reverb_out` | DattorroReverb 出口 | リバーブ Wet 出力 L/R |
| `crossfeed` | クロスフィード経路 | リバーブ→ディレイ FB に戻る信号量 |
| `output` | processBlock 出口 | 最終出力 L/R の Peak+RMS |

### 表示

-   各ポイントに Peak (白バー) + RMS (色付きバー) を水平表示
-   Peak は -60dB 〜 +6dB 範囲
-   色: 緑 (< -12dB), 黄 (-12dB 〜 -3dB), 赤 (> -3dB)
-   Peak ホールド 1.5秒、減衰 15dB/秒
-   GUI 下半分に信号フロー図風に配置

### 実装方式

-   `Source/DSP/LevelMeter.h`: lock-free な atomic ベースの Peak/RMS 計測
-   `AvalancheEngine` 内の各ステージで `LevelMeter::pushSample()` を呼ぶ
-   `PluginEditor` 側で 30fps タイマーで `LevelMeter::getAndReset()` を読み取り描画
-   processBlock ⇔ Editor 間はすべて `std::atomic<float>` で受け渡し (lock-free)

## 2\. デバッグログ出力 (JUCE Logger 経由)

-   `DEBUG_LEVELS` マクロを有効にすると、1秒ごとに各ポイントの Peak/RMS を `DBG()` でコンソール出力
-   リリースビルドでは完全に除去される

## 3\. 外部解析ツール統合

### 3-1. pluginval (Tracktion) — プラグイン正常性検証

-   **用途**: クラッシュ、メモリリーク、パラメータ異常、オーディオスレッド違反の検出
-   **リポジトリ**: [https://github.com/Tracktion/pluginval](https://github.com/Tracktion/pluginval)
-   **インストール**:
    
    ```bash
    Copy# macOS (Homebrew)
    brew install pluginval
    # または GitHub Releases からダウンロード
    # https://github.com/Tracktion/pluginval/releases
    ```
    
-   **実行手順**:
    
    ```bash
    Copy# Strictness Level 1-10 (開発中は5推奨、リリース前は10)
    pluginval --validate "build/EMverbAvalanche_artefacts/VST3/EMverb Avalanche.vst3" --strictness-level 5
    # CI用 (タイムアウト付き)
    pluginval --validate "build/EMverbAvalanche_artefacts/VST3/EMverb Avalanche.vst3" --strictness-level 10 --timeout-ms 60000
    ```
    
-   **チェック項目**: パラメータ範囲外アクセス、processBlock 内の allocate、 NaN/Inf 出力、急激なパラメータ変更への耐性

### 3-2. Plugin Analyser (Conceptual-Machines) — 信号特性の定量計測

-   **用途**: 周波数応答、THD、ダイナミクス (入出力レベル特性)、 パラメータ空間のグリッドサーチ
-   **リポジトリ**: [https://github.com/Conceptual-Machines/plugin-analyser](https://github.com/Conceptual-Machines/plugin-analyser)
-   **ビルド**:
    
    ```bash
    Copygit clone https://github.com/Conceptual-Machines/plugin-analyser.git
    cd plugin-analyser
    mkdir build && cd build
    cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
    cmake --build . --parallel
    ```
    
-   **使い方 (GUI)**:
    1.  PluginAnalyser を起動
    2.  VST3 ファイルを読み込み
    3.  計測したいパラメータ (cross\_feed, delay\_feedback, reverb\_decay) を選択
    4.  Analyzer: RmsPeak + LinearResponse + Thd を有効化
    5.  Signal: sweep (対数スイープ) を選択
    6.  実行 → CSV エクスポート
-   **使い方 (CLI)**:
    
    ```bash
    Copy./plugin_measure_grid_cli \
      --config ../emverb_avalanche_config.json \
      --out ./results/ \
      --plugin "path/to/EMverb Avalanche.vst3"
    ```
    
-   **重点計測シナリオ**:
    -   cross\_feed を 0.0 → 1.0 で 10段階掃引 + feedback 0.9 固定 → 発振しないか、出力が bounded か確認
    -   reverb\_decay 0.99 + cross\_feed 0.8 で 10秒放置 → テイルが∞に発散しないか確認
    -   全パラメータ min/center/max の 3^11 ≈ 177K 組み合わせで NaN/Inf/クリップを検出

### 3-3. Voxengo SPAN — リアルタイム周波数スペクトル確認

-   **用途**: DAW 内で EMverb Avalanche の後段に挿して、 リアルタイムのスペクトルバランスを目視確認
-   **入手**: [https://www.voxengo.com/product/span/](https://www.voxengo.com/product/span/) (フリー)
-   **確認ポイント**:
    -   クロスフィード増加時に特定周波数でピーク蓄積がないか
    -   Tone フィルタの効きが適切か (LP/HP の傾斜)
    -   Reverse モードでの DC オフセット発生がないか

### 3-4. MeldaProduction MAnalyzer — 高精度スペクトル + ソノグラム

-   **用途**: ソノグラム表示でリバーブテイルの時間変化を可視化
-   **入手**: [https://www.meldaproduction.com/MFreeFXBundle](https://www.meldaproduction.com/MFreeFXBundle) (フリーバンドル内)
-   **確認ポイント**:
    -   リバーブテイルの減衰カーブが自然か
    -   モジュレーションが時間軸上でどう揺らいでいるか

## 4\. ファイル構成 (追加分)

```
Source/
  DSP/
    LevelMeter.h              ← 新規: lock-free Peak/RMS 計測
    AvalancheEngine.h / .cpp  ← 変更: 計測ポイント追加
  PluginProcessor.h / .cpp    ← 変更: LevelMeter 保持
  PluginEditor.h / .cpp       ← 変更: メーター描画 + フロー図
Tools/
  emverb_avalanche_config.json ← Plugin Analyser 用設定
  validate.sh                  ← pluginval ワンショット実行スクリプト
  analyze.sh                   ← Plugin Analyser CLI 実行スクリプト
```