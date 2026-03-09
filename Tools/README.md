# EMverb Avalanche — デバッグ・解析ツール

## 概要

複雑なクロスフィードルーティングのゲイン管理を検証するための 内部レベルメーターと外部解析ツール群。

## 内蔵レベルメーター

プラグインGUI下部に6ポイントのリアルタイムメーターを表示:

```
INPUT  ─→ DELAY ─→ REV.IN ─→ REV.OUT ─→ OUTPUT
                                │
                                └─→ X-FEED ─→ DELAY feedback
```

-   Peak (カラーバー) + RMS (半透明バー) + Peak Hold (白線)
-   色: 緑 (< -12dB), 黄 (-12 〜 -3dB), 赤 (> -3dB)
-   デバッグビルドではコンソールにも1秒ごとにレベルをログ出力

## 外部ツール

### 1\. pluginval — プラグイン正常性検証

```bash
Copy# インストール
brew install pluginval

# 実行 (strictness 1-10)
./Tools/validate.sh 5      # 開発中
./Tools/validate.sh 10     # リリース前
```

**検証項目:**

-   パラメータ範囲外アクセス
-   processBlock 内のメモリ確保
-   NaN / Inf 出力
-   急激なパラメータ変更への耐性
-   サンプルレート変更への対応

### 2\. Plugin Analyser — 信号特性の定量計測

```bash
Copy# ビルド (初回のみ)
git clone https://github.com/Conceptual-Machines/plugin-analyser.git
cd plugin-analyser && mkdir build && cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel

# 実行
cd /path/to/EMverbAvalanche
./Tools/analyze.sh ../plugin-analyser/build/plugin_measure_grid_cli
```

**計測内容:**

-   cross\_feed 0→1 の掃引で出力レベルが bounded か
-   高フィードバック + 高クロスフィードでの発振チェック
-   周波数応答のバランス (特定帯域に蓄積がないか)
-   THD (高域の歪み特性)

### 3\. Voxengo SPAN — リアルタイムスペクトル確認

```
ダウンロード: https://www.voxengo.com/product/span/
```

DAW 内で EMverb Avalanche の後段に挿して使用:

-   クロスフィード増加時の帯域蓄積チェック
-   Tone フィルタの LP/HP 傾斜確認
-   Reverse モードの DC オフセット確認

### 4\. MeldaProduction MAnalyzer — ソノグラム

```
ダウンロード: https://www.meldaproduction.com/MFreeFXBundle
(MFreeFXBundle 内に含まれる)
```

-   リバーブテイルの時間的な減衰カーブを可視化
-   モジュレーションの揺らぎを時間軸で確認

## 推奨デバッグワークフロー

```
1. cmake -B build && cmake --build build/
2. ./Tools/validate.sh 5                    ← まず壊れてないか確認
3. DAW で起動、内蔵メーターで各段のレベル確認
4. パラメータを極端に振ってメーターが赤にならないか確認
   - cross_feed=1.0 + feedback=0.95
   - reverb_decay=0.99 + cross_feed=0.8
   - mode=Reverse + feedback=0.9
5. SPAN を後段に挿してスペクトルバランスを確認
6. ./Tools/analyze.sh で定量計測、CSV を確認
7. リリース前: ./Tools/validate.sh 10
```