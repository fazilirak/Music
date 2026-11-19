// SPDX-License-Identifier: AGPL-3.0-or-later
// Fazıl Irak · 03.11.2026: kullanıcının istediği üç küçük knob; mevcut 912 × 511 panel korunur.
#pragma once
#include "PluginProcessor.h"
#include <BinaryData.h>
class FKickTransientKnob final : public juce::Component {
    struct Theme : juce::LookAndFeel_V4 {
        juce::Typeface::Ptr face=juce::Typeface::createSystemTypefaceFor(fkick_data::UbuntuMedium_ttf,fkick_data::UbuntuMedium_ttfSize);
        juce::Typeface::Ptr getTypefaceForFont(const juce::Font&) override {return face;}
        juce::Font getLabelFont(juce::Label&) override {return juce::FontOptions(7.8f);}
        void drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float value,float start,float end,juce::Slider&) override {
            const float cx=x+w*.5f,cy=y+h*.5f,r=juce::jmin(w,h)*.42f;
            g.setColour(juce::Colour(0xff343f42));g.drawEllipse(cx-r,cy-r,2*r,2*r,1.2f);
            juce::Path arc;const float angle=start+(end-start)*value;
            arc.addCentredArc(cx,cy,r,r,0,start,angle,true);
            g.setColour(juce::Colour(0xffed6c46));g.strokePath(arc,juce::PathStrokeType(1.35f));
            g.setColour(juce::Colour(0xff272f31));g.fillEllipse(cx-r+2,cy-r+2,2*r-4,2*r-4);
            g.setColour(juce::Colour(0xffe5e7df));g.drawLine(cx+std::sin(angle)*r*.25f,cy-std::cos(angle)*r*.25f,cx+std::sin(angle)*r*.72f,cy-std::cos(angle)*r*.72f,1.2f);
        }
    } theme;
    juce::String caption;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
public:
    juce::Slider slider;
    FKickTransientKnob(FKickProcessor& p,const char* id,const char* name,const char* help):caption(name) {
        setComponentID(id);slider.setComponentID(juce::String(id)+"-slider");slider.setName(name);
        slider.setLookAndFeel(&theme);slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        slider.setRotaryParameters(-2.27f,2.27f,true);slider.setTextBoxStyle(juce::Slider::TextBoxBelow,false,30,11);
        slider.setDoubleClickReturnValue(true,0);slider.setTooltip(help);
        slider.setColour(juce::Slider::textBoxTextColourId,juce::Colour(0xffe5e7df));
        slider.setColour(juce::Slider::textBoxBackgroundColourId,juce::Colours::transparentBlack);
        slider.setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);
        addAndMakeVisible(slider);attachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.state,id,slider);
        // Fazıl Irak · 03.11.2026: attachment parametre biçimini kurduktan sonra okunaklı yüzde gösterimi uygulanır.
        slider.textFromValueFunction=[](double v){return juce::String(juce::roundToInt(v))+"%";};
        slider.valueFromTextFunction=[](const juce::String& t){return t.getDoubleValue();};
        slider.setRange(slider.getMinimum(),slider.getMaximum(),1);
        slider.updateText();
    }
    ~FKickTransientKnob() override {attachment.reset();slider.setLookAndFeel(nullptr);}
    void resized() override {slider.setBounds(1,10,getWidth()-2,getHeight()-10);}
    void paint(juce::Graphics& g) override {g.setColour(juce::Colour(0xff8d9799));g.setFont(juce::FontOptions(7.3f));g.drawText(caption,0,0,getWidth(),10,juce::Justification::centred);}
};
