// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (c) 2026 Fazıl Irak
// Geliştirici: Fazıl Irak | Son düzenleme: 26.10.2026
// Alt sıranın tek parametreli düğmesi: DAW otomasyonu, sayısal giriş ve çift tıkla sıfırlama.
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
namespace irak
{
class DeskKnob final : public juce::Component
{
    juce::Label title;
    juce::Slider slider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
  public:
    DeskKnob(juce::AudioProcessorValueTreeState& state,const char* id,const char* heading,const char* tooltip)
    {
        setComponentID("desk-"+juce::String(id));
        title.setText(heading,juce::dontSendNotification);
        title.setJustificationType(juce::Justification::centred);
        title.setFont(juce::FontOptions(10.5f));
        addAndMakeVisible(title);addAndMakeVisible(slider);
        slider.setComponentID("desk-slider-"+juce::String(id));
        slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow,false,78,16);
        // Fazıl Irak: küçük düğmelerde kutu çerçevesi yerine sade değer satırı, eski arayüzün görsel ağırlığını korur.
        slider.setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);
        slider.setColour(juce::Slider::textBoxBackgroundColourId,juce::Colours::transparentBlack);
        slider.setDoubleClickReturnValue(true,0);
        slider.setTooltip(tooltip);
        attachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state,id,slider);
        // Attachment varsayılan metin dönüştürücüsünü kurduktan sonra fiziksel birim gösterimini uygula.
        const bool db=juce::String(id)=="bass"||juce::String(id)=="treble";
        slider.textFromValueFunction=[db](double v){return db?juce::String(v,1)+" dB":juce::String(juce::roundToInt(v*100))+"%";};
        slider.valueFromTextFunction=[db](const juce::String& text){return text.getDoubleValue()/(db?1:100);};
        slider.updateText();
    }
    void resized() override
    {
        title.setBounds(0,0,getWidth(),14);
        slider.setBounds(2,14,getWidth()-4,getHeight()-14);
    }
};
} // namespace irak
