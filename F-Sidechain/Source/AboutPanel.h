// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 Fazıl Irak
// Fazıl Irak — 26.10.2026: Logodan açılan çevrimdışı About ve lisans okuyucusu.
#pragma once
#include <JuceHeader.h>

class AboutPanel : public juce::Component
{
public:
    AboutPanel();
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    juce::TextButton close{"Close"};
    juce::ComboBox documents;
    juce::TextEditor text;
    void selectDocument();
};
