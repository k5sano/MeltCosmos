#include "PluginProcessor.h" #include "PluginEditor.h" #include

AvalanchePlugin::AvalanchePlugin()

AudioProcessor(BusesProperties() .withInput("Input", juce::AudioChannelSet::stereo(), true) .withOutput("Output", juce::AudioChannelSet::stereo(), true)), apvts(\*this, nullptr, "PARAMETERS", AvalancheParams::createLayout()) { delayTimeParam = apvts.getRawParameterValue("delay\_time"); delayFeedbackParam = apvts.getRawParameterValue("delay\_feedback"); delayToneParam = apvts.getRawParameterValue("delay\_tone"); delayMixParam = apvts.getRawParameterValue("delay\_mix"); reverbDecayParam = apvts.getRawParameterValue("reverb\_decay"); reverbDampingParam = apvts.getRawParameterValue("reverb\_damping"); reverbMixParam = apvts.getRawParameterValue("reverb\_mix"); diffusionParam = apvts.getRawParameterValue("diffusion"); crossFeedParam = apvts.getRawParameterValue("cross\_feed"); modDepthParam = apvts.getRawParameterValue("mod\_depth"); modeParam = apvts.getRawParameterValue("mode"); }

void AvalanchePlugin::prepareToPlay(double sampleRate, int samplesPerBlock) { engine\_.init(); adapter\_.prepare(sampleRate, samplesPerBlock); dryBuffer\_.setSize(2, samplesPerBlock); }

void AvalanchePlugin::releaseResources() { engine\_.clear(); }

void AvalanchePlugin::processBlock(juce::AudioBuffer& buffer, juce::MidiBuffer&) { juce::ScopedNoDenormals noDenormals;

```
const int numChannels = buffer.getNumChannels();
const int numSamples  = buffer.getNumSamples();

if (numChannels < 2 || numSamples == 0)
    return;

// Read all parameters
engine_.setDelayTime(delayTimeParam->load());
engine_.setDelayFeedback(delayFeedbackParam->load());
engine_.setDelayTone(delayToneParam->load());
engine_.setDelayMix(delayMixParam->load());
engine_.setReverbDecay(reverbDecayParam->load());
engine_.setReverbDamping(reverbDampingParam->load());
engine_.setReverbMix(reverbMixParam->load());
engine_.setDiffusion(diffusionParam->load());
engine_.setCrossFeed(crossFeedParam->load());
engine_.setModDepth(modDepthParam->load());
engine_.setMode(static_cast<int>(modeParam->load()));

dryBuffer_.setSize(2, numSamples, false, false, true);
for (int ch = 0; ch < 2; ++ch)
    dryBuffer_.copyFrom(ch, 0, buffer, ch, 0, numSamples);

float* outL = buffer.getWritePointer(0);
float* outR = buffer.getWritePointer(1);
const float* inL = dryBuffer_.getReadPointer(0);
const float* inR = dryBuffer_.getReadPointer(1);

adapter_.process(inL, inR, outL, outR, numSamples, engine_);
```

}

juce::AudioProcessorEditor\* AvalanchePlugin::createEditor() { return new AvalancheEditor(\*this); }

void AvalanchePlugin::getStateInformation(juce::MemoryBlock& destData) { auto state = apvts.copyState(); std::unique\_ptrjuce::XmlElement xml(state.createXml()); copyXmlToBinary(\*xml, destData); }

void AvalanchePlugin::setStateInformation(const void\* data, int sizeInBytes) { std::unique\_ptrjuce::XmlElement xml( getXmlFromBinary(data, sizeInBytes)); if (xml != nullptr && xml->hasTagName(apvts.state.getType())) apvts.replaceState(juce::ValueTree::fromXml(\*xml)); }

juce::AudioProcessor\* JUCE\_CALLTYPE createPluginFilter() { return new AvalanchePlugin(); }