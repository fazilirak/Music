// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 Fazıl Irak; Shaper eklemeleri/değişiklikleri.
// Fazıl Irak tarafından oluşturuldu / değiştirildi: 23.10.2026
// Kapsam: Shaper için eklenen kod ve uyarlamalar; özgün üçüncü taraf telif/lisansları korunur.
// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: Çok noktalı envelope eğrisi, düğüm/segment düzenleme, kalıp ve taşıma etkileşimlerini yönetir.
#pragma once
#include "EnvelopeEffect.h"
#include "Embedded/RackEditorCommands.h"
#include "Embedded/RackKnob.h"
class EnvelopeEditor final : public juce::AudioProcessorEditor, public RackEditorCommands, private juce::Timer
{
    EnvelopeEffect &owner;
    std::unique_ptr<juce::AudioProcessorEditor> curveEditor;
    struct Panel final : juce::Component
    {
        // Mevcut kontrol/durumun görselini çizer; ses callback’inde çağrılmamalıdır.
        void paint(juce::Graphics &g) override
        {
            g.fillAll(juce::Colour(0xff1c2027));
            g.setColour(juce::Colour(0xff87909c));
            g.setFont(juce::FontOptions(10.5f));
            g.drawText("EFFECT SETTINGS", 12, 3, 180, 14, juce::Justification::centredLeft);
        }
    } settings;
    juce::LookAndFeel_V4 theme;
    std::vector<std::unique_ptr<shaperui::RackKnob>> knobs;
    juce::ComboBox model;
    std::array<juce::ToggleButton, 3> switches;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>, 3> buttonAttachments;
    juce::TextButton pitchTab{"Pitch curve"}, formantTab{"Formant curve"};
    juce::Label hint;
    int dockLeft = 316;
    void rebuild();
    void timerCallback() override;
    // commands alt adımı: Çok noktalı envelope eğrisi, düğüm/segment düzenleme, kalıp ve taşıma etkileşimlerini yönetir.
    RackEditorCommands *commands() const
    {
        return dynamic_cast<RackEditorCommands *>(curveEditor.get());
    }

  public:
    explicit EnvelopeEditor(EnvelopeEffect &);
    ~EnvelopeEditor() override;
    void resized() override;
    juce::PopupMenu rackOptions() override;
    void rackCommand(int) override;
    // rackCanEdit alt adımı: Çok noktalı envelope eğrisi, düğüm/segment düzenleme, kalıp ve taşıma etkileşimlerini yönetir.
    bool rackCanEdit() const override
    {
        return commands() && commands()->rackCanEdit();
    }
    // rackIsSequencing alt adımı: Çok noktalı envelope eğrisi, düğüm/segment düzenleme, kalıp ve taşıma etkileşimlerini yönetir.
    bool rackIsSequencing() const override
    {
        return commands() && commands()->rackIsSequencing();
    }
    // rackDockHeader alt adımı: Çok noktalı envelope eğrisi, düğüm/segment düzenleme, kalıp ve taşıma etkileşimlerini yönetir.
    void rackDockHeader(int left) override
    {
        dockLeft = left;
        resized();
    }
    // rackHeaderHeight alt adımı: Çok noktalı envelope eğrisi, düğüm/segment düzenleme, kalıp ve taşıma etkileşimlerini yönetir.
    int rackHeaderHeight() const override
    {
        return 330;
    }
};
