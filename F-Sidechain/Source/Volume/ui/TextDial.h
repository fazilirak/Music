// Geliştirici: Fazıl Irak - Son düzenleme: 27.10.2026
// Dosyanın görevi: Envelope düzenleme ve ürün kontrol bileşeni; özgün kaynak atıfları korunur.
// Katkı kapsamı: F-Sidechain bakımı ve uyarlamaları. GATE-12/Shaper özgün telifleri korunur.
// Fazıl Irak tarafından değiştirildi: 23.10.2026; Shaper uyarlaması.
// Özgün telif ve lisans bildirimleri aşağıda korunur.
#include <JuceHeader.h>
#include <gate12_data.h>
#pragma once

#include <JuceHeader.h>
#include "../Globals.h"

namespace volumef {

using namespace globals;
class VOLUMEFAudioProcessor;

enum TextDialLabel {
    tdPercx100,
};

class TextDial : public juce::SettableTooltipClient, public juce::Component, private juce::AudioProcessorValueTreeState::Listener {
public:
    TextDial(VOLUMEFAudioProcessor& p, String paramId, String prefix, String suffix, TextDialLabel format, float fontSize, unsigned int color);
    ~TextDial() override;
    void parameterChanged (const juce::String& parameterID, float newValue) override;

    void paint(juce::Graphics& g) override;

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseDoubleClick(const juce::MouseEvent& e) override;
    void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;


private:
    String paramId;
    String prefix;
    String suffix;
    TextDialLabel format;
    float fontSize = 16.0f;
    unsigned int fontColor;
    VOLUMEFAudioProcessor& audioProcessor;

    float pixels_per_percent{100.0f};
    float cur_normed_value{0.0f};
    juce::Point<int> last_mouse_position;
    bool mouse_down = false;
};
} // namespace volumef
