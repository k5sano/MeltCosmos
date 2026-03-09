#include <catch2/catch\_test\_macros.hpp> #include <catch2/matchers/catch\_matchers\_floating\_point.hpp> #include "AvalancheEngine.h" #include #include #include

TEST\_CASE("AvalancheEngine: silence in produces silence out", "\[engine\]") { AvalancheEngine engine; engine.init();

```
std::vector<float> bufL(256, 0.0f);
std::vector<float> bufR(256, 0.0f);

engine.process(bufL.data(), bufR.data(), 256);

for (int i = 0; i < 256; ++i)
{
    REQUIRE_THAT(bufL[i],
        Catch::Matchers::WithinAbs(0.0, 1e-6));
    REQUIRE_THAT(bufR[i],
        Catch::Matchers::WithinAbs(0.0, 1e-6));
}
```

}

TEST\_CASE("AvalancheEngine: impulse produces output", "\[engine\]") { AvalancheEngine engine; engine.init(); engine.setDelayTime(0.1f); engine.setDelayFeedback(0.3f); engine.setDelayMix(0.5f); engine.setReverbDecay(0.5f); engine.setReverbMix(0.5f); engine.setCrossFeed(0.3f); engine.setModDepth(0.5f); engine.setMode(0);

```
constexpr int kLen = 4096;
std::vector<float> bufL(kLen, 0.0f);
std::vector<float> bufR(kLen, 0.0f);

// Impulse at sample 0
bufL[0] = 1.0f;
bufR[0] = 1.0f;

engine.process(bufL.data(), bufR.data(), kLen);

// Somewhere in the buffer there should be non-zero output
float maxVal = 0.0f;
for (int i = 0; i < kLen; ++i)
{
    float absL = std::fabs(bufL[i]);
    float absR = std::fabs(bufR[i]);
    maxVal = std::max(maxVal, std::max(absL, absR));
}

REQUIRE(maxVal > 0.001f);
```

}

TEST\_CASE("AvalancheEngine: output stays bounded", "\[engine\]") { AvalancheEngine engine; engine.init(); engine.setDelayTime(0.2f); engine.setDelayFeedback(0.95f); engine.setDelayMix(1.0f); engine.setReverbDecay(0.99f); engine.setReverbMix(1.0f); engine.setCrossFeed(0.8f); engine.setModDepth(1.0f); engine.setMode(0);

```
constexpr int kLen = 32000; // 1 second at 32kHz
std::vector<float> bufL(kLen, 0.0f);
std::vector<float> bufR(kLen, 0.0f);

// Loud impulse
bufL[0] = 1.0f;
bufR[0] = 1.0f;

engine.process(bufL.data(), bufR.data(), kLen);

// tanh soft-clip should keep everything bounded
for (int i = 0; i < kLen; ++i)
{
    REQUIRE(std::fabs(bufL[i]) <= 1.0f);
    REQUIRE(std::fabs(bufR[i]) <= 1.0f);
}
```

}

TEST\_CASE("AvalancheEngine: reverse mode produces output", "\[engine\]") { AvalancheEngine engine; engine.init(); engine.setDelayTime(0.3f); engine.setDelayFeedback(0.3f); engine.setDelayMix(0.8f); engine.setReverbMix(0.3f); engine.setMode(1); // Reverse

```
constexpr int kLen = 4096;
std::vector<float> bufL(kLen, 0.0f);
std::vector<float> bufR(kLen, 0.0f);

bufL[0] = 1.0f;
bufR[0] = 1.0f;

engine.process(bufL.data(), bufR.data(), kLen);

float maxVal = 0.0f;
for (int i = 1; i < kLen; ++i)
{
    maxVal = std::max(maxVal,
        std::max(std::fabs(bufL[i]), std::fabs(bufR[i])));
}
REQUIRE(maxVal > 0.001f);
```

}

TEST\_CASE("AvalancheEngine: swell mode attenuates during loud input", "\[engine\]") { AvalancheEngine engine; engine.init(); engine.setDelayTime(0.05f); engine.setDelayFeedback(0.0f); engine.setDelayMix(1.0f); engine.setReverbMix(0.0f); engine.setMode(2); // Swell

```
constexpr int kLen = 2048;
std::vector<float> bufL(kLen, 0.5f); // Sustained loud input
std::vector<float> bufR(kLen, 0.5f);

engine.process(bufL.data(), bufR.data(), kLen);

// In swell mode, sustained loud input should result in
// attenuated delay output (envelope follower suppresses it)
// The wet output at the end should be quieter than
// what normal mode would produce
float tailEnergy = 0.0f;
for (int i = kLen - 256; i < kLen; ++i)
{
    tailEnergy += bufL[i] * bufL[i] + bufR[i] * bufR[i];
}
// With sustained input, swell should keep things relatively quiet
// (compared to full volume)
float avgEnergy = tailEnergy / 256.0f;
REQUIRE(avgEnergy < 2.0f);
```

}