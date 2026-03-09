#include "PluginEditor.h"

AvalancheEditor::AvalancheEditor(AvalanchePlugin& p)

AudioProcessorEditor(p), processor(p), genericEditor(p) { addAndMakeVisible(genericEditor); setSize(500, 400); }

void AvalancheEditor::paint(juce::Graphics& g) { g.fillAll(juce::Colour(0xFF1a1a2e)); }

void AvalancheEditor::resized() { genericEditor.setBounds(getLocalBounds()); }