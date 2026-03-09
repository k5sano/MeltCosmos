#pragma once

#include <juce\_audio\_processors/juce\_audio\_processors.h> #include "Parameters.h" #include "DSP/AvalancheEngine.h" #include "SampleRateAdapter.h"

class AvalanchePlugin : public juce::AudioProcessor { public: AvalanchePlugin(); ~AvalanchePlugin() override = default;

```
void prepareToPlay(double sampleRate, int samplesPerBlock) override;
void releaseResources() override;
void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

juce::AudioProcessorEditor* createEditor() override;
bool hasEditor() const override { return true; }

const juce::String getName() const override
{
    return "EMverb Avalanche";
}
bool acceptsMidi() const override { return false; }
bool producesMidi() const override { return false; }
double getTailLengthSeconds() const override { return 8.0; }

int getNumPrograms() override { return 1; }
int getCurrentProgram() override { return 0; }
void setCurrentProgram(int) override {}
const juce::String getProgramName(int) override { return {}; }
void changeProgramName(int, const juce::String&) override {}

void getStateInformation(juce::MemoryBlock& destData) override;
void setStateInformation(const void* data, int sizeInBytes) override;

juce::AudioProcessorValueTreeState apvts;
```

private: std::atomic\* delayTimeParam = nullptr; std::atomic\* delayFeedbackParam = nullptr; std::atomic\* delayToneParam = nullptr; std::atomic\* delayMixParam = nullptr; std::atomic\* reverbDecayParam = nullptr; std::atomic\* reverbDampingParam = nullptr; std::atomic\* reverbMixParam = nullptr; std::atomic\* diffusionParam = nullptr; std::atomic\* crossFeedParam = nullptr; std::atomic\* modDepthParam = nullptr; std::atomic\* modeParam = nullptr;

```
AvalancheEngine engine_;
SampleRateAdapter adapter_;
juce::AudioBuffer<float> dryBuffer_;

JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AvalanchePlugin)
```

};