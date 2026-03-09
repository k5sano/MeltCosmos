#pragma once

#include "DattorroReverb.h" #include "ModDelay.h"

/// Avalanche Engine: Mod Delay + Dattorro Reverb with cross-feed. /// Designed to be called at 32 kHz internal sample rate via SampleRateAdapter. /// Provides the same process(float\*, float\*, int) interface as DattorroReverb /// so SampleRateAdapter can drive it without modification. class AvalancheEngine { public: AvalancheEngine(); ~AvalancheEngine() = default;

```
void init();
void clear();

/// Main processing — same signature as DattorroReverb::process
void process(float* inOutL, float* inOutR, int numSamples);

// --- Delay parameters ---
void setDelayTime(float norm);
void setDelayFeedback(float fb);
void setDelayTone(float tone);
void setDelayMix(float mix);

// --- Reverb parameters ---
void setReverbDecay(float decay);
void setReverbDamping(float damp);
void setReverbMix(float mix);

// --- Shared parameters ---
void setDiffusion(float diff);
void setCrossFeed(float amount);
void setModDepth(float depth);
void setMode(int mode);
```

private: ModDelay delay\_; DattorroReverb reverb\_;

```
float delayMix_    = 0.5f;
float reverbMix_   = 0.5f;
float crossFeed_   = 0.3f;

float smoothDelayMix_  = 0.5f;
float smoothReverbMix_ = 0.5f;
static constexpr float kSmoothCoeff = 0.001f;
```

};