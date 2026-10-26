// Fazıl Irak tarafından değiştirildi: 23.10.2026; Shaper uyarlaması.
// Özgün telif ve lisans bildirimleri aşağıda korunur.
#include <JuceHeader.h>
#include <gate12_data.h>
#pragma once

#include <JuceHeader.h>
#include "../Globals.h"

namespace volumef {

using namespace globals;

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel();
    juce::Typeface::Ptr getTypefaceForFont(const juce::Font&) override;
    int getPopupMenuBorderSize() override;
    void drawButtonBackground (Graphics& g, Button& button, const Colour& backgroundColour, bool isMouseOverButton, bool isButtonDown) override;
    void drawComboBox (Graphics&, int width, int height, bool isButtonDown,int buttonX, int buttonY, int buttonW, int buttonH, ComboBox&) override;
    void positionComboBoxText (ComboBox& box, Label& label) override;
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos, float minSliderPos, float maxSliderPos,
        const juce::Slider::SliderStyle style, juce::Slider& slider) override;


    juce::Font getTextButtonFont(juce::TextButton& button,int buttonHeight) override {

        const auto id=button.getComponentID();
        if(id.startsWith("effect-")||id=="snap-outline"||id=="pattern"||id=="leftPattern"||id=="rightPattern") return juce::FontOptions(13.0f);
 return juce::LookAndFeel_V4::getTextButtonFont(button,buttonHeight); }
    juce::Font getComboBoxFont(juce::ComboBox& box) override {

        if(box.getComponentID().startsWith("effect-")) return juce::FontOptions(14.0f);
 return juce::LookAndFeel_V4::getComboBoxFont(box); }
private:
    juce::Typeface::Ptr typeface;
};
} // namespace volumef
