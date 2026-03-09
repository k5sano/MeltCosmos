#pragma once

#include <juce\_audio\_processors/juce\_audio\_processors.h> #include "PluginProcessor.h"

class AvalancheEditor : public juce::AudioProcessorEditor { public: explicit AvalancheEditor(AvalanchePlugin&); ~AvalancheEditor() override = default;

```
void paint(juce::Graphics&) override;
void resized() override;
```

private: AvalanchePlugin& processor; juce::GenericAudioProcessorEditor genericEditor;

```
JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AvalancheEditor)
```

};