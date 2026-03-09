```cpp
Copy#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "SpaceEngine.h"
#include <vector>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static float rms(const float* buf, int n)
{
    double sum = 0.0;
    for (int i = 0; i < n; ++i)
        sum += static_cast<double>(buf[i]) * buf[i];
    return static_cast<float>(std::sqrt(sum / n));
}

// ============================================================
TEST_CASE("Silence in → silence out (zero feedback/decay)", "[space]")
{
    SpaceEngine eng;
    eng.prepare(48000.0);
    eng.setDelayFeedback(0.0f);
    eng.setReverbDecay(0.0f);
    eng.setReverbMix(1.0f);
    eng.setDelayMix(0.5f);
    eng.setCrossFeed(0.0f);

    constexpr int N = 4096;
    std::vector<float> L(N, 0.0f), R(N, 0.0f);
    eng.process(L.data(), R.data(), N);

    REQUIRE(rms(L.data(), N) < 1e-6f);
    REQUIRE(rms(R.data(), N) < 1e-6f);
}

// ============================================================
TEST_CASE("Impulse produces reverb tail", "[space]")
{
    SpaceEngine eng;
    eng.prepare(48000.0);
    eng.setReverbDecay(0.7f);
    eng.setReverbMix(1.0f);
    eng.setDelayMix(0.0f);
    eng.setCrossFeed(0.0f);
    eng.setDiffusion(0.625f);
    eng.setReverbDamping(0.7f);

    constexpr int N = 16000;
    std::vector<float> L(N, 0.0f), R(N, 0.0f);
    L[0] = 1.0f; R[0] = 1.0f;

    eng.process(L.data(), R.data(), N);
    REQUIRE(rms(L.data() + 2000, N - 2000) > 1e-4f);
}

// ============================================================
TEST_CASE("Delay produces echo", "[space]")
{
    SpaceEngine eng;
    eng.prepare(48000.0);
    eng.setDelayTime(0.5f);     // 0.25 * 2000 = 500ms
    eng.setDelayFeedback(0.0f);
    eng.setDelayMix(1.0f);
    eng.setReverbMix(0.0f);     // reverb off for this test
    eng.setCrossFeed(0.0f);

    constexpr int N = 48000;
    std::vector<float> L(N, 0.0f), R(N, 0.0f);
    L[0] = 1.0f; R[0] = 1.0f;

    eng.process(L.data(), R.data(), N);

    // There should be energy around 500ms = 24000 samples
    float midRms = rms(L.data() + 20000, 8000);
    REQUIRE(midRms > 1e-4f);
}

// ============================================================
TEST_CASE("Cross-feed increases density", "[space]")
{
    auto runWithCF = [](float cf) -> float {
        SpaceEngine eng;
        eng.prepare(48000.0);
        eng.setDelayTime(0.3f);
        eng.setDelayFeedback(0.4f);
        eng.setDelayMix(0.5f);
        eng.setReverbDecay(0.6f);
        eng.setReverbMix(0.8f);
        eng.setCrossFeed(cf);
        eng.setDiffusion(0.625f);
        eng.setReverbDamping(0.7f);

        constexpr int N = 24000;
        std::vector<float> L(N, 0.0f), R(N, 0.0f);
        L[0] = 1.0f; R[0] = 1.0f;
        eng.process(L.data(), R.data(), N);
        return rms(L.data() + 12000, 12000);
    };

    float noCF   = runWithCF(0.0f);
    float withCF = runWithCF(0.8f);

    // Cross-feed should add energy/density
    REQUIRE(withCF > noCF);
}

// ============================================================
TEST_CASE("Clear resets all state", "[space]")
{
    SpaceEngine eng;
    eng.prepare(48000.0);
    eng.setReverbDecay(0.9f);
    eng.setDelayFeedback(0.6f);
    eng.setReverbMix(1.0f);
    eng.setDelayMix(0.5f);
    eng.setCrossFeed(0.5f);

    constexpr int N = 4096;
    std::vector<float> L(N, 0.0f), R(N, 0.0f);
    L[0] = 1.0f; R[0] = 1.0f;
    eng.process(L.data(), R.data(), N);

    eng.clear();
    std::fill(L.begin(), L.end(), 0.0f);
    std::fill(R.begin(), R.end(), 0.0f);
    eng.process(L.data(), R.data(), N);

    REQUIRE(rms(L.data(), N) < 1e-6f);
}

// ============================================================
TEST_CASE("Works at 44100Hz", "[space]")
{
    SpaceEngine eng;
    eng.prepare(44100.0);
    eng.setReverbDecay(0.5f);
    eng.setDelayTime(0.3f);
    eng.setDelayMix(0.5f);
    eng.setReverbMix(0.8f);

    constexpr int N = 8000;
    std::vector<float> L(N, 0.0f), R(N, 0.0f);
    L[0] = 1.0f; R[0] = 1.0f;
    eng.process(L.data(), R.data(), N);

    REQUIRE(rms(L.data() + 2000, 4000) > 1e-5f);
}

// ============================================================
TEST_CASE("Works at 96000Hz", "[space]")
{
    SpaceEngine eng;
    eng.prepare(96000.0);
    eng.setReverbDecay(0.5f);
    eng.setDelayTime(0.3f);
    eng.setDelayMix(0.5f);
    eng.setReverbMix(0.8f);

    constexpr int N = 16000;
    std::vector<float> L(N, 0.0f), R(N, 0.0f);
    L[0] = 1.0f; R[0] = 1.0f;
    eng.process(L.data(), R.data(), N);

    REQUIRE(rms(L.data() + 4000, 8000) > 1e-5f);
}

// ============================================================
TEST_CASE("ReverbMix=0 gives dry signal", "[space]")
{
    SpaceEngine eng;
    eng.prepare(48000.0);
    eng.setReverbMix(0.0f);
    eng.setDelayMix(0.0f);
    eng.setCrossFeed(0.0f);

    constexpr int N = 256;
    std::vector<float> L(N), R(N), origL(N);
    for (int i = 0; i < N; ++i) {
        float v = std::sin(2.0f * static_cast<float>(M_PI)
                           * 440.0f * i / 48000.0f);
        L[i] = v; R[i] = v; origL[i] = v;
    }

    eng.process(L.data(), R.data(), N);

    // With reverbMix=0, delayMix=0: output ≈ diffused dry
    // Since diffuser is present, exact match won't hold.
    // But output should have similar energy level.
    float origRms = rms(origL.data(), N);
    float outRms  = rms(L.data(), N);
    float ratio = outRms / std::max(origRms, 1e-10f);
    REQUIRE(ratio > 0.3f);
    REQUIRE(ratio < 3.0f);
}

// ============================================================
TEST_CASE("No blowup with max feedback + cross-feed", "[space]")
{
    SpaceEngine eng;
    eng.prepare(48000.0);
    eng.setDelayFeedback(1.0f);
    eng.setCrossFeed(1.0f);
    eng.setReverbDecay(0.99f);
    eng.setReverbMix(1.0f);
    eng.setDelayMix(1.0f);

    constexpr int N = 96000; // 2 seconds
    std::vector<float> L(N, 0.0f), R(N, 0.0f);
    L[0] = 1.0f; R[0] = 1.0f;
    eng.process(L.data(), R.data(), N);

    // Should be clamped, no inf/nan
    for (int i = 0; i < N; ++i) {
        REQUIRE(std::isfinite(L[i]));
        REQUIRE(std::isfinite(R[i]));
        REQUIRE(std::abs(L[i]) <= 4.0f);
        REQUIRE(std::abs(R[i]) <= 4.0f);
    }
}
Copy
```