#pragma once

#include <juce\_audio\_processors/juce\_audio\_processors.h>

namespace AvalancheParams {

inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout() { std::vector<std::unique\_ptrjuce::RangedAudioParameter\> params;

```
// --- Delay Section ---
params.push_back(std::make_unique<juce::AudioParameterFloat>(
    juce::ParameterID{"delay_time", 1}, "Time",
    juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.3f));

params.push_back(std::make_unique<juce::AudioParameterFloat>(
    juce::ParameterID{"delay_feedback", 1}, "Repeats",
    juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.4f));

params.push_back(std::make_unique<juce::AudioParameterFloat>(
    juce::ParameterID{"delay_tone", 1}, "Tone",
    juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.5f));

params.push_back(std::make_unique<juce::AudioParameterFloat>(
    juce::ParameterID{"delay_mix", 1}, "D.Mix",
    juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.5f));

// --- Reverb Section ---
params.push_back(std::make_unique<juce::AudioParameterFloat>(
    juce::ParameterID{"reverb_decay", 1}, "Decay",
    juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.5f));

params.push_back(std::make_unique<juce::AudioParameterFloat>(
    juce::ParameterID{"reverb_damping", 1}, "Damping",
    juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.7f));

params.push_back(std::make_unique<juce::AudioParameterFloat>(
    juce::ParameterID{"reverb_mix", 1}, "R.Mix",
    juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.5f));

// --- Shared ---
params.push_back(std::make_unique<juce::AudioParameterFloat>(
    juce::ParameterID{"diffusion", 1}, "Diffusion",
    juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.625f));

params.push_back(std::make_unique<juce::AudioParameterFloat>(
    juce::ParameterID{"cross_feed", 1}, "Cross Feed",
    juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.3f));

params.push_back(std::make_unique<juce::AudioParameterFloat>(
    juce::ParameterID{"mod_depth", 1}, "Mod Depth",
    juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.5f));

// --- Mode ---
params.push_back(std::make_unique<juce::AudioParameterChoice>(
    juce::ParameterID{"mode", 1}, "Mode",
    juce::StringArray{"Normal", "Reverse", "Swell"}, 0));

return { params.begin(), params.end() };
```

}

} // namespace AvalancheParams