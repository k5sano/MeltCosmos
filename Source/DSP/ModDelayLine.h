```cpp
Copy#pragma once

#include <cmath>
#include <cstring>
#include <algorithm>
#include <memory>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/// Modulated delay line with Hermite interpolation.
/// Maximum 2000ms. Triangle-wave LFO for chorus-like mod.
class ModDelayLine {
public:
    void prepare(double sampleRate)
    {
        sr_ = std::max(sampleRate, 8000.0);
        // 2000ms + margin for LFO excursion
        int maxSamples = static_cast<int>(sr_ * 2.1);
        bufSize_ = nextPow2(maxSamples + 4);
        bufMask_ = bufSize_ - 1;
        buf_ = std::make_unique<float[]>(bufSize_);
        clear();
    }

    void clear()
    {
        if (buf_)
            std::memset(buf_.get(), 0,
                sizeof(float) * static_cast<size_t>(bufSize_));
        wp_ = 0;
        lfoPhase_ = 0.0f;
    }

    /// Set delay time in normalised 0–1 (quadratic → 0–2000ms)
    void setTimeNorm(float norm)
    {
        float ms = norm * norm * 2000.0f;
        delaySamples_ = ms * 0.001f * static_cast<float>(sr_);
        delaySamples_ = std::clamp(delaySamples_, 1.0f,
            static_cast<float>(bufSize_ - 4));
    }

    void setModDepth(float depth) { modDepth_ = depth; }

    void setModSpeed(float speed)
    {
        // 0.1–5 Hz triangle LFO
        lfoFreq_ = (0.1f + speed * 4.9f) / static_cast<float>(sr_);
    }

    void write(float sample)
    {
        buf_[wp_ & bufMask_] = sample;
        ++wp_;
    }

    float read()
    {
        // Triangle LFO
        lfoPhase_ += lfoFreq_;
        if (lfoPhase_ >= 1.0f) lfoPhase_ -= 1.0f;
        float tri = 4.0f * std::abs(lfoPhase_ - 0.5f) - 1.0f;

        // ±3ms max modulation
        float modSamples = tri * modDepth_ * 0.003f
                           * static_cast<float>(sr_);

        float totalDelay = delaySamples_ + modSamples;
        totalDelay = std::clamp(totalDelay, 1.0f,
            static_cast<float>(bufSize_ - 4));

        return hermiteRead(totalDelay);
    }

private:
    std::unique_ptr<float[]> buf_;
    int bufSize_ = 0, bufMask_ = 0, wp_ = 0;
    double sr_ = 48000.0;
    float delaySamples_ = 1.0f;
    float modDepth_ = 0.5f;
    float lfoPhase_ = 0.0f;
    float lfoFreq_ = 0.0f;

    static int nextPow2(int n)
    {
        int v = 1;
        while (v < n) v <<= 1;
        return v;
    }

    float hermiteRead(float delaySamp) const
    {
        float rp = static_cast<float>(wp_) - delaySamp;
        int idx = static_cast<int>(std::floor(rp));
        float frac = rp - static_cast<float>(idx);

        float xm1 = buf_[(idx - 1) & bufMask_];
        float x0  = buf_[(idx)     & bufMask_];
        float x1  = buf_[(idx + 1) & bufMask_];
        float x2  = buf_[(idx + 2) & bufMask_];

        // Hermite interpolation
        float c0 = x0;
        float c1 = 0.5f * (x1 - xm1);
        float c2 = xm1 - 2.5f * x0 + 2.0f * x1 - 0.5f * x2;
        float c3 = 0.5f * (x2 - xm1) + 1.5f * (x0 - x1);

        return ((c3 * frac + c2) * frac + c1) * frac + c0;
    }
};
Copy
```