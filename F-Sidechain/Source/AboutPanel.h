// Geliştirici: Fazıl Irak - Son düzenleme: 27.10.2026
// Dosyanın görevi: Çevrimdışı geliştirici bilgisi, kaynak atıfları ve tam lisans okuyucusu.
// Katkı kapsamı: F-Sidechain bakımı ve uyarlamaları. GATE-12/Shaper özgün telifleri korunur.
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
