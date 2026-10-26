// Fazıl Irak tarafından değiştirildi: 23.10.2026; Shaper uyarlaması.
// Özgün telif ve lisans bildirimleri aşağıda korunur.
#include <JuceHeader.h>
#include <gate12_data.h>
#pragma once

#include <JuceHeader.h>
#include "../Globals.h"

namespace volumef {

using namespace globals;
class RipplerXAudioProcessor;

class About : public juce::Component {
public:
    About() {}
    ~About() override {}

    void mouseDown(const juce::MouseEvent& e) override;
    void paint(Graphics& g) override;
};
} // namespace volumef
