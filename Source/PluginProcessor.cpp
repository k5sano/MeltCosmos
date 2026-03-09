```cpp
Copy#include "PluginProcessor.h"
#include "PluginEditor.h"

EMSpacePlugin::EMSpacePlugin()
    : AudioProcessor(BusesProperties()
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMETERS", EMSpaceParams::createLayout()),
      presetManager(apvts)
{
    delayTimeParam     = apvts.getRawParameterValue("delay_time");
    delayFeedbackParam = apvts.getRawParameterValue("delay_feedback");
    delayToneParam     = apvts.getRawParameterValue("delay_tone");
    delayMixParam      = apvts.getRawParameterValue("delay_mix");
    reverbDecayParam   = apvts.getRawParameterValue("reverb_decay");
    reverbDampParam    = apvts.getRawParameterValue("reverb_damping");
    reverbMixParam     = apvts.getRawParameterValue("reverb_mix");
    diffusionParam     = apvts.getRawParameterValue("diffusion");
    crossFeedParam     = apvts.getRawParameterValue("cross_feed");
    modSpeedParam      = apvts.getRawParameterValue("mod_speed");
    modDepthParam      = apvts.getRawParameterValue("mod_depth");
}

void EMSpacePlugin::prepareToPlay(double sampleRate, int)
{
    engine_.prepare(sampleRate);
    levelMeter.reset();
}

void EMSpacePlugin::releaseResources()
{
    engine_.clear();
}

void EMSpacePlugin::processBlock(juce::AudioBuffer<float>& buffer,
                                  juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numCh = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    if (numCh < 2 || numSamples == 0) return;

    float* L = buffer.getWritePointer(0);
    float* R = buffer.getWritePointer(1);

    // Input level
    levelMeter.measureInput(L, R, numSamples);

    // Update DSP params
    engine_.setDelayTime(delayTimeParam->load());
    engine_.setDelayFeedback(delayFeedbackParam->load());
    engine_.setDelayTone(delayToneParam->load());
    engine_.setDelayMix(delayMixParam->load());
    engine_.setReverbDecay(reverbDecayParam->load());
    engine_.setReverbDamping(reverbDampParam->load());
    engine_.setReverbMix(reverbMixParam->load());
    engine_.setDiffusion(diffusionParam->load());
    engine_.setCrossFeed(crossFeedParam->load());
    engine_.setModSpeed(modSpeedParam->load());
    engine_.setModDepth(modDepthParam->load());

    // DSP
    engine_.process(L, R, numSamples);

    // Output level
    levelMeter.measureOutput(L, R, numSamples);
}

juce::AudioProcessorEditor* EMSpacePlugin::createEditor()
{
    return new EMSpaceEditor(*this);
}

void EMSpacePlugin::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void EMSpacePlugin::setStateInformation(const void* data,
                                         int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(
        getXmlFromBinary(data, sizeInBytes));
    if (xml && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EMSpacePlugin();
}
Copy
```