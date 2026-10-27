// Geliştirici: Fazıl Irak - Son düzenleme: 27.10.2026
// Dosyanın görevi: Envelope düzenleme ve ürün kontrol bileşeni; özgün kaynak atıfları korunur.
// Katkı kapsamı: F-Sidechain bakımı ve uyarlamaları. GATE-12/Shaper özgün telifleri korunur.
// Fazıl Irak tarafından değiştirildi: 23.10.2026; Shaper uyarlaması.
// Özgün telif ve lisans bildirimleri aşağıda korunur.
#include <JuceHeader.h>
#include <gate12_data.h>
#pragma once

#include <JuceHeader.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "../Globals.h"

namespace volumef {

using namespace globals;

class VOLUMEFAudioProcessor;

class GridSelector : public juce::SettableTooltipClient, public juce::Component, private juce::AudioProcessorValueTreeState::Listener {
public:
    GridSelector(VOLUMEFAudioProcessor&, bool seqStepSelector = false);
    ~GridSelector() override;
    void paint(juce::Graphics& g) override;

    void mouseDown(const juce::MouseEvent& e) override;

    void parameterChanged (const juce::String& parameterID, float newValue) override;

private:
    bool seqStepSelector;
    VOLUMEFAudioProcessor& audioProcessor;
};
} // namespace volumef
