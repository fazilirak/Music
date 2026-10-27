// Geliştirici: Fazıl Irak - Son düzenleme: 27.10.2026
// Dosyanın görevi: Kompakt İngilizce ürün arayüzü, kontrol bağlantıları ve kullanıcı etkileşimi.
// Copyright (c) 2026 Fazıl Irak - Özgün F-Kick uygulama kodu.
// SPDX-License-Identifier: AGPL-3.0-or-later
// Fazıl Irak · 27.10.2026 · 760 x 360 cihaz paneli; sürüklenebilir metinler, sade logo ve gerçek preset gezinmesi.
#pragma once
#include "PluginProcessor.h"
#include "NativeFileDrop.h"

class FKickTheme final : public juce::LookAndFeel_V4 {
public:
    FKickTheme();
    juce::Typeface::Ptr getTypefaceForFont(const juce::Font&) override {return typeface;}
    juce::Font getTextButtonFont(juce::TextButton&,int) override {return juce::FontOptions(11.f);}
    juce::Font getComboBoxFont(juce::ComboBox&) override {return juce::FontOptions(12.f);}
    void drawButtonBackground(juce::Graphics&,juce::Button&,const juce::Colour&,bool,bool) override;
    void drawComboBox(juce::Graphics&,int,int,bool,int,int,int,int,juce::ComboBox&) override;
private:
    juce::Typeface::Ptr typeface;
};

// Logo bir Button değildir. Arka plan/çerçeve çizilmez; tıklama ve klavye erişimi About'u açar.
class FKickLogo final : public juce::Component,public juce::SettableTooltipClient {
public:
    FKickLogo();
    void paint(juce::Graphics&) override;
    void mouseUp(const juce::MouseEvent&) override;
    bool keyPressed(const juce::KeyPress&) override;
    std::function<void()> onClick;
};

// Parametre kimlikleri/DSP aynıdır; knob yerine dikey sürükleme, wheel ve çift tıkla sayı girişi.
class FKickValue final : public juce::Component,public juce::SettableTooltipClient {
public:
    // Wheel mouse listener'dan işlenir; Label'ın parent forwarding'i çift adım üretmesin.
    class NumberLabel final : public juce::Label {void mouseWheelMove(const juce::MouseEvent&,const juce::MouseWheelDetails&) override {}};
    FKickValue(FKickProcessor&,const char* id,const char* caption,const char* help);
    ~FKickValue() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;
    void mouseWheelMove(const juce::MouseEvent&,const juce::MouseWheelDetails&) override;
    NumberLabel number;
    void refreshDisplay(){refresh();}
    void setInlineCaption(int width){inlineWidth=width;resized();repaint();}
private:
    FKickProcessor& processor;
    juce::RangedAudioParameter& parameter;
    juce::String id,caption;
    float plain=0,dragValue=0;
    int dragY=0;
    bool dragging=false;
    int inlineWidth=0;
    juce::ParameterAttachment attachment;
    juce::String formatted(float) const;
    float parsed(const juce::String&) const;
    void refresh();
};

class FKickSidechainGraph final : public juce::Component {
public:
    explicit FKickSidechainGraph(FKickProcessor&);
    ~FKickSidechainGraph() override;
    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;
    void mouseDoubleClick(const juce::MouseEvent&) override;
private:
    FKickProcessor& processor;
    juce::ParameterAttachment shift;
    float startShift=0,startX=0;
    bool dragging=false;
};

class FKickGraph final : public juce::Component {
public:
    explicit FKickGraph(FKickProcessor& p):processor(p) {setComponentID("envelope-editor");setMouseCursor(juce::MouseCursor::CrosshairCursor);}
    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;
    void mouseDoubleClick(const juce::MouseEvent&) override;
    int tab=1; // 0 waveform/trim, 1 pitch, 2 amplitude.
    juce::Rectangle<float> plotBounds() const {return plot();}
private:
    FKickProcessor& processor;
    int selected=-1,trim=-1;
    float bendStart=0;
    juce::Point<float> dragOrigin;
    juce::Rectangle<float> plot() const;
    juce::Point<float> point(const fkick::Curve&,int) const;
    int hit(juce::Point<float>) const;
};

// Gerçek sample paneli bir native dosya bırakma hedefidir; çocuk dalga/etiketlerin üzerinden de bulunur.
class FKickSampleDropArea final : public juce::Component, public juce::FileDragAndDropTarget {
public:
    FKickSampleDropArea() {setComponentID("sample-drop-area");}
    bool isInterestedInFileDrag(const juce::StringArray&) override;
    void fileDragEnter(const juce::StringArray&,int,int) override;
    void fileDragMove(const juce::StringArray&,int,int) override;
    void fileDragExit(const juce::StringArray&) override;
    void filesDropped(const juce::StringArray&,int,int) override;
    void paintOverChildren(juce::Graphics&) override;
    std::function<void(const juce::File&)> onSampleDropped;
private:
    bool hovering=false;
};

class FKickAbout final : public juce::Component {
public:
    FKickAbout();
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    juce::TextButton close{"Close"};
    juce::ComboBox documents;
    juce::TextEditor text;
};

class FKickEditor final : public juce::AudioProcessorEditor,private juce::Timer,public juce::FileDragAndDropTarget {
public:
    explicit FKickEditor(FKickProcessor&);
    ~FKickEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    bool isInterestedInFileDrag(const juce::StringArray&) override;
    void filesDropped(const juce::StringArray&,int,int) override;
    bool testNativeDrop(const juce::StringArray& files,juce::Point<int> p){nativeDrop.attach(*this);return nativeDrop.testNativeTransfer(files,p);}
private:
    FKickProcessor& processor;
    FKickTheme theme;
    juce::TooltipWindow tooltips{this,600};
    FKickLogo logo;
    juce::TextButton previousPreset{"<"},nextPreset{">"},save{"Save"},load{"Load"},options{"..."};
    juce::TextButton openSample{"Drop / load sample"},play{juce::String::fromUTF8("▶")},exportButton{"WAV"},panic{juce::String::fromUTF8("■")};
    juce::TextButton kickMode{"Kick"},samplerMode{"Sampler"};
    juce::TextButton gate{"Trigger"},poly{"Mono"},loop{"Loop"},reverse{"Reverse"},tracking{"Track tail"};
    juce::TextButton sampleTab{"Sample"},pitchTab{"Pitch"},ampTab{"Amp"};
    juce::TextButton soundPage{"Sound"},fxPage{"FX"},modPage{"Mod"},echoSync{"Sync"};
    std::array<juce::TextButton,4> scPresets;
    juce::ComboBox presets;
    juce::ComboBox echoDivision,echoSource,driveType;
    std::vector<std::unique_ptr<FKickValue>> values;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> buttons;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> choices;
    std::vector<std::unique_ptr<FKickValue>> fxValues;
    FKickValue hpValue;
    FKickValue bodyPunch;
    FKickValue tempoValue;
    FKickValue scMixValue,scShiftValue,roomMixValue,fxHPValue,fxLPValue,bassLiftValue,bassQValue;
    FKickSampleDropArea samplePanel;
    FKickGraph graph;
    FKickSidechainGraph sidechainGraph;
    FKickAbout about;
    NativeFileDrop nativeDrop;
    std::unique_ptr<juce::FileChooser> chooser;
    float peak=0;
    int shownProgram=-1;
    juce::String shownSample;
    int page=0;
    void timerCallback() override;
    void updateControls();
    void stepPreset(int);
    void chooseFile(int operation);
    void showOptions();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FKickEditor)
};
