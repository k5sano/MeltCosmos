#include "PluginEditor.h"

AvalancheEditor::AvalancheEditor(AvalanchePlugin& p)

AudioProcessorEditor(p), processor(p), genericEditor(p) { addAndMakeVisible(genericEditor); setSize(600, 580); startTimerHz(30); }

AvalancheEditor::~AvalancheEditor() { stopTimer(); }

void AvalancheEditor::timerCallback() { auto& eng = processor.getEngine();

```
LevelMeter* meterPtrs[kNumMeters] = {
    &eng.meterInput,
    &eng.meterDelayOut,
    &eng.meterReverbIn,
    &eng.meterReverbOut,
    &eng.meterCrossFeed,
    &eng.meterOutput
};

constexpr float kHoldTime = 1.5f;    // seconds
constexpr float kDecayRate = 15.0f;   // dB/sec
constexpr float kDt = 1.0f / 30.0f;  // timer interval

for (int i = 0; i < kNumMeters; ++i)
{
    auto levels = meterPtrs[i]->getAndReset();
    meters_[i].peakDb = levels.peakDb;
    meters_[i].rmsDb  = levels.rmsDb;

    // Peak hold logic
    if (levels.peakDb > meters_[i].peakHold)
    {
        meters_[i].peakHold = levels.peakDb;
        meters_[i].holdTimer = kHoldTime;
    }
    else
    {
        meters_[i].holdTimer -= kDt;
        if (meters_[i].holdTimer <= 0.0f)
        {
            meters_[i].peakHold -= kDecayRate * kDt;
            if (meters_[i].peakHold < -100.0f)
                meters_[i].peakHold = -100.0f;
        }
    }
}

repaint(0, 400, getWidth(), 180);
```

}

juce::Colour AvalancheEditor::levelToColour(float db) { if (db > -3.0f) return juce::Colour(0xFFFF4444); if (db > -12.0f) return juce::Colour(0xFFFFCC00); return juce::Colour(0xFF44DD66); }

float AvalancheEditor::dbToX(float db, float width) { // Range: -60 to +6 dB constexpr float minDb = -60.0f; constexpr float maxDb = 6.0f; float clamped = std::max(minDb, std::min(db, maxDb)); return (clamped - minDb) / (maxDb - minDb) \* width; }

void AvalancheEditor::drawMeter(juce::Graphics& g, juce::Rectangle bounds, const char\* label, const MeterState& state) { int labelW = 60; int meterX = bounds.getX() + labelW; int meterW = bounds.getWidth() - labelW - 4; int meterY = bounds.getY() + 2; int meterH = bounds.getHeight() - 4;

```
// Label
g.setColour(juce::Colour(0xFFCCCCCC));
auto font = juce::Font(
    juce::FontOptions{}.withHeight(12.0f));
g.setFont(font);
g.drawText(label,
           bounds.getX(), bounds.getY(),
           labelW, bounds.getHeight(),
           juce::Justification::centredRight);

// Background
g.setColour(juce::Colour(0xFF222222));
g.fillRect(meterX, meterY, meterW, meterH);

// RMS bar
float rmsW = dbToX(state.rmsDb, static_cast<float>(meterW));
if (rmsW > 0.0f)
{
    g.setColour(levelToColour(state.rmsDb).withAlpha(0.7f));
    g.fillRect(meterX, meterY,
               static_cast<int>(rmsW), meterH);
}

// Peak bar (thin)
float peakX = dbToX(state.peakDb, static_cast<float>(meterW));
if (peakX > 0.0f)
{
    g.setColour(levelToColour(state.peakDb));
    g.fillRect(meterX + static_cast<int>(peakX) - 1,
               meterY, 2, meterH);
}

// Peak hold line (white)
float holdX = dbToX(state.peakHold, static_cast<float>(meterW));
if (holdX > 0.0f)
{
    g.setColour(juce::Colours::white);
    g.fillRect(meterX + static_cast<int>(holdX) - 1,
               meterY, 2, meterH);
}

// dB scale markers
g.setColour(juce::Colour(0xFF666666));
for (float db : {-48.0f, -36.0f, -24.0f, -12.0f,
                 -6.0f, -3.0f, 0.0f})
{
    int x = meterX
        + static_cast<int>(dbToX(db,
            static_cast<float>(meterW)));
    g.drawVerticalLine(x, static_cast<float>(meterY),
                       static_cast<float>(meterY + meterH));
}
```

}

void AvalancheEditor::paint(juce::Graphics& g) { g.fillAll(juce::Colour(0xFF1a1a2e));

```
// Signal flow title
auto titleFont = juce::Font(
    juce::FontOptions{}.withHeight(14.0f));
g.setFont(titleFont);
g.setColour(juce::Colour(0xFF88AADD));
g.drawText("Signal Flow Meters",
           10, 405, getWidth() - 20, 18,
           juce::Justification::centredLeft);

// Signal flow arrows
g.setColour(juce::Colour(0xFF555577));
auto arrowFont = juce::Font(
    juce::FontOptions{}.withHeight(11.0f));
g.setFont(arrowFont);
g.drawText(
    "IN -> [DELAY] -> [REVERB] -> OUT  |  "
    "REVERB tail -> X-FEED -> DELAY feedback",
    10, 420, getWidth() - 20, 14,
    juce::Justification::centredLeft);

// Draw meters
int meterAreaY = 440;
int meterH = 20;
int spacing = 2;

for (int i = 0; i < kNumMeters; ++i)
{
    juce::Rectangle<int> bounds(
        10, meterAreaY + i * (meterH + spacing),
        getWidth() - 20, meterH);
    drawMeter(g, bounds, meterNames_[i], meters_[i]);
}
```

}

void AvalancheEditor::resized() { // Parameters area: top 400px genericEditor.setBounds(0, 0, getWidth(), 400); // Meters area: bottom 180px (drawn in paint) }