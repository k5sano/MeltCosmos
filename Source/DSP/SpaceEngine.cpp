```cpp
Copy#include "SpaceEngine.h"
#include <cmath>
#include <algorithm>

SpaceEngine::SpaceEngine()
{
    prepare(48000.0);
}

void SpaceEngine::prepare(double sampleRate)
{
    diffuser_.prepare(sampleRate);
    delay_.prepare(sampleRate);
    tone_.prepare(sampleRate);
    tank_.prepare(sampleRate);

    delayOutPrev_ = 0.0f;
    reverbTailPrev_ = 0.0f;
}

void SpaceEngine::clear()
{
    diffuser_.clear();
    delay_.clear();
    tone_.clear();
    tank_.clear();
    delayOutPrev_ = 0.0f;
    reverbTailPrev_ = 0.0f;
}

void SpaceEngine::setDiffusion(float k)
{
    diffuser_.setDiffusion(k);
    tank_.setDiffusion(k);
}

void SpaceEngine::setModSpeed(float s)
{
    diffuser_.setModSpeed(s);
    delay_.setModSpeed(s);
    tank_.setModSpeed(s);
}

void SpaceEngine::process(float* inOutL, float* inOutR,
                           int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        // Mono sum
        float mono = (inOutL[i] + inOutR[i]) * 0.5f;
        float dry = mono;

        // 1. Input Diffusion AP ×4
        float diffused = diffuser_.process(mono);

        // 2. Cross-feed: reverb tail → delay feedback
        float fbSig = delayOutPrev_ * feedback_
                     + reverbTailPrev_ * crossFeed_;

        // 3. Tone filter on feedback
        float fbFilt = tone_.process(fbSig);

        // 4. Write to delay line (diffused + filtered feedback)
        delay_.write(diffused + fbFilt);

        // 5. Read from delay line (with LFO modulation)
        float delayOut = delay_.read();

        // 6. Delay mix → Tank input
        float tankIn = diffused * (1.0f - delayMix_)
                      + delayOut * delayMix_;

        // 7. Reverb Tank
        TankOutput tankOut = tank_.process(tankIn);

        // 8. Final output: dry/wet via reverb_mix
        float outL = dry * (1.0f - reverbMix_)
                    + tankOut.wetL * reverbMix_;
        float outR = dry * (1.0f - reverbMix_)
                    + tankOut.wetR * reverbMix_;

        // Soft clamp
        inOutL[i] = std::clamp(outL, -4.0f, 4.0f);
        inOutR[i] = std::clamp(outR, -4.0f, 4.0f);

        // 9. Carry over for next sample
        delayOutPrev_   = delayOut;
        reverbTailPrev_ = tankOut.tail;
    }
}
Copy
```