// Fazıl Irak tarafından değiştirildi: 23.10.2026; Shaper uyarlaması.
// Özgün telif ve lisans bildirimleri aşağıda korunur.
#include <JuceHeader.h>
#include <gate12_data.h>
/*
  ==============================================================================

    AudioDisplay.h
    Author:  tiagolr

    A wave display for audio buffer with markers on detected transients

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "../dsp/Pattern.h"
#include <deque>

namespace volumef {

class VOLUMEFAudioProcessor;

class AudioDisplay : public juce::Component, private juce::Timer
{
public:
    AudioDisplay(VOLUMEFAudioProcessor&);
    ~AudioDisplay() override {};
    void resized() override;
    void timerCallback() override;
    void paint(Graphics& g) override;
    
    std::deque<double> audioBuffer;
    std::deque<bool> hitBuffer; 
    VOLUMEFAudioProcessor& audioProcessor;
};
} // namespace volumef
