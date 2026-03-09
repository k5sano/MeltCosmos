#pragma once

#include #include #include

#ifndef M\_PI #define M\_PI 3.14159265358979323846 #endif

/// Modulated delay line with tone filter and reverse/swell modes. /// Operates at 32 kHz internal sample rate. /// Maximum delay: 2 seconds (64000 samples at 32 kHz). class ModDelay { public: ModDelay() = default; ~ModDelay() = default;

```
void init()
{
    std::memset(buffer_, 0, sizeof(buffer_));
    writePos_ = 0;
    lfoPhase_ = 0.0f;
    lpState_ = 0.0f;
    hpState_ = 0.0f;
    feedbackSample_ = 0.0f;
    envelopeFollower_ = 0.0f;
    reverseReadPos_ = 0;
    reverseBlockLen_ = 0;
    reverseCount_ = 0;
}

void clear()
{
    std::memset(buffer_, 0, sizeof(buffer_));
    feedbackSample_ = 0.0f;
    lpState_ = 0.0f;
    hpState_ = 0.0f;
    envelopeFollower_ = 0.0f;
}

void setDelayTime(float norm)
{
    float samples = norm * kMaxDelaySamples;
    delaySamples_ = std::max(1.0f, samples);
}

void setFeedback(float fb)     { feedback_ = fb; }
void setTone(float tone)       { tone_ = tone; }
void setModDepth(float depth)  { modDepth_ = depth; }
void setMode(int mode)         { mode_ = mode; }

/// Inject external signal into the feedback path (cross-feed)
void injectCrossFeed(float sample) { crossFeedSample_ = sample; }

/// Process one sample. Returns the delayed (wet) output.
float processSample(float input)
{
    // Write input + feedback into buffer
    float toWrite = input + feedbackSample_;
    buffer_[writePos_] = toWrite;

    float delayed = 0.0f;

    if (mode_ == 0) // Normal
    {
        delayed = readInterpolated(delaySamples_);
    }
    else if (mode_ == 1) // Reverse
    {
        delayed = processReverse();
    }
    else // Swell
    {
        delayed = readInterpolated(delaySamples_);
        delayed = processSwell(input, delayed);
    }

    // Apply modulation to read position (chorus-like)
    if (mode_ != 1 && modDepth_ > 0.0f)
    {
        float lfoVal = std::sin(
            2.0f * static_cast<float>(M_PI) * lfoPhase_);
        float modOffset = lfoVal * modDepth_ * 40.0f;
        float modulated = readInterpolated(delaySamples_ + modOffset);
        delayed = delayed * 0.5f + modulated * 0.5f;
    }

    // Advance LFO
    lfoPhase_ += kLfoFreq;
    if (lfoPhase_ >= 1.0f) lfoPhase_ -= 1.0f;

    // Tone filter on feedback path
    float filtered = applyToneFilter(delayed);

    // Feedback with cross-feed injection
    feedbackSample_ = filtered * feedback_
                    + crossFeedSample_;
    crossFeedSample_ = 0.0f;

    // Advance write position
    writePos_ = (writePos_ + 1) & kBufferMask;

    return delayed;
}
```

private: // 32kHz × 2sec = 64000, round up to power of 2 static constexpr int kBufferSize = 65536; static constexpr int kBufferMask = kBufferSize - 1; static constexpr float kMaxDelaySamples = 63999.0f; static constexpr float kLfoFreq = 0.7f / 32000.0f; // Reverse grain size ~50ms at 32kHz static constexpr int kReverseGrain = 1600;

```
float buffer_[kBufferSize] = {};
int writePos_ = 0;

float delaySamples_ = 9600.0f; // default 300ms
float feedback_     = 0.4f;
float tone_         = 0.5f;
float modDepth_     = 0.5f;
int   mode_         = 0;

float lfoPhase_        = 0.0f;
float lpState_         = 0.0f;
float hpState_         = 0.0f;
float feedbackSample_  = 0.0f;
float crossFeedSample_ = 0.0f;
float envelopeFollower_ = 0.0f;

int reverseReadPos_ = 0;
int reverseBlockLen_ = 0;
int reverseCount_ = 0;

float readInterpolated(float delaySamp) const
{
    float clamped = std::max(1.0f,
        std::min(delaySamp, kMaxDelaySamples));
    float readPos = static_cast<float>(writePos_) - clamped;
    if (readPos < 0.0f) readPos += static_cast<float>(kBufferSize);

    int idx = static_cast<int>(readPos);
    float frac = readPos - static_cast<float>(idx);

    float s0 = buffer_[(idx - 1 + kBufferSize) & kBufferMask];
    float s1 = buffer_[idx & kBufferMask];
    float s2 = buffer_[(idx + 1) & kBufferMask];
    float s3 = buffer_[(idx + 2) & kBufferMask];

    // Hermite interpolation
    float c = (s2 - s0) * 0.5f;
    float v = s1 - s2;
    float w = c + v;
    float a = w + v + (s3 - s1) * 0.5f;
    float b_neg = w + a;
    return ((a * frac - b_neg) * frac + c) * frac + s1;
}

float processReverse()
{
    if (reverseCount_ <= 0)
    {
        int grainLen = static_cast<int>(delaySamples_);
        if (grainLen < 32) grainLen = 32;
        reverseBlockLen_ = std::min(grainLen, kReverseGrain);
        reverseReadPos_ = (writePos_ - 1 + kBufferSize) & kBufferMask;
        reverseCount_ = reverseBlockLen_;
    }

    // Crossfade window (raised cosine)
    float pos = static_cast<float>(
        reverseBlockLen_ - reverseCount_);
    float norm = pos / static_cast<float>(reverseBlockLen_);
    float window = 0.5f - 0.5f * std::cos(
        2.0f * static_cast<float>(M_PI) * norm);

    int readIdx = (reverseReadPos_ + reverseCount_) & kBufferMask;
    float sample = buffer_[readIdx] * window;

    --reverseCount_;
    return sample;
}

float processSwell(float dryInput, float delayed)
{
    // Envelope follower (attack ~10ms, release ~100ms)
    float absIn = std::fabs(dryInput);
    float attack  = 1.0f - std::exp(-1.0f / (32000.0f * 0.01f));
    float release = 1.0f - std::exp(-1.0f / (32000.0f * 0.10f));
    float coeff = (absIn > envelopeFollower_) ? attack : release;
    envelopeFollower_ += coeff * (absIn - envelopeFollower_);

    // Invert envelope: quiet when loud, loud when quiet
    float gain = 1.0f - std::min(1.0f, envelopeFollower_ * 8.0f);
    gain = gain * gain; // Smooth curve
    return delayed * gain;
}

float applyToneFilter(float input)
{
    // tone < 0.5 => low-pass (dark), tone > 0.5 => high-pass (bright)
    if (tone_ <= 0.5f)
    {
        float cutoff = 0.1f + tone_ * 1.8f; // 0.1 .. 1.0
        lpState_ += cutoff * (input - lpState_);
        return lpState_;
    }
    else
    {
        float cutoff = 0.1f + (1.0f - tone_) * 1.8f;
        lpState_ += cutoff * (input - lpState_);
        return input - lpState_ * (1.0f - (tone_ - 0.5f) * 0.6f);
    }
}
```

};