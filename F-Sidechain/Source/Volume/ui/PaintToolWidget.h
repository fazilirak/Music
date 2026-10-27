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

class PaintToolWidget : public juce::Component, private juce::Timer {
public:
    PaintToolWidget(VOLUMEFAudioProcessor& p);
    ~PaintToolWidget() override {}

    TextButton paintEditButton;
    TextButton paintNextButton;
    TextButton paintPrevButton;
    Label paintPageLabel;

    void toggleUIComponents();
    void timerCallback() override;
    void paint(Graphics& g) override;
    void resized() override;
    void drawPattern(Graphics& g, Rectangle<int> bounds, int index, Colour color);
    void mouseDown(const juce::MouseEvent& e) override;
    std::vector<Rectangle<int>> getPatRects();

private:
    VOLUMEFAudioProcessor& audioProcessor;
};
} // namespace volumef
