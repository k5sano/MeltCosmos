```cpp
Copy#pragma once

#include "InputDiffuser.h"
#include "ModDelayLine.h"
#include "ToneFilter.h"
#include "ReverbTank.h"

/// Unified DSP engine: InputDiffuser → ModDelay → ReverbTank
/// with cross-feed from reverb tail back into delay feedback.
class SpaceEngine {
public:
    SpaceEngine();
    ~SpaceEngine() = default;

    void prepare(double sampleRate);
    void clear();

    void process(float* inOutL, float* inOutR, int numSamples);

    // Delay params
    void setDelayTime(float norm)     { delay_.setTimeNorm(norm); }
    void setDelayFeedback(float fb)   { feedback_ = fb; }
    void setDelayTone(float t)        { tone_.setTone(t); }
    void setDelayMix(float m)         { delayMix_ = m; }

    // Reverb params
    void setReverbDecay(float d)      { tank_.setDecay(d); }
    void setReverbDamping(float lp)   { tank_.setDamping(lp); }
    void setReverbMix(float m)        { reverbMix_ = m; }

    // Shared params
    void setDiffusion(float k);
    void setCrossFeed(float cf)       { crossFeed_ = cf; }
    void setModSpeed(float s);
    void setModDepth(float d)         { delay_.setModDepth(d); }

private:
    InputDiffuser diffuser_;
    ModDelayLine  delay_;
    ToneFilter    tone_;
    ReverbTank    tank_;

    float feedback_   = 0.4f;
    float delayMix_   = 0.5f;
    float reverbMix_  = 0.5f;
    float crossFeed_  = 0.3f;

    // Per-sample state carried across samples
    float delayOutPrev_  = 0.0f;
    float reverbTailPrev_ = 0.0f;
};
Copy
```