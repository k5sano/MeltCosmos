```

Avalanche Runの設計思想をもう少し深く調査してから、完全な仕様とコードを出力します。

ツールを使用する

|

検索

Dispatch Master delay into reverb feedback loop interaction DSP design

エージェントを表示

ツールを使用する

|

検索

Dattorro reverb delay cross-feed ambient pedal DSP implementation

エージェントを表示

ツールを使用する

|

読み取り

[https://raw.githubusercontent.com/k5sano/EMverb/main/Source/PluginEditor.h](https://raw.githubusercontent.com/k5sano/EMverb/main/Source/PluginEditor.h)

エージェントを表示

ツールを使用する

|

読み取り

[https://raw.githubusercontent.com/k5sano/EMverb/main/Source/PluginEditor.cpp](https://raw.githubusercontent.com/k5sano/EMverb/main/Source/PluginEditor.cpp)

エージェントを表示

全ソースを把握しました。既存EMverbのルール・構造に完全準拠して、Avalanche Run的な「とろけるディレイ+リバーブ空間系」の全ファイルを出力します。

* * *

【START:.claude/handoff/spec.md】

# EMverb Avalanche — とろけてたゆたう空間系プラグイン仕様書

## コンセプト

EarthQuaker Devices Avalanche Run の設計思想を参考に、 既存 EMverb (Dattorro plate reverb) を核としたディレイ＋リバーブ融合プラグインを構築する。

Avalanche Run の音響的特徴:

1.  **ディレイ→リバーブ直列 + リバーブテイル→ディレイフィードバックのクロスフィード**
2.  **フィードバックループ内トーンフィルタ** による有機的減衰
3.  **ディレイライン＋リバーブ双方への微量モジュレーション** によるたゆたい
4.  **リバースディレイ / スウェルモード** の3モード切替

## 信号フロー

```
Input (mono sum)
  │
  ├─────────────────────────────────────────── Dry
  │
  ▼
[Input Diffusion AP × 4]  ← 既存 DattorroReverb の前段
  │
  ▼
[Mod Delay Line] ◀──── feedback ◀── cross-feed ◀── reverb tail 一部
  │                       │
  │ Tone Filter (LP/HP)   │
  │                       │
  ▼                       │
[Dattorro Reverb Tank]    │
  │  │                    │
  │  └── reverb tail ─────┘ (cross-feed amount で混合)
  │
  ├── Left Out  (tank side A)
  └── Right Out (tank side B)

最終出力 = dry × (1 - mix) + wet × mix
```

## パラメータ一覧 (11個)

| ID | 表示名 | 範囲 | デフォルト | 説明 |
| --- | --- | --- | --- | --- |
| delay\_time | Time | 0.0 – 1.0 (→ 0ms – 2000ms) | 0.3 | ディレイタイム |
| delay\_feedback | Repeats | 0.0 – 1.0 | 0.4 | ディレイフィードバック量 |
| delay\_tone | Tone | 0.0 – 1.0 | 0.5 | FB内フィルタ (0=暗い, 0.5=flat, 1=明るい) |
| delay\_mix | D.Mix | 0.0 – 1.0 | 0.5 | ディレイウェット量 |
| reverb\_decay | Decay | 0.0 – 1.0 | 0.5 | リバーブ残響長 |
| reverb\_damping | Damping | 0.0 – 1.0 | 0.7 | リバーブ高域減衰 |
| reverb\_mix | R.Mix | 0.0 – 1.0 | 0.5 | リバーブウェット量 |
| diffusion | Diffusion | 0.0 – 1.0 | 0.625 | 拡散量 |
| cross\_feed | Cross Feed | 0.0 – 1.0 | 0.3 | リバーブ→ディレイFB混合量 |
| mod\_depth | Mod Depth | 0.0 – 1.0 | 0.5 | モジュレーション深さ |
| mode | Mode | choice 0/1/2 | 0 | Normal / Reverse / Swell |

## 内部サンプルレート

32 kHz (既存 SampleRateAdapter をそのまま使用)

## モード

-   **Normal (0)**: 通常ディレイ→リバーブ
-   **Reverse (1)**: 逆再生ディレイ→リバーブ
-   **Swell (2)**: エンベロープフォロワーで自動ボリュームスウェル→リバーブ

## ファイル構成

```
Source/
  DSP/
    DattorroReverb.h / .cpp    ← 既存 (変更なし)
    ModDelay.h                 ← 新規: モジュレーションディレイ
    AvalancheEngine.h / .cpp   ← 新規: ディレイ+リバーブ統合エンジン
  Parameters.h                 ← 置換: パラメータ定義
  PluginProcessor.h / .cpp     ← 置換: エンジン統合
  PluginEditor.h / .cpp        ← 置換: プラグイン名変更
  SampleRateAdapter.h / .cpp   ← 既存 (変更なし — AvalancheEngine を渡す形に)
```

## ビルド

```
cmake -B build
cmake --build build/
```