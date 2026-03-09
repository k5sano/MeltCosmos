```markdown
Copy# EMSpace 設計書

## クラス構成

```

EMSpacePlugin : juce::AudioProcessor ├── SpaceEngine (統合 DSP コア) │ ├── InputDiffuser (AP×4 — DattorroReverb の前段を抽出) │ ├── ModDelayLine (0–2000ms, LFO モジュレーション付き) │ │ └── ToneFilter (1次 IIR tilt EQ) │ ├── ReverbTank (DattorroReverb のループ部を抽出) │ │ ├── CosineOsc ×2 (LFO) │ │ └── LP Damping (1次 IIR) │ └── CrossFeed パス (Tank tail → Delay FB) ├── LevelMeter (Input / Output RMS, atomic で GUI へ) └── PresetManager

```

## SpaceEngine 内部処理フロー (1サンプル単位)

```

1.  mono = (L + R) \* 0.5
    
2.  dry = mono
    
    // Input Diffusion AP ×4
    
3.  diffused = InputDiffuser.process(mono)
    
    // Cross-feed: reverb tail の一部を delay FB に混合
    
4.  fbSignal = delayOut\_prev \* feedback + reverbTail\_prev \* crossFeed
    
5.  fbFiltered = ToneFilter.process(fbSignal)
    
    // Mod Delay Line
    
6.  delayOut = ModDelayLine.read(delayTimeSamples + lfoMod)
    
7.  ModDelayLine.write(diffused + fbFiltered)
    
    // Delay mix → Tank 入力
    
8.  tankInput = diffused \* (1 - delayMix) + delayOut \* delayMix
    
    // Reverb Tank (Dattorro ループ)
    
9.  (wetL, wetR, reverbTail) = ReverbTank.process(tankInput)
    
    // 最終出力
    
10.  outL = dry \* (1 - reverbMix) + wetL \* reverbMix
     
11.  outR = dry \* (1 - reverbMix) + wetR \* reverbMix
     

// 次サンプルへ持ち越し 12. delayOut\_prev = delayOut 13. reverbTail\_prev = reverbTail

```

## ファイル構成

```

Source/ PluginProcessor.h PluginProcessor.cpp PluginEditor.h PluginEditor.cpp Parameters.h LevelMeter.h PresetManager.h PresetManager.cpp DSP/ SpaceEngine.h SpaceEngine.cpp InputDiffuser.h ← AP×4 (DattorroReverb 前段抽出) ModDelayLine.h ← 最大 2000ms + LFO mod ToneFilter.h ← 1次 IIR tilt EQ ReverbTank.h ← Dattorro ループ部 (tank only) ReverbTank.cpp CMakeLists.txt Tests/ SpaceEngineTests.cpp scripts/ run-pluginval.sh

```

## モジュール責務

### InputDiffuser
DattorroReverb の AP1–AP4 を独立クラス化。
smear LFO は InputDiffuser が管理。
prepare() でサンプルレート依存のタップ長をスケーリング。

### ModDelayLine
最大 96000 サンプル (2000ms@48kHz) の循環バッファ。
三角波 LFO でリードポインタを揺らし、Hermite 補間で読み出す。
書き込み = diffused + fbFiltered (cross-feed 混合済み)。

### ToneFilter
tilt EQ: パラメータ 0.5 で flat、0→LP 強調、1→HP 強調。
LP 係数と HP 係数を param から算出し、1次 IIR で処理。

### ReverbTank
DattorroReverb のループ部 (Dap1a/1b → Del1, Dap2a/2b → Del2)。
外部から tankInput を受け取り、(wetL, wetR, tail) を返す。
tail = (Del1末端 + Del2末端) * 0.5 — cross-feed ソース用。

### SpaceEngine
上記4モジュール + CrossFeed ロジックを統合。
processBlock() でバッファ単位処理、内部は per-sample。

### LevelMeter
processBlock 前後で RMS を計算し std::atomic<float> に格納。
GUI タイマーコールバック (30Hz) で読み出して描画。
```