#pragma once

#include <juce\_audio\_processors/juce\_audio\_processors.h> #include "PluginProcessor.h" #include "DSP/LevelMeter.h"

class AvalancheEditor : public juce::AudioProcessorEditor, private juce::Timer { public: explicit AvalancheEditor(AvalanchePlugin&); ~AvalancheEditor() override;

```
void paint(juce::Graphics&) override;
void resized() override;
```

private: void timerCallback() override;

```
AvalanchePlugin& processor;
juce::GenericAudioProcessorEditor genericEditor;

// Cached meter levels for painting
struct MeterState {
    float peakDb = -100.0f;
    float rmsDb  = -100.0f;
    float peakHold = -100.0f;
    float holdTimer = 0.0f;
};

static constexpr int kNumMeters = 6;
MeterState meters_[kNumMeters] = {};
static constexpr const char* meterNames_[kNumMeters] = {
    "INPUT", "DELAY", "REV.IN", "REV.OUT",
    "X-FEED", "OUTPUT"
};

void drawMeter(juce::Graphics& g, juce::Rectangle<int> bounds,
               const char* label, const MeterState& state);

static juce::Colour levelToColour(float db);
static float dbToX(float db, float width);

JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AvalancheEditor)
```

};