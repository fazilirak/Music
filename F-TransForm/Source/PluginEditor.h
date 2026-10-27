// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (c) 2026 Fazıl Irak
// Geliştirici: Fazıl Irak | Oluşturma / son düzenleme: 27.10.2026
// Dosyanın görevi: Üç knob, bypass, gömülü tipografi ve bilgi alanlarının arayüz tanımı.
#pragma once
#include "PluginProcessor.h"
#include "AboutPanel.h"
namespace ft {
class Editor final : public juce::AudioProcessorEditor,private juce::Timer {
public:
 explicit Editor(Processor&);
 ~Editor() override;
 void paint(juce::Graphics&) override;
 void resized() override;
private:
 struct Theme : juce::LookAndFeel_V4 {
   Theme();
   juce::Typeface::Ptr book,demi;
   juce::Typeface::Ptr getTypefaceForFont(const juce::Font& f) override {return f.isBold()?demi:book;}
   void drawRotarySlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider&) override;
 } theme;
 // Attachment'lar slider'lardan önce yok edilmelidir; aşağıdaki üye sırası bunu sağlar.
 Processor& processor;
 juce::Slider technique,intensity,character;
 juce::TextButton bypass{"BYPASS"};
 ProductLogo logo;
 AboutPanel about;
 juce::TooltipWindow tooltip{this,600};
 std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> techniqueLink,intensityLink,characterLink;
 std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassLink;
 // Timer yalnız görüntüyü yeniler; ses ve otomasyon processor'da çalışır.
 void timerCallback() override {repaint();}
 void select(int index);
 void mouseDown(const juce::MouseEvent&) override;
};
}
