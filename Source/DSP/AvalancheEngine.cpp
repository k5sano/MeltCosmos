#include "AvalancheEngine.h" #include #include #include

AvalancheEngine::AvalancheEngine() { init(); }

void AvalancheEngine::init() { delay\_.init(); reverb\_.init(); smoothDelayMix\_ = delayMix\_; smoothReverbMix\_ = reverbMix\_; }

void AvalancheEngine::clear() { delay\_.clear(); reverb\_.clear(); }

// --- Delay parameters --- void AvalancheEngine::setDelayTime(float norm) { delay\_.setDelayTime(norm); } void AvalancheEngine::setDelayFeedback(float fb) { delay\_.setFeedback(fb); } void AvalancheEngine::setDelayTone(float tone) { delay\_.setTone(tone); } void AvalancheEngine::setDelayMix(float mix) { delayMix\_ = mix; }

// --- Reverb parameters --- void AvalancheEngine::setReverbDecay(float decay) { reverb\_.setDecay(decay); } void AvalancheEngine::setReverbDamping(float damp) { reverb\_.setLp(damp); } void AvalancheEngine::setReverbMix(float mix) { reverbMix\_ = mix; }

// --- Shared parameters --- void AvalancheEngine::setDiffusion(float diff) { reverb\_.setDiffusion(diff); } void AvalancheEngine::setCrossFeed(float amount) { crossFeed\_ = amount; }

void AvalancheEngine::setModDepth(float depth) { delay\_.setModDepth(depth); reverb\_.setModSpeed(depth); }

void AvalancheEngine::setMode(int mode) { delay\_.setMode(mode); }

// ============================================================ // Process — Avalanche Run style signal flow // // Input → ModDelay → DattorroReverb // ↑ │ // └── crossfeed ┘ // // Output = dry\*(1-mix) + delayWet_dMix + reverbWet_rMix // ============================================================

void AvalancheEngine::process(float\* inOutL, float\* inOutR, int numSamples) { // Reverb engine works in-place on its own buffer pair, // so we prepare separate buffers for the reverb input. // To stay within 300-line limit, we process sample-by-sample // and accumulate into the in-place buffers.

```
for (int i = 0; i < numSamples; ++i)
{
    // Smooth mix parameters to avoid zipper noise
    smoothDelayMix_  += kSmoothCoeff * (delayMix_  - smoothDelayMix_);
    smoothReverbMix_ += kSmoothCoeff * (reverbMix_ - smoothReverbMix_);

    float dryL = inOutL[i];
    float dryR = inOutR[i];
    float monoIn = (dryL + dryR) * 0.5f;

    // --- Cross-feed injection from previous reverb tail ---
    float xfeed = (reverb_.lastWetL() + reverb_.lastWetR()) * 0.5f;
    delay_.injectCrossFeed(xfeed * crossFeed_);

    // --- Mod Delay (mono in, mono out) ---
    float delayOut = delay_.processSample(monoIn);

    // --- Feed delay output into reverb ---
    // Reverb input = delay output (the reverb processes in-place)
    float revInL = delayOut;
    float revInR = delayOut;

    // Reverb amount is set to 1.0 internally;
    // we control wet/dry externally for finer control.
    reverb_.setAmount(1.0f);
    reverb_.setInputGain(1.0f);

    // Process 1 sample through reverb (in-place)
    reverb_.process(&revInL, &revInR, 1);

    // revInL/R now contain the reverb wet output

    // --- Mix ---
    float dMix = smoothDelayMix_;
    float rMix = smoothReverbMix_;

    // Delay contributes to both channels with slight spread
    float delayL = delayOut;
    float delayR = delayOut;

    // Final output
    float wetL = delayL * dMix + revInL * rMix;
    float wetR = delayR * dMix + revInR * rMix;

    // Dry/wet blend: full wet when both mixes are at max
    float totalWet = std::min(1.0f, dMix + rMix);
    float dryGain = 1.0f - totalWet * 0.5f;

    inOutL[i] = dryL * dryGain + wetL;
    inOutR[i] = dryR * dryGain + wetR;

    // Soft clip to prevent runaway
    inOutL[i] = std::tanh(inOutL[i]);
    inOutR[i] = std::tanh(inOutR[i]);
}
```

}