// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 Fazıl Irak; Shaper eklemeleri/değişiklikleri.
// Fazıl Irak tarafından oluşturuldu / değiştirildi: 23.10.2026
// Kapsam: Shaper için eklenen kod ve uyarlamalar; özgün üçüncü taraf telif/lisansları korunur.
// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: Çok noktalı envelope eğrisi, düğüm/segment düzenleme, kalıp ve taşıma etkileşimlerini yönetir.
#include "EnvelopeEditor.h"
#include "Saturation/PluginProcessor.h"
#include "Pitch/PitchProcessor.h"
// EnvelopeEditor alt adımı: Çok noktalı envelope eğrisi, düğüm/segment düzenleme, kalıp ve taşıma etkileşimlerini yönetir.
// Gövdedeki başlıca çağrılar: setComponentID → juce::Colour → theme.setColour → colour.withMultipliedBrightness. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
EnvelopeEditor::EnvelopeEditor(EnvelopeEffect &p) : AudioProcessorEditor(p), owner(p)
{
    setComponentID(p.isPitch ? "pitch-editor" : "irak-editor");
    const auto colour = juce::Colour(p.isPitch ? 0xff54dfdb : 0xffed6c46);
    theme.setColour(juce::ToggleButton::tickColourId, colour);
    theme.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff252c39));
    theme.setColour(juce::TextButton::buttonOnColourId, colour.withMultipliedBrightness(.35f));
    theme.setColour(juce::TextButton::textColourOnId, colour);
    setLookAndFeel(&theme);
    addAndMakeVisible(settings);
    auto &state = p.isPitch ? dynamic_cast<pitchfx::Processor &>(*p.native).state
                            : dynamic_cast<irak::Processor &>(*p.native).state;
    const char *satIds[]{"drive", "tone", "lowcut", "highcut", "bias", "dynamics"};
    const char *satNames[]{"Drive", "Tone", "Low cut", "High cut", "Bias", "Dynamics"};
    const char *pitchIds[]{"semitones", "cents", "formant", "formant_base"};
    const char *pitchNames[]{"Pitch", "Fine", "Formant", "Base Hz"};
    for (int i = 0; i < (p.isPitch ? 4 : 6); ++i)
    {
        const char *id = p.isPitch ? pitchIds[i] : satIds[i], *name = p.isPitch ? pitchNames[i] : satNames[i];
        auto k = std::make_unique<shaperui::RackKnob>(state, id, name, colour,
                                                      p.isPitch ? i < 3 : i == 1 || i == 4);
        k->setComponentID((p.isPitch ? "pitch-" : "irak-") + juce::String(id));
        k->format = [id = juce::String(id)](float value)
        {
            if (id == "semitones" || id == "formant")
                return juce::String(value, 1) + " st";
            if (id == "cents")
                return juce::String(value, 1) + " ct";
            if (id == "drive")
                return juce::String(value, 1) + " dB";
            if (id == "formant_base" || id == "lowcut" || id == "highcut")
                return juce::String(juce::roundToInt(value)) + " Hz";
            return juce::String(juce::roundToInt(value * 100)) + " %";
        };
        settings.addAndMakeVisible(*k);
        knobs.push_back(std::move(k));
    }
    auto depth =
        std::make_unique<shaperui::RackKnob>(p.state, "curve_depth", p.isPitch ? "Range" : "Depth", colour);
    depth->setComponentID("effect-curve-depth");
    depth->format = [pitch = p.isPitch](float v)
    { return pitch ? juce::String(v * 24, 1) + " st" : juce::String(juce::roundToInt(v * 100)) + " %"; };
    settings.addAndMakeVisible(*depth);
    knobs.push_back(std::move(depth));
    if (p.isPitch)
    {
        settings.addAndMakeVisible(pitchTab);
        settings.addAndMakeVisible(formantTab);
        pitchTab.setComponentID("pitch-curve-tab");
        formantTab.setComponentID("formant-curve-tab");
        pitchTab.onClick = [this]
        {
            owner.selectedCurve = 0;
            rebuild();
        };
        formantTab.onClick = [this]
        {
            owner.selectedCurve = 1;
            rebuild();
        };
        switches[0].setButtonText("Preserve formants");
        settings.addAndMakeVisible(switches[0]);
        buttonAttachments[0] = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
            state, "preserve_formants", switches[0]);
    }
    else
    {
        for (int i = 0; i < 5; ++i)
            model.addItem(irak::modelNames[i], i + 1);
        settings.addAndMakeVisible(model);
        model.setComponentID("saturation-model");
        model.onChange = [this]
        {
            if (model.getSelectedId() > 0)
            {
                auto &state = dynamic_cast<irak::Processor &>(*owner.native).state;
                auto *param = state.getParameter("model");
                param->beginChangeGesture();
                param->setValueNotifyingHost(param->convertTo0to1((float)model.getSelectedId() - 1));
                param->endChangeGesture();
            }
        };
        const char *ids[]{"heat", "auto_gain", "bypass"};
        const char *names[]{"Heat +20 dB", "Auto gain", "Bypass"};
        for (int i = 0; i < 3; ++i)
        {
            settings.addAndMakeVisible(switches[(size_t)i]);
            switches[(size_t)i].setButtonText(names[i]);
            buttonAttachments[(size_t)i] =
                std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(state, ids[i],
                                                                                       switches[(size_t)i]);
        }
    }
    settings.addAndMakeVisible(hint);
    hint.setFont(juce::FontOptions(11));
    hint.setColour(juce::Label::textColourId, juce::Colour(0xff87909c));
    hint.setText(p.isPitch ? "Centre = 0 st / Pitch +/-24 st / Formant +/-12 st"
                           : "Draw the drive level / Bottom = 0 dB / Top = Drive",
                 juce::dontSendNotification);
    setSize(1060, 700);
    rebuild();
    startTimerHz(30);
}
// ~EnvelopeEditor alt adımı: Çok noktalı envelope eğrisi, düğüm/segment düzenleme, kalıp ve taşıma etkileşimlerini yönetir.
EnvelopeEditor::~EnvelopeEditor()
{
    stopTimer();
    curveEditor.reset();
    knobs.clear();
    setLookAndFeel(nullptr);
}
// Mevcut ayarlara bağlı türetilmiş görünüm/katsayı/cache durumunu yeniden kurar.
void EnvelopeEditor::rebuild()
{
    curveEditor.reset();
    curveEditor.reset(owner.curve().createEditor());
    addAndMakeVisible(*curveEditor);
    settings.toFront(false);
    resized();
    timerCallback();
}
// Pencere/kontrol boyutuna bağlı yerleşimi yeniden hesaplar; hit-test aynı geometriyi izlemelidir.
// Gövdedeki başlıca çağrılar: curveEditor->setBounds → getLocalBounds → commands → c->rackDockHeader. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
void EnvelopeEditor::resized()
{
    if (curveEditor)
    {
        curveEditor->setBounds(getLocalBounds());
        if (auto *c = commands())
            c->rackDockHeader(dockLeft);
    }
    settings.setBounds(dockLeft, 168, getWidth() - dockLeft - 8, 124);
    for (size_t i = 0; i < knobs.size(); ++i)
        knobs[i]->setBounds(12 + (int)i * 72, 22, shaperui::knobWidth, shaperui::knobHeight);
    if (owner.isPitch)
    {
        pitchTab.setBounds(12, 94, 102, 26);
        formantTab.setBounds(120, 94, 112, 26);
        hint.setBounds(242, 95, settings.getWidth() - 250, 24);
        switches[0].setBounds(settings.getWidth() - 178, 26, 176, 28);
    }
    else
    {
        for (int i = 0; i < 3; ++i)
            switches[(size_t)i].setBounds(settings.getWidth() - 148, 16 + i * 30, 145, 27);
        model.setBounds(12, 94, 155, 26);
        hint.setBounds(175, 95, settings.getWidth() - 185, 24);
    }
}
// Mesaj iş parçacığındaki periyodik görünüm eşitlemesini yapar; audio callback değildir.
void EnvelopeEditor::timerCallback()
{
    settings.setVisible(!owner.audioSettingsShown());
    pitchTab.setToggleState(owner.selectedCurve == 0, juce::dontSendNotification);
    formantTab.setToggleState(owner.selectedCurve == 1, juce::dontSendNotification);
    if (!owner.isPitch)
        model.setSelectedId(dynamic_cast<irak::Processor &>(*owner.native).settings().model + 1,
                            juce::dontSendNotification);
}
// rackOptions alt adımı: Çok noktalı envelope eğrisi, düğüm/segment düzenleme, kalıp ve taşıma etkileşimlerini yönetir.
juce::PopupMenu EnvelopeEditor::rackOptions()
{
    juce::PopupMenu result;
    if (auto *c = commands())
    {
        auto menu = c->rackOptions();
        for (juce::PopupMenu::MenuItemIterator it(menu, false); it.next();)
            if (it.getItem().text != "Output")
                result.addItem(it.getItem());
    }
    return result;
}
// rackCommand alt adımı: Çok noktalı envelope eğrisi, düğüm/segment düzenleme, kalıp ve taşıma etkileşimlerini yönetir.
void EnvelopeEditor::rackCommand(int command)
{
    if (command != 700 && commands())
        commands()->rackCommand(command);
}
