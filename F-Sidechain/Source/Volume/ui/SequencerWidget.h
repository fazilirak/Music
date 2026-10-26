// Fazıl Irak tarafından değiştirildi: 23.10.2026; Shaper uyarlaması.
// Özgün telif ve lisans bildirimleri aşağıda korunur.
#include <JuceHeader.h>
#include <gate12_data.h>
#pragma once

#include <JuceHeader.h>
#include "../Globals.h"
#include "GridSelector.h"

namespace volumef {

using namespace globals;
class VOLUMEFAudioProcessor;

class SequencerWidget : public juce::Component {
public:
    SequencerWidget(VOLUMEFAudioProcessor& p);
    ~SequencerWidget() override {}
    void resized() override;

    std::unique_ptr<GridSelector> stepSelector;

    TextButton maxBtn;
    TextButton tenBtn;
    TextButton skewBtn;
    TextButton flipXBtn;

    TextButton silenceBtn;
    TextButton rampupBtn;
    TextButton rampdnBtn;
    TextButton lineBtn;
    TextButton sineBtn;
    TextButton triBtn;
    TextButton ptoolBtn;

    TextButton randomBtn;
    TextButton randomMenuBtn;
    Slider randomRange;
    TextButton clearBtn;

    TextButton applyBtn;
    TextButton resetBtn;
    TextButton linkStepBtn;

    double randomMin = 0.0;
    double randomMax = 1.0;

    void updateButtonsState();
    void paint(Graphics& g) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void drawChain(Graphics& g, Rectangle<int> bounds, Colour color, Colour background);

private:
    VOLUMEFAudioProcessor& audioProcessor;
};
} // namespace volumef
