#include "AvalancheEngine.h" #include #include #include

AvalancheEngine::AvalancheEngine() { init(); }

void AvalancheEngine::init() { delay\_.init(); reverb\_.init(); smoothDelayMix\_ = delayMix\_; smoothReverbMix\_ = reverbMix\_; }

void AvalancheEngine::clear() { delay\_.clear(); reverb\_.clear(); }

// --- Delay parameters --- void AvalancheEngine::setDelayTime(float norm) { delay\_.setDelayTime(norm); }

void AvalancheEngine::setDelayFeedback(float fb) { delay\_.setFeedback(fb); }

void AvalancheEngine::setDelayTone(float tone) { delay\_.setTone(tone); }

void AvalancheEngine::setDelayMix(float mix) { delayMix\_ = mix; }

// --- Reverb parameters --- void AvalancheEngine::setReverbDecay(float decay) { reverb\_.setDecay(decay); }

void AvalancheEngine::setReverbDamping(float damp) { reverb\_.setLp(damp); }

void AvalancheEngine::setReverbMix(float mix) { reverbMix\_ = mix; }

// --- Shared parameters --- void AvalancheEngine::setDiffusion(float diff) { reverb\_.setDiffusion(diff); }

void AvalancheEngine::setCrossFeed(float amount) { crossFeed\_ = amount; }

void AvalancheEngine::setModDepth(float depth) { delay\_.setModDepth(depth); reverb\_.setModSpeed(depth); }

void AvalancheEngine::setMode(int mode) { delay\_.setMode(mode); }

// ============================================================ // Process — Avalanche Run style signal flow with level metering // ============================================================

void AvalancheEngine::process(float\* inOutL, float\* inOutR, int numSamples) { for (int i = 0; i < numSamples; ++i) { // Smooth mix parameters smoothDelayMix\_ += kSmoothCoeff \* (delayMix\_ - smoothDelayMix\_); smoothReverbMix\_ += kSmoothCoeff \* (reverbMix\_ - smoothReverbMix\_);

```
    float dryL = inOutL[i];
    float dryR = inOutR[i];
    float monoIn = (dryL + dryR) * 0.5f;

    // [METER] Input
    meterInput.pushSample(monoIn);

    // --- Cross-feed injection from previous reverb tail ---
    float xfeed = (reverb_.lastWetL() + reverb_.lastWetR())
                 * 0.5f;
    float xfeedScaled = xfeed * crossFeed_;
    delay_.injectCrossFeed(xfeedScaled);

    // [METER] CrossFeed
    meterCrossFeed.pushSample(xfeedScaled);

    // --- Mod Delay (mono in, mono out) ---
    float delayOut = delay_.processSample(monoIn);

    // [METER] Delay Out
    meterDelayOut.pushSample(delayOut);

    // --- Feed delay output into reverb ---
    float revInL = delayOut;
    float revInR = delayOut;

    // [METER] Reverb In
    meterReverbIn.pushSample(delayOut);

    reverb_.setAmount(1.0f);
    reverb_.setInputGain(1.0f);
    reverb_.process(&revInL, &revInR, 1);

    // [METER] Reverb Out
    meterReverbOut.pushSample(
        (revInL + revInR) * 0.5f);

    // --- Mix ---
    float dMix = smoothDelayMix_;
    float rMix = smoothReverbMix_;

    float wetL = delayOut * dMix + revInL * rMix;
    float wetR = delayOut * dMix + revInR * rMix;

    float totalWet = std::min(1.0f, dMix + rMix);
    float dryGain = 1.0f - totalWet * 0.5f;

    inOutL[i] = dryL * dryGain + wetL;
    inOutR[i] = dryR * dryGain + wetR;

    // Soft clip
    inOutL[i] = std::tanh(inOutL[i]);
    inOutR[i] = std::tanh(inOutR[i]);

    // [METER] Output
    meterOutput.pushSample(
        (inOutL[i] + inOutR[i]) * 0.5f);
}
```

}