// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 Fazıl Irak; Shaper eklemeleri/değişiklikleri.
// Fazıl Irak tarafından oluşturuldu / değiştirildi: 24.10.2026
// Kapsam: Shaper için eklenen kod ve uyarlamalar; özgün üçüncü taraf telif/lisansları korunur.
// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 24.10.2026
// Dosyanın görevi: Ana rack editörünü, efekt sırasını, seçimlerini ve mevcut ortak kontrol yerleşimini yönetir.
#include "Editor.h"
#include "MultibandProcessor.h"
namespace
{
constexpr int commands[]{53, 54, 55, 56, 57, 52};
}
// effectColour alt adımı: Ana rack editörünü, efekt sırasını, seçimlerini ve mevcut ortak kontrol yerleşimini yönetir.
juce::Colour ShaperEditor::effectColour(int e)
{
    return e < 0 ? juce::Colour(0xff596574) : rackEffectColour(e);
}
// ShaperEditor alt adımı: Ana rack editörünü, efekt sırasını, seçimlerini ve mevcut ortak kontrol yerleşimini yönetir.
// Gövdedeki başlıca çağrılar: theme.setColour → juce::Colour → setLookAndFeel → title.setText. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
// Eşzamanlılık sınırı: bu kilidin hangi thread’de alındığını kontrol edin; audio callback’e yeni bekleyen kilit taşımayın.
ShaperEditor::ShaperEditor(ShaperProcessor &p) : AudioProcessorEditor(p), processor(p), palette(p)
{
    theme.setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(0xff141820));
    theme.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff252c39));
    theme.setColour(juce::Slider::thumbColourId, juce::Colour(0xff6ce1cf));
    setLookAndFeel(&theme);
    title.onClick = [this] { showAbout(); };
    addAndMakeVisible(title);
    addAndMakeVisible(aboutButton);
    aboutButton.setComponentID("rack-about-button");
    aboutButton.onClick = [this] { showAbout(); };
    rackDirectory =
        juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("Shaper Presets");
    rackLabel.setText("RACK PRESET", juce::dontSendNotification);
    addAndMakeVisible(rackLabel);
    addAndMakeVisible(rackPreset);
    rackPreset.setEditableText(true);
    rackPreset.setTextWhenNothingSelected("Name or select a complete rack preset");
    rackPreset.setTooltip("Complete effect palette, order, bypass, curves and settings");
    rackPreset.onChange = [this]
    {
        int id = rackPreset.getSelectedId();
        if (id >= 1 && id <= ShaperProcessor::factoryPresetCount)
            loadFactory(id - 1);
        else if (id >= 1001 && id < 1001 + (int)rackFiles.size())
            loadRackFile(rackFiles[(size_t)(id - 1001)]);
    };
    refreshRackPresets();
    addAndMakeVisible(palette);
    palette.onChange = [this] { select(); };
    emptyLabel.setText(
        "Your rack is empty\n\nClick + to add Gate, Time, Filter, Reverb, Volume, Pan, Saturation or Pitch.",
        juce::dontSendNotification);
    emptyLabel.setFont(juce::FontOptions(20));
    emptyLabel.setJustificationType(juce::Justification::centred);
    emptyLabel.setColour(juce::Label::textColourId, juce::Colour(0xff8993a1));
    addAndMakeVisible(emptyLabel);
    // Fazıl Irak, 24.10.2026: alt rack Mix/Out görselleri kaldırıldı.
    // Efektin sağ üst ortak çıkış paneli korunur. Eski rack mix/gain parametreleri
    // işlemcide kalır; DAW otomasyonu ve eski presetlerin sesi değişmez.
    presetLabel.setText("EFFECT PRESET", juce::dontSendNotification);
    addChildComponent(presetLabel);
    preset.setComponentID("effect-preset");
    rackPreset.setComponentID("rack-preset");
    previous.setComponentID("previous-effect");
    next.setComponentID("next-effect");
    for (auto *button : {&previous, &next, &saveRack, &loadRack, &options, &previousRack, &nextRack,
                         &saveEffect, &loadEffect})
        addAndMakeVisible(button);
    previousRack.setComponentID("previous-rack");
    nextRack.setComponentID("next-rack");
    saveEffect.setComponentID("save-effect");
    loadEffect.setComponentID("load-effect");
    previousRack.setTooltip("Previous complete rack preset");
    nextRack.setTooltip("Next complete rack preset");
    addAndMakeVisible(preset);
    preset.onChange = [this]
    {
        if (preset.getSelectedId() > 0 && displayEffect >= 0)
        {
            const juce::ScopedLock lock(processor.getCallbackLock());
            processor.suspendProcessing(true);
            processor.module(displayEffect).setCurrentProgram(preset.getSelectedId() - 1);
            customEffectPreset = false;
            processor.suspendProcessing(false);
        }
    };
    previous.onClick = [this]
    {
        int n = preset.getNumItems();
        if (n > 0)
            preset.setSelectedItemIndex((preset.getSelectedItemIndex() + n - 1) % n,
                                        juce::sendNotificationSync);
    };
    next.onClick = [this]
    {
        int n = preset.getNumItems();
        if (n > 0)
            preset.setSelectedItemIndex((preset.getSelectedItemIndex() + 1) % n, juce::sendNotificationSync);
    };
    saveRack.onClick = [this] { chooseRack(true); };
    loadRack.onClick = [this] { chooseRack(false); };
    previousRack.onClick = [this]
    {
        int n = rackPreset.getNumItems();
        if (n > 0)
            rackPreset.setSelectedItemIndex((rackPreset.getSelectedItemIndex() + n - 1) % n,
                                            juce::sendNotificationSync);
    };
    nextRack.onClick = [this]
    {
        int n = rackPreset.getNumItems();
        if (n > 0)
            rackPreset.setSelectedItemIndex((rackPreset.getSelectedItemIndex() + 1) % n,
                                            juce::sendNotificationSync);
    };
    saveEffect.onClick = [this] { chooseEffect(true); };
    loadEffect.onClick = [this] { chooseEffect(false); };
    options.onClick = [this]
    {
        if (auto *editor = dynamic_cast<RackEditorCommands *>(child.get()))
        {
            auto menu = editor->rackOptions();
            juce::Component::SafePointer<ShaperEditor> safe(this);
            int effect = displayEffect;
            menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&options),
                               [safe, effect](int result)
                               {
                                   if (safe && safe->displayEffect == effect && result > 0)
                                       safe->executeEdit(result);
                               });
        }
    };
    const char *labels[]{"Copy", "Paste", "Invert", "Reverse", "Double", "Clear"};
    for (int i = 0; i < 6; ++i)
    {
        auto &button = editButtons[i];
        button.setButtonText(labels[i]);
        button.setTooltip(labels[i]);
        button.setComponentID("edit-" + juce::String(labels[i]).toLowerCase());
        button.onClick = [this, i] { executeEdit(commands[i]); };
        addAndMakeVisible(button);
    }
    previous.setTooltip("Previous effect preset");
    next.setTooltip("Next effect preset");
    saveEffect.setTooltip("Save effect preset");
    loadEffect.setTooltip("Load effect preset");
    addAndMakeVisible(viewport);
    viewport.setScrollBarsShown(true, true);
    setResizable(true, false);
    setResizeLimits(1100, 850, 1800, 1500);
    setSize(1100, 960);
    select();
    startTimerHz(5);
}
// ~ShaperEditor alt adımı: Ana rack editörünü, efekt sırasını, seçimlerini ve mevcut ortak kontrol yerleşimini yönetir.
ShaperEditor::~ShaperEditor()
{
    stopTimer();
    viewport.setViewedComponent(nullptr, false);
    child.reset();
    setLookAndFeel(nullptr);
}
// select alt adımı: Ana rack editörünü, efekt sırasını, seçimlerini ve mevcut ortak kontrol yerleşimini yönetir.
// Gövdedeki başlıca çağrılar: processor.palette → std::find → displayPalette.begin → displayPalette.end. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
void ShaperEditor::select()
{
    displayPalette = processor.palette();
    if (std::find(displayPalette.begin(), displayPalette.end(), processor.selectedEffect) ==
        displayPalette.end())
        processor.selectedEffect = displayPalette.empty() ? -1 : displayPalette.front();
    displayEffect = processor.selectedEffect;
    palette.refresh();
    viewport.setViewedComponent(nullptr, false);
    child.reset();
    if (displayEffect >= 0)
    {
        child.reset(processor.module(displayEffect).createEditor());
        child->setResizable(false, false);
        viewport.setViewedComponent(child.get(), false);
    }
    emptyLabel.setVisible(displayEffect < 0);
    preset.setEnabled(displayEffect >= 0);
    previous.setEnabled(displayEffect >= 0);
    next.setEnabled(displayEffect >= 0);
    options.setEnabled(displayEffect >= 0);
    saveEffect.setEnabled(displayEffect >= 0);
    loadEffect.setEnabled(displayEffect >= 0);
    customEffectPreset = false;
    displayedBand = -1;
    for (auto &button : editButtons)
        button.setEnabled(displayEffect >= 0);
    auto colour = effectColour(displayEffect);
    preset.setTooltip(displayEffect >= 0
                          ? juce::String(ShaperProcessor::effectName(displayEffect)) + " preset"
                          : "Effect preset");
    options.setColour(juce::TextButton::textColourOffId, colour);
    for (auto &button : editButtons)
        button.setColour(juce::TextButton::textColourOffId, colour);
    refreshPresets();
    timerCallback();
    resized();
    repaint();
}
// executeEdit alt adımı: Ana rack editörünü, efekt sırasını, seçimlerini ve mevcut ortak kontrol yerleşimini yönetir.
void ShaperEditor::executeEdit(int command)
{
    if (auto *editor = dynamic_cast<RackEditorCommands *>(child.get()))
    {
        if (command >= 53 && command <= 56 && !editor->rackCanEdit())
            return;
        processor.suspendProcessing(true);
        editor->rackCommand(command);
        processor.suspendProcessing(false);
        child->repaint();
        timerCallback();
    }
}
// Mesaj iş parçacığındaki periyodik görünüm eşitlemesini yapar; audio callback değildir.
// Gövdedeki başlıca çağrılar: processor.palette → select → palette.repaint → processor.module. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
void ShaperEditor::timerCallback()
{
    if (displayEffect != processor.selectedEffect || displayPalette != processor.palette())
    {
        select();
        return;
    }
    palette.repaint();
    if (displayEffect < 0)
        return;
    auto *mb = dynamic_cast<MultibandProcessor *>(&processor.module(displayEffect));
    int band = mb ? mb->editorBand() : 1;
    int current = processor.module(displayEffect).getCurrentProgram() + 1;
    if (band != displayedBand)
    {
        customEffectPreset = false;
        displayedBand = band;
    }
    if (current != displayedProgram)
        customEffectPreset = false;
    if (!customEffectPreset && preset.getSelectedId() != current)
        preset.setSelectedId(current, juce::dontSendNotification);
    displayedProgram = current;
    if (auto *editor = dynamic_cast<RackEditorCommands *>(child.get()))
    {
        for (int i = 0; i < 4; ++i)
            editButtons[i].setEnabled(editor->rackCanEdit());
        editButtons[5].setButtonText(editor->rackIsSequencing() ? "Reset" : "Clear");
        editButtons[5].setTooltip(editor->rackIsSequencing() ? "Reset sequence" : "Clear curve");
    }
}
// refreshPresets alt adımı: Ana rack editörünü, efekt sırasını, seçimlerini ve mevcut ortak kontrol yerleşimini yönetir.
// Gövdedeki başlıca çağrılar: preset.clear → preset.setTextWhenNothingSelected → processor.module → m.getNumPrograms. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
void ShaperEditor::refreshPresets()
{
    if (displayEffect < 0)
    {
        preset.clear(juce::dontSendNotification);
        preset.setTextWhenNothingSelected("Add an effect");
        return;
    }
    auto &m = processor.module(displayEffect);
    preset.clear(juce::dontSendNotification);
    for (int i = 0; i < m.getNumPrograms(); ++i)
    {
        auto name = m.getProgramName(i);
        preset.addItem(name.isNotEmpty() ? name : "Preset " + juce::String(i + 1), i + 1);
    }
    preset.setSelectedId(m.getCurrentProgram() + 1, juce::dontSendNotification);
    displayedProgram = m.getCurrentProgram() + 1;
}
// chooseRack alt adımı: Ana rack editörünü, efekt sırasını, seçimlerini ve mevcut ortak kontrol yerleşimini yönetir.
// Gövdedeki başlıca çağrılar: rackDirectory.createDirectory → juce::File::createLegalFileName → rackPreset.getText → isNotEmpty. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
void ShaperEditor::chooseRack(bool save)
{
    if (save)
        rackDirectory.createDirectory();
    auto name =
        juce::File::createLegalFileName(rackPreset.getText().isNotEmpty() ? rackPreset.getText() : "My Rack");
    chooser = std::make_unique<juce::FileChooser>(
        save ? "Save Jellybone rack" : "Load Jellybone rack",
        save ? rackDirectory.getChildFile(name).withFileExtension("shaper") : rackDirectory, "*.shaper");
    juce::Component::SafePointer<ShaperEditor> safe(this);
    chooser->launchAsync(
        juce::FileBrowserComponent::canSelectFiles |
            (save ? juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::warnAboutOverwriting
                  : juce::FileBrowserComponent::openMode),
        [safe, save](const juce::FileChooser &fc)
        {
            if (!safe)
                return;
            auto file = fc.getResult();
            if (file == juce::File())
                return;
            bool ok = false;
            if (save)
                ok = safe->saveRackFile(file.withFileExtension("shaper"));
            else
                ok = safe->loadRackFile(file);
            if (!ok)
                juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon, "Preset",
                                                       save ? "The rack could not be saved."
                                                            : "The rack could not be loaded.");
        });
}
// saveRackFile: Ana rack editörünü, efekt sırasını, seçimlerini ve mevcut ortak kontrol yerleşimini yönetir. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
// Gövdedeki başlıca çağrılar: file.getFileNameWithoutExtension → processor.getStateInformation → file.replaceWithData → block.getData. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
bool ShaperEditor::saveRackFile(const juce::File &file)
{
    auto previousName = processor.rackPresetName;
    int previousIndex = processor.factoryPresetIndex;
    processor.rackPresetName = file.getFileNameWithoutExtension();
    processor.factoryPresetIndex = -1;
    juce::MemoryBlock block;
    processor.getStateInformation(block);
    if (!file.replaceWithData(block.getData(), block.getSize()))
    {
        processor.rackPresetName = previousName;
        processor.factoryPresetIndex = previousIndex;
        return false;
    }
    rackDirectory = file.getParentDirectory();
    refreshRackPresets(file);
    return true;
}
// loadRackFile: Ana rack editörünü, efekt sırasını, seçimlerini ve mevcut ortak kontrol yerleşimini yönetir. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
// Gövdedeki başlıca çağrılar: file.loadFileAsData → juce::AudioProcessor::getXmlFromBinary → block.getData → block.getSize. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
bool ShaperEditor::loadRackFile(const juce::File &file)
{
    juce::MemoryBlock block;
    if (!file.loadFileAsData(block))
        return false;
    auto xml = juce::AudioProcessor::getXmlFromBinary(block.getData(), (int)block.getSize());
    if (!xml || !xml->hasTagName("SHAPER"))
        return false;
    viewport.setViewedComponent(nullptr, false);
    child.reset();
    processor.suspendProcessing(true);
    processor.setStateInformation(block.getData(), (int)block.getSize());
    processor.suspendProcessing(false);
    processor.rackPresetName = file.getFileNameWithoutExtension();
    processor.factoryPresetIndex = -1;
    rackDirectory = file.getParentDirectory();
    refreshRackPresets(file);
    select();
    return true;
}
// refreshRackPresets alt adımı: Ana rack editörünü, efekt sırasını, seçimlerini ve mevcut ortak kontrol yerleşimini yönetir.
// Gövdedeki başlıca çağrılar: rackPreset.clear → rackFiles.clear → rackPreset.addItem → ShaperProcessor::factoryPresetName. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
void ShaperEditor::refreshRackPresets(const juce::File &selected)
{
    const auto selectedFile = selected;
    rackPreset.clear(juce::dontSendNotification);
    rackFiles.clear();
    for (int i = 0; i < ShaperProcessor::factoryPresetCount; ++i)
        rackPreset.addItem(ShaperProcessor::factoryPresetName(i), i + 1);
    rackPreset.addSeparator();
    auto files = rackDirectory.findChildFiles(juce::File::findFiles, false, "*.shaper");
    std::sort(files.begin(), files.end(), [](const juce::File &a, const juce::File &b)
              { return a.getFileName().compareNatural(b.getFileName()) < 0; });
    for (const auto &file : files)
    {
        rackFiles.push_back(file);
        rackPreset.addItem(file.getFileNameWithoutExtension(), 1000 + (int)rackFiles.size());
        if (file == selectedFile)
            rackPreset.setSelectedId(1000 + (int)rackFiles.size(), juce::dontSendNotification);
    }
    if (selectedFile == juce::File())
    {
        if (processor.factoryPresetIndex >= 0)
            rackPreset.setSelectedId(processor.factoryPresetIndex + 1, juce::dontSendNotification);
        else if (processor.rackPresetName.isNotEmpty())
            rackPreset.setText(processor.rackPresetName, juce::dontSendNotification);
    }
}
// loadFactory: Ana rack editörünü, efekt sırasını, seçimlerini ve mevcut ortak kontrol yerleşimini yönetir. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
// Eşzamanlılık sınırı: bu kilidin hangi thread’de alındığını kontrol edin; audio callback’e yeni bekleyen kilit taşımayın.
void ShaperEditor::loadFactory(int index)
{
    const juce::ScopedLock lock(processor.getCallbackLock());
    viewport.setViewedComponent(nullptr, false);
    child.reset();
    processor.suspendProcessing(true);
    processor.loadFactoryPreset(index);
    processor.suspendProcessing(false);
    rackPreset.setSelectedId(index + 1, juce::dontSendNotification);
    select();
}
// chooseEffect alt adımı: Ana rack editörünü, efekt sırasını, seçimlerini ve mevcut ortak kontrol yerleşimini yönetir.
// Gövdedeki başlıca çağrılar: rackDirectory.getChildFile → getChildFile → ShaperProcessor::effectName → directory.createDirectory. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
void ShaperEditor::chooseEffect(bool save)
{
    if (displayEffect < 0)
        return;
    auto directory =
        rackDirectory.getChildFile("Effects").getChildFile(ShaperProcessor::effectName(displayEffect));
    if (save)
        directory.createDirectory();
    chooser = std::make_unique<juce::FileChooser>(
        save ? "Save effect preset (all bands)" : "Load effect preset",
        save ? directory.getChildFile("My " + juce::String(ShaperProcessor::effectName(displayEffect)) +
                                      ".shapereffect")
             : directory,
        "*.shapereffect");
    juce::Component::SafePointer<ShaperEditor> safe(this);
    int effect = displayEffect;
    chooser->launchAsync(
        juce::FileBrowserComponent::canSelectFiles |
            (save ? juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::warnAboutOverwriting
                  : juce::FileBrowserComponent::openMode),
        [safe, save, effect](const juce::FileChooser &fc)
        {
            if (!safe || safe->displayEffect != effect || fc.getResult() == juce::File())
                return;
            bool ok = save ? safe->saveEffectFile(fc.getResult().withFileExtension("shapereffect"))
                           : safe->loadEffectFile(fc.getResult());
            if (!ok)
                juce::AlertWindow::showMessageBoxAsync(
                    juce::MessageBoxIconType::WarningIcon, "Effect preset",
                    "Could not read or write this preset. Load a preset for the selected effect.");
        });
}
// saveEffectFile: Ana rack editörünü, efekt sırasını, seçimlerini ve mevcut ortak kontrol yerleşimini yönetir. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
// Gövdedeki başlıca çağrılar: lock → processor.getCallbackLock → processor.module → getStateInformation. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
// Eşzamanlılık sınırı: bu kilidin hangi thread’de alındığını kontrol edin; audio callback’e yeni bekleyen kilit taşımayın.
bool ShaperEditor::saveEffectFile(const juce::File &file)
{
    if (displayEffect < 0)
        return false;
    const juce::ScopedLock lock(processor.getCallbackLock());
    juce::MemoryBlock data, wrapped;
    processor.module(displayEffect).getStateInformation(data);
    juce::XmlElement xml("SHAPER_EFFECT");
    xml.setAttribute("effect", displayEffect);
    xml.setAttribute("version", 1);
    xml.addTextElement(data.toBase64Encoding());
    juce::AudioProcessor::copyXmlToBinary(xml, wrapped);
    return file.replaceWithData(wrapped.getData(), wrapped.getSize());
}
// loadEffectFile: Ana rack editörünü, efekt sırasını, seçimlerini ve mevcut ortak kontrol yerleşimini yönetir. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
// Gövdedeki başlıca çağrılar: file.loadFileAsData → juce::AudioProcessor::getXmlFromBinary → wrapped.getData → wrapped.getSize. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
// Eşzamanlılık sınırı: bu kilidin hangi thread’de alındığını kontrol edin; audio callback’e yeni bekleyen kilit taşımayın.
bool ShaperEditor::loadEffectFile(const juce::File &file)
{
    if (displayEffect < 0)
        return false;
    juce::MemoryBlock wrapped, data;
    if (!file.loadFileAsData(wrapped))
        return false;
    auto xml = juce::AudioProcessor::getXmlFromBinary(wrapped.getData(), (int)wrapped.getSize());
    if (!xml || !xml->hasTagName("SHAPER_EFFECT") || xml->getIntAttribute("effect", -1) != displayEffect ||
        !data.fromBase64Encoding(xml->getAllSubText()))
        return false;
    auto payload = juce::AudioProcessor::getXmlFromBinary(data.getData(), (int)data.getSize());
    if (!payload ||
        !(payload->hasTagName("MULTIBAND") || (displayEffect == 1 && payload->hasTagName("PluginState"))))
        return false;
    const juce::ScopedLock lock(processor.getCallbackLock());
    viewport.setViewedComponent(nullptr, false);
    child.reset();
    processor.suspendProcessing(true);
    processor.module(displayEffect).setStateInformation(data.getData(), (int)data.getSize());
    processor.suspendProcessing(false);
    select();
    customEffectPreset = true;
    preset.setEditableText(true);
    preset.setText(file.getFileNameWithoutExtension(), juce::dontSendNotification);
    return true;
}
// showAbout alt adımı: Ana rack editörünü, efekt sırasını, seçimlerini ve mevcut ortak kontrol yerleşimini yönetir.
void ShaperEditor::showAbout()
{
    if (!aboutPanel)
    {
        aboutPanel = std::make_unique<AboutPanel>();
        addChildComponent(*aboutPanel);
    }
    aboutPanel->setBounds(getLocalBounds());
    aboutPanel->setVisible(true);
    aboutPanel->toFront(true);
    if (displayEffect >= 0)
        aboutPanel->tabs.setCurrentTabIndex(displayEffect);
}
// Mevcut kontrol/durumun görselini çizer; ses callback’inde çağrılmamalıdır.
void ShaperEditor::paint(juce::Graphics &g)
{
    g.fillAll(juce::Colour(0xff141820));
    g.setColour(effectColour(processor.selectedEffect));
    g.fillRect(20, 177, getWidth() - 40, 2);
    g.setColour(juce::Colour(0xff1b222c));
    g.fillRoundedRectangle(20, (float)getHeight() - 66, (float)getWidth() - 40, 46, 4);
}
// Pencere/kontrol boyutuna bağlı yerleşimi yeniden hesaplar; hit-test aynı geometriyi izlemelidir.
// Gövdedeki başlıca çağrılar: title.setBounds → aboutButton.setBounds → getWidth → rackLabel.setBounds. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
void ShaperEditor::resized()
{
    title.setBounds(20, 5, 235, 43);
    aboutButton.setBounds(getWidth() - 48, 14, 28, 28);
    rackLabel.setBounds(20, 54, 115, 30);
    previousRack.setBounds(140, 54, 28, 28);
    rackPreset.setBounds(176, 54, getWidth() - 304, 28);
    nextRack.setBounds(getWidth() - 120, 54, 28, 28);
    saveRack.setBounds(getWidth() - 84, 54, 28, 28);
    loadRack.setBounds(getWidth() - 48, 54, 28, 28);
    palette.setBounds(20, 96, getWidth() - 40, 70);
    const int row = getHeight() - 57;
    previous.setBounds(26, row, 28, 28);
    preset.setBounds(62, row, 225, 28);
    next.setBounds(295, row, 28, 28);
    saveEffect.setBounds(331, row, 28, 28);
    loadEffect.setBounds(367, row, 28, 28);
    options.setBounds(411, row, 28, 28);
    for (int i = 0; i < 6; ++i)
        editButtons[i].setBounds(447 + i * 36, row, 28, 28);
    viewport.setBounds(20, 190, getWidth() - 40, getHeight() - 266);
    emptyLabel.setBounds(viewport.getBounds());
    if (child)
    {
        const int minimumHeight = 650;
        const int visibleWidth =
            viewport.getWidth() -
            (viewport.getHeight() < minimumHeight ? viewport.getScrollBarThickness() : 0);
        child->setSize(juce::jmax(displayEffect == 3 ? 690 : 640, visibleWidth),
                       juce::jmax(minimumHeight, viewport.getHeight()));
    }
    if (aboutPanel)
        aboutPanel->setBounds(getLocalBounds());
}
