// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 Fazıl Irak; Shaper eklemeleri/değişiklikleri.
// Fazıl Irak tarafından oluşturuldu / değiştirildi: 24.10.2026
// Kapsam: Shaper için eklenen kod ve uyarlamalar; özgün üçüncü taraf telif/lisansları korunur.
// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 24.10.2026
// Dosyanın görevi: Ana rack editörünü, efekt sırasını, seçimlerini ve mevcut ortak kontrol yerleşimini yönetir.
#pragma once
#include "Processor.h"
#include "Embedded/RackEditorCommands.h"
#include "EffectPalette.h"
#include "AboutPanel.h"
#include "IconButton.h"
struct RackLookAndFeel final : juce::LookAndFeel_V4
{
    // getTextButtonFont: Ana rack editörünü, efekt sırasını, seçimlerini ve mevcut ortak kontrol yerleşimini yönetir. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
    juce::Font getTextButtonFont(juce::TextButton &, int) override
    {
        return juce::FontOptions(14.0f);
    }
};
// Fazıl Irak - 28.10.2026: Sade, erişilebilir logo. Host/state kimliği değişmez.
class JellyboneLogo final : public juce::Component, public juce::SettableTooltipClient {
public:
    JellyboneLogo(){setComponentID("jellybone-logo");setWantsKeyboardFocus(true);setTitle("About Jellybone");setTooltip("About Jellybone and licences");setMouseCursor(juce::MouseCursor::PointingHandCursor);}
    std::function<void()> onClick;
    void paint(juce::Graphics& g) override {
        g.setColour(juce::Colours::white);g.setFont(juce::FontOptions(24.f,juce::Font::bold));
        g.drawText("Jellybone",0,0,getWidth(),28,juce::Justification::centredLeft);
        g.setColour(juce::Colour(0xff8993a1));g.setFont(juce::FontOptions(9.f));
        g.drawText("FAZIL IRAK / SHAPER RACK",0,28,getWidth(),12,juce::Justification::centredLeft);
    }
    void mouseUp(const juce::MouseEvent& e) override {if(e.mouseWasClicked()&&getLocalBounds().contains(e.getPosition())&&onClick)onClick();}
    bool keyPressed(const juce::KeyPress& key) override {if(key==juce::KeyPress::returnKey||key==juce::KeyPress::spaceKey){if(onClick)onClick();return true;}return false;}
};
class ShaperEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
  public:
    explicit ShaperEditor(ShaperProcessor &);
    ~ShaperEditor() override;
    void paint(juce::Graphics &) override;
    void resized() override;
    void executeEdit(int command);
    bool saveRackFile(const juce::File &);
    bool loadRackFile(const juce::File &);
    bool saveEffectFile(const juce::File &);
    bool loadEffectFile(const juce::File &);
    void loadFactory(int index);
    void showAbout();

  private:
    ShaperProcessor &processor;
    RackLookAndFeel theme;
    JellyboneLogo title;
    juce::Label presetLabel, rackLabel;
    EffectPalette palette;
    juce::Label emptyLabel;
    juce::ComboBox preset, rackPreset;
    CompactIconButton previous{"<"}, next{">"}, saveRack{"Save rack"}, loadRack{"Load rack"},
        options{"Options"}, aboutButton{"About"};
    CompactIconButton previousRack{"<"}, nextRack{">"}, saveEffect{"Save"}, loadEffect{"Load"};
    std::unique_ptr<AboutPanel> aboutPanel;
    juce::File rackDirectory;
    std::vector<juce::File> rackFiles;
    std::array<CompactIconButton, 6> editButtons;
    juce::TooltipWindow tooltip{this, 450};
    std::unique_ptr<juce::FileChooser> chooser;
    juce::Viewport viewport;
    std::unique_ptr<juce::AudioProcessorEditor> child;
    void select();
    void refreshPresets();
    void chooseRack(bool save);
    void chooseEffect(bool save);
    void refreshRackPresets(const juce::File &selected = {});
    void timerCallback() override;
    int displayEffect = -1;
    int displayedProgram = -1, displayedBand = -1;
    bool customEffectPreset = false;
    std::vector<int> displayPalette;
    static juce::Colour effectColour(int effect);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ShaperEditor)
};
