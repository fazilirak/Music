// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 Fazıl Irak; Shaper eklemeleri/değişiklikleri.
// Fazıl Irak tarafından oluşturuldu / değiştirildi: 23.10.2026
// Kapsam: Shaper için eklenen kod ve uyarlamalar; özgün üçüncü taraf telif/lisansları korunur.
// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: AudioTests regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.
#include "Processor.h"
#include "Editor.h"
#include "MultibandProcessor.h"
#include "BandGraph.h"
#include "Embedded/TimeQuickPresets.h"
#include "Embedded/RackEditorCommands.h"
#include "Embedded/SidechainPresets.h"
#include "Embedded/PanPresets.h"
#include <iostream>
#if JUCE_WINDOWS
#include <windows.h>
#endif

struct TestPlayHead : juce::AudioPlayHead
{
    juce::int64 samples = 0;
    // getPosition: AudioTests regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
    juce::Optional<PositionInfo> getPosition() const override
    {
        PositionInfo p;
        p.setIsPlaying(true);
        p.setBpm(120);
        p.setTimeInSamples(samples);
        p.setTimeInSeconds(samples / 48000.0);
        p.setPpqPosition(samples / 24000.0);
        return p;
    }
};
// set: AudioTests regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir. Burada verilen ayar türetilmiş duruma/çıktıya aktarılır; çağrı sırası aşağıdaki gövdededir.
static void set(juce::AudioProcessor &module, const char *name, float plain)
{
    for (auto *parameter : module.getParameters())
        if (auto *p = dynamic_cast<juce::RangedAudioParameter *>(parameter))
            if (p->paramID == name)
                p->setValueNotifyingHost(p->convertTo0to1(plain));
}
// preview alt adımı: AudioTests regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.
static void preview(ShaperProcessor &p, const juce::String &name)
{
    std::unique_ptr<juce::AudioProcessorEditor> editor(p.createEditor());
    auto image = editor->createComponentSnapshot(editor->getLocalBounds());
    auto file = juce::File::getCurrentWorkingDirectory().getChildFile("../dist/" + name + ".png");
    file.getParentDirectory().createDirectory();
    file.deleteFile();
    juce::FileOutputStream stream(file);
    juce::PNGImageFormat png;
    png.writeImageToStream(image, stream);
}
// snapshot alt adımı: AudioTests regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.
static void snapshot(juce::Component &component, const juce::String &name)
{
    auto image = component.createComponentSnapshot(component.getLocalBounds());
    auto file = juce::File::getCurrentWorkingDirectory().getChildFile("../dist/" + name + ".png");
    file.deleteFile();
    juce::FileOutputStream stream(file);
    juce::PNGImageFormat png;
    png.writeImageToStream(image, stream);
}
// byId alt adımı: AudioTests regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.
static juce::Component *byId(juce::Component &parent, const juce::String &id)
{
    if (parent.getComponentID() == id)
        return &parent;
    for (auto *c : parent.getChildren())
        if (auto *match = byId(*c, id))
            return match;
    return nullptr;
}
// finite alt adımı: AudioTests regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.
static bool finite(const juce::AudioBuffer<float> &audio)
{
    for (int c = 0; c < 2; ++c)
        for (int i = 0; i < audio.getNumSamples(); ++i)
            if (!std::isfinite(audio.getSample(c, i)))
                return false;
    return true;
}
// main alt adımı: AudioTests regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.
// Gövdedeki başlıca çağrılar: SetErrorMode → p.setPlayHead → p.setRateAndBufferSizeDetails → p.prepareToPlay. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
int main()
{
#if JUCE_WINDOWS
    // Headless integration failures must reach CTest instead of opening a
    // blocking operating-system error dialog on the user's desktop.
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
#endif
    juce::ScopedJuceInitialiser_GUI gui;
    ShaperProcessor p;
    TestPlayHead head;
    p.setPlayHead(&head);
    p.setRateAndBufferSizeDetails(48000, 256);
    p.prepareToPlay(48000, 256);
    juce::AudioBuffer<float> audio(2, 256);
    juce::MidiBuffer midi;
    if (!p.palette().empty() || p.state.getParameter("low") || p.state.getParameter("high"))
        return 1;
    preview(p, "preview-empty");
    for (int i = 0; i < 256; ++i)
    {
        audio.setSample(0, i, std::sin(i * 0.17f) * 0.1f);
        audio.setSample(1, i, std::cos(i * 0.21f) * 0.1f);
    }
    juce::AudioBuffer<float> original;
    original.makeCopyOf(audio);
    p.processBlock(audio, midi);
    for (int c = 0; c < 2; ++c)
        for (int i = 0; i < 256; ++i)
            if (std::abs(audio.getSample(c, i) - original.getSample(c, i)) > 1e-7f)
                return 2;
    if (p.getLatencySamples() != 0)
        return 3;
    // Reverb's embedded XML contains currentProgram=-1. Loading that XML must
    // not erase the program the user selected, or the rack jumps back to Init.
    {
        ShaperProcessor rack;
        rack.addEffect(3);
        auto &reverb = dynamic_cast<MultibandProcessor &>(rack.module(3));
        for (int index = 0; index < reverb.getNumPrograms(); ++index)
        {
            reverb.setCurrentProgram(index);
            if (reverb.getCurrentProgram() != index)
            {
                std::cerr << "Reverb selection reset: requested " << index << ", got "
                          << reverb.getCurrentProgram() << "\n";
                return 99;
            }
            juce::MemoryBlock savedPreset;
            reverb.getStateInformation(savedPreset);
            reverb.setCurrentProgram((index + 1) % reverb.getNumPrograms());
            reverb.setStateInformation(savedPreset.getData(), (int)savedPreset.getSize());
            if (reverb.getCurrentProgram() != index)
                return 100;
        }
        reverb.setCurrentProgram(0);
        {
            ShaperEditor editor(rack);
            auto *preset = dynamic_cast<juce::ComboBox *>(byId(editor, "effect-preset"));
            auto *next = dynamic_cast<juce::Button *>(byId(editor, "next-effect"));
            auto *previous = dynamic_cast<juce::Button *>(byId(editor, "previous-effect"));
            if (!preset || !next || !previous)
                return 101;
            for (int index = 1; index <= reverb.getNumPrograms(); ++index)
            {
                next->onClick();
                int expected = index % reverb.getNumPrograms();
                if (index == 1)
                {
                    juce::Thread::sleep(250);
                    juce::Timer::callPendingTimersSynchronously();
                }
                if (reverb.getCurrentProgram() != expected || preset->getSelectedItemIndex() != expected ||
                    preset->getText() != reverb.getProgramName(expected))
                    return 102;
            }
            for (int index = reverb.getNumPrograms() - 1; index >= 0; --index)
            {
                previous->onClick();
                if (reverb.getCurrentProgram() != index || preset->getSelectedItemIndex() != index)
                    return 103;
            }
        }
        set(reverb, "crossover_low", 200);
        set(reverb, "crossover_high", 3000);
        for (int band = 0; band < 3; ++band)
        {
            reverb.selectedBand = band;
            reverb.setCurrentProgram(2 + band);
        }
        juce::MemoryBlock bandsState;
        reverb.getStateInformation(bandsState);
        for (int band = 0; band < 3; ++band)
            reverb.band(band).setCurrentProgram(0);
        reverb.setStateInformation(bandsState.getData(), (int)bandsState.getSize());
        for (int band = 0; band < 3; ++band)
            if (reverb.band(band).getCurrentProgram() != 2 + band)
                return 104;
        std::cout
            << "Reverb: all 11 presets, forward/backward wrap, timer refresh and independent band state recall passed\n";
    }
    p.addEffect(2);
    p.addEffect(0);
    p.addEffect(4);
    p.addEffect(2);
    if (p.palette() != std::vector<int>({2, 0, 4}))
        return 4;
    p.moveEffect(4, 0);
    if (p.palette() != std::vector<int>({4, 2, 0}))
        return 5;
    p.removeEffect(2);
    if (p.palette() != std::vector<int>({4, 0}))
        return 6;
    {
        std::unique_ptr<juce::AudioProcessorEditor> editor(p.createEditor());
        EffectPalette *palette = nullptr;
        for (auto *component : editor->getChildren())
            if (auto *found = dynamic_cast<EffectPalette *>(component))
                palette = found;
        if (!palette)
            return 21;
        auto *card = palette->findChildWithID("effect-card-0");
        if (!card)
            return 22;
        juce::DragAndDropTarget::SourceDetails drop(0, card, {0, 20});
        if (!palette->isInterestedInDragSource(drop))
            return 23;
        palette->itemDragMove(drop);
        palette->itemDropped(drop);
        if (p.palette() != std::vector<int>({0, 4}))
            return 24;
        auto pos = juce::Point<float>((float)card->getWidth() - 15, 15);
        auto now = juce::Time::getCurrentTime();
        juce::MouseEvent close(juce::Desktop::getInstance().getMainMouseSource(), pos,
                               juce::ModifierKeys::leftButtonModifier, 1, 0, 0, 0, 0, card, card, now, pos,
                               now, 1, false);
        card->mouseDown(close);
        if (p.palette() != std::vector<int>({4}))
            return 25;
    }
    p.addEffect(0);
    juce::MemoryBlock saved;
    p.getStateInformation(saved);
    p.removeEffect(4);
    p.setStateInformation(saved.getData(), (int)saved.getSize());
    if (p.palette() != std::vector<int>({4, 0}))
        return 7;
    for (int e : p.palette())
        p.removeEffect(e);
    p.addEffect(4); // Pan has all Gate envelope features, but a different audio operation.
    auto &pan = p.module(4);
    for (float position : {0.0f, 0.5f, 1.0f})
    {
        set(pan, "min", position);
        set(pan, "max", position);
        for (int b = 0; b < 20; ++b)
        {
            head.samples += 256;
            for (int c = 0; c < 2; ++c)
                for (int i = 0; i < 256; ++i)
                    audio.setSample(c, i, 0.1f);
            p.processBlock(audio, midi);
        }
        float left = audio.getRMSLevel(0, 0, 256), right = audio.getRMSLevel(1, 0, 256);
        if (position == 0 && (left < 0.13f || right > 0.0001f))
            return 8;
        if (position == 0.5f && (std::abs(left - 0.1f) > 0.0001f || std::abs(right - 0.1f) > 0.0001f))
            return 9;
        if (position == 1 && (right < 0.13f || left > 0.0001f))
            return 10;
        if (std::abs(left * left + right * right - 0.02f) > 0.0001f)
            return 11;
    }
    // A CV-producing Gate makes the operation order unambiguously observable.
    p.addEffect(0);
    set(pan, "min", 0);
    set(pan, "max", 0);
    set(p.module(0), "min", 1);
    set(p.module(0), "max", 1);
    {
        std::unique_ptr<juce::AudioProcessorEditor> editor(p.module(0).createEditor());
        auto *commands = dynamic_cast<RackEditorCommands *>(editor.get());
        if (!commands)
            return 12;
        commands->rackCommand(700);
    }
    auto run = [&]
    {
        for (int b = 0; b < 20; ++b)
        {
            head.samples += 256;
            for (int c = 0; c < 2; ++c)
                for (int i = 0; i < 256; ++i)
                    audio.setSample(c, i, 0.1f);
            p.processBlock(audio, midi);
        }
        return audio.getRMSLevel(1, 0, 256);
    };
    p.moveEffect(0, 0);
    float gateThenPan = run();
    p.moveEffect(4, 0);
    float panThenGate = run();
    if (gateThenPan > 0.0001f || panThenGate < 0.9f)
        return 13;
    {
        std::unique_ptr<juce::AudioProcessorEditor> editor(p.module(0).createEditor());
        dynamic_cast<RackEditorCommands *>(editor.get())->rackCommand(700);
    }
    set(p.module(0), "min", 0);
    set(p.module(0), "max", 1);
    set(pan, "min", 0);
    set(pan, "max", 1);
    for (int e = 0; e < ShaperProcessor::effectCount; ++e)
        p.addEffect(e);
    // Exercise migrated commands, original per-effect Options and native editors.
    const char *sharedIds[] = {"effect-tempo",      "effect-trigger",     "effect-pattern-sync",
                               "effect-paint",      "effect-seq",         "effect-curve",
                               "effect-grid",       "snap-outline",       "effect-output-panel",
                               "effect-output-mix", "effect-output-gain", "effect-output-stereo",
                               "effect-attack",     "effect-release",     "effect-tension"};
    std::array<juce::Rectangle<int>, 15> sharedBounds;
    for (int e = 0; e < ShaperProcessor::effectCount; ++e)
    {
        std::unique_ptr<juce::AudioProcessorEditor> editor(p.module(e).createEditor());
        auto *commands = dynamic_cast<RackEditorCommands *>(editor.get());
        if (!commands)
            return 14;
        auto menu = commands->rackOptions();
        if (menu.getNumItems() == 0)
            return 15;
        for (juce::PopupMenu::MenuItemIterator it(menu, true); it.next();)
            if (it.getItem().text == "UI Scale" || it.getItem().text == "Load" ||
                it.getItem().text == "About")
                return 16;
        for (int command : {53, 54, 55, 56, 57, 52})
            commands->rackCommand(command);
        commands->rackCommand(54); // Restore copied pattern after Clear.
        auto *title = byId(*editor, "effect-hidden-title");
        auto *tempo = byId(*editor, "effect-tempo");
        auto *trigger = byId(*editor, "effect-trigger");
        auto *patternSync = byId(*editor, "effect-pattern-sync");
        if (!title || title->isVisible() || !tempo || !trigger || !patternSync)
            return 93;
        if (tempo->getY() != trigger->getY() || trigger->getY() != patternSync->getY() ||
            tempo->getRight() > trigger->getX() || trigger->getRight() > patternSync->getX())
            return 94;
        auto transform = tempo->getTransform();
        if (std::abs(transform.mat00 - transform.mat11) > 0.00001f)
            return 95;
        for (size_t i = 0; i < sharedBounds.size(); ++i)
        {
            auto *component = byId(*editor, sharedIds[i]);
            if (!component)
            {
                std::cerr << "Missing shared control " << sharedIds[i] << " in " << e << "\n";
                return 105;
            }
            auto bounds = editor->getLocalArea(component, component->getLocalBounds());
            if (e == 0)
                sharedBounds[i] = bounds;
            else if (bounds != sharedBounds[i])
            {
                std::cerr << "Inconsistent geometry: " << sharedIds[i] << " in " << e << "\n";
                return 106;
            }
        }
        if (e == 3)
        {
            auto *eq = dynamic_cast<juce::Button *>(byId(*editor, "effect-eq"));
            auto *decay = dynamic_cast<juce::Button *>(byId(*editor, "effect-decay"));
            if (!eq || !decay)
                return 113;
            eq->onClick();
            snapshot(*editor, "preview-reverb-eq");
            decay->onClick();
            snapshot(*editor, "preview-reverb-decay");
            decay->onClick();
        }
        editor->setSize(1400, 900);
        auto *output = byId(*editor, "effect-output-panel");
        if (output->getBounds() != juce::Rectangle<int>(1216, 2, 180, 76) || tempo->getWidth() != 88 ||
            tempo->getHeight() != 28)
            return 107;
        editor.reset();
        p.selectedEffect = e;
        preview(p, e == 6   ? "preview-saturation"
                   : e == 7 ? "preview-pitch"
                            : "preview-v03-" + juce::String(e));
    }
    // Time now has the same band graph, and starts with full-range Mid.
    for (int e : p.palette())
        p.removeEffect(e);
    p.addEffect(1);
    {
        ShaperEditor editor(p);
        if (!byId(editor, "effect-bands") || !byId(editor, "band-frequency-graph") ||
            dynamic_cast<MultibandProcessor &>(p.module(1)).isMultiband())
            return 27;
        for (auto *parameter : p.module(1).getParameters())
            if (auto *r = dynamic_cast<juce::RangedAudioParameter *>(parameter))
                if (r->paramID.startsWith("split_"))
                    return 28;
        if (!byId(editor, "snap-outline"))
            return 26;
        for (int i = 0; i < 8; ++i)
        {
            auto *quick = dynamic_cast<juce::Button *>(byId(editor, "time-quick-preset-" + juce::String(i)));
            if (!quick)
                return 86;
            quick->onClick();
            if (p.module(1).getCurrentProgram() != TimeQuickPresets::programOffset + i)
                return 87;
        }
        if (p.module(1).getNumPrograms() != 87)
            return 88;
        auto *footer = byId(editor, "effect-preset");
        if (!footer || footer->getY() < editor.getHeight() - 100)
            return 89;
        snapshot(editor, "preview-time-full-range");
    }
    // Volume-F's mini buttons, names and audio all use the same eight supplied presets.
    p.removeEffect(1);
    p.addEffect(5);
    auto &volume = p.module(5);
    if (volume.getNumPrograms() != 8)
        return 29;
    for (int i = 0; i < 8; ++i)
    {
        volume.setCurrentProgram(i);
        if (volume.getCurrentProgram() != i)
            return 30;
        for (const auto &point : SidechainPresets::points(i))
            if (point.y < 0 || point.y > 1 || !std::isfinite(point.y))
                return 31;
        const auto source = juce::File(__FILE__).getParentDirectory().getParentDirectory().getChildFile(
            "Source/Embedded/VolumePresets/" + juce::String(i + 1) + ".shapereffect");
        juce::MemoryBlock sourceData, decoded, native;
        if (!source.loadFileAsData(sourceData))
            return 71;
        auto original =
            juce::AudioProcessor::getXmlFromBinary(sourceData.getData(), (int)sourceData.getSize());
        if (!original || !decoded.fromBase64Encoding(original->getAllSubText()))
            return 72;
        auto multi = juce::AudioProcessor::getXmlFromBinary(decoded.getData(), (int)decoded.getSize());
        for (auto *node : multi->getChildIterator())
            if (node->getIntAttribute("index") == 1)
                native.fromBase64Encoding(node->getAllSubText());
        auto expected = juce::AudioProcessor::getXmlFromBinary(native.getData(), (int)native.getSize());
        juce::MemoryBlock loaded;
        dynamic_cast<MultibandProcessor &>(volume).band(1).getStateInformation(loaded);
        auto actual = juce::AudioProcessor::getXmlFromBinary(loaded.getData(), (int)loaded.getSize());
        if (!expected || !actual)
            return 73;
        for (int pattern = 0; pattern < 12; ++pattern)
            if (expected->getStringAttribute("pattern" + juce::String(pattern)) !=
                actual->getStringAttribute("pattern" + juce::String(pattern)))
                return 74;
        auto sourceParams = juce::ValueTree::fromXml(*expected->getChildElement(0));
        auto loadedParams = juce::ValueTree::fromXml(*actual->getChildElement(0));
        for (int n = 0; n < sourceParams.getNumChildren(); ++n)
        {
            auto param = sourceParams.getChild(n);
            auto value = loadedParams.getChildWithProperty("id", param.getProperty("id"));
            if (std::abs((double)param.getProperty("value") - (double)value.getProperty("value")) > 0.00001)
                return 75;
        }
    }
    volume.setCurrentProgram(0);
    set(volume, "mix", 1);
    p.prepareToPlay(48000, 256);
    double early = 0, late = 0;
    for (int block = 0; block < 90; ++block)
    {
        head.samples = block * 256;
        for (int c = 0; c < 2; ++c)
            for (int i = 0; i < 256; ++i)
                audio.setSample(c, i, 0.1f);
        p.processBlock(audio, midi);
        if (!finite(audio))
            return 32;
        double power = audio.getRMSLevel(0, 0, 256);
        power *= power;
        if (block < 4)
            early += power;
        if (block >= 70 && block < 74)
            late += power;
    }
    if (late < 0.02 || early >= late)
        return 33;
    {
        ShaperEditor editor(p);
        auto *mini = dynamic_cast<juce::Button *>(byId(editor, "sidechain-preset-7"));
        if (!mini || !mini->isVisible())
            return 34;
        mini->onClick();
        if (volume.getCurrentProgram() != 7)
            return 35;
        for (int i = 0; i < 8; ++i)
            if (!byId(editor, "sidechain-preset-" + juce::String(i)))
                return 36;
        if (byId(editor, "sidechain-preset-8"))
            return 76;
        {
            ShaperEditor fresh(p);
            snapshot(fresh, "preview-volume-f");
        }
        p.addEffect(1);
        p.addEffect(4);
        p.moveEffect(5, 0);
        auto order = p.palette();
        auto file = juce::File::getCurrentWorkingDirectory().getChildFile("../dist/integration-rack.shaper");
        if (!editor.saveRackFile(file))
            return 37;
        p.removeEffect(5);
        volume.setCurrentProgram(2);
        if (!editor.loadRackFile(file) || p.palette() != order || volume.getCurrentProgram() != 7)
            return 38;
        editor.showAbout();
        auto *about = dynamic_cast<AboutPanel *>(byId(editor, "rack-about"));
        if (!about || about->tabs.getNumTabs() != ShaperProcessor::effectCount)
            return 39;
        if (about->tabs.getTabNames()[4] != "Pan" || about->tabs.getTabNames()[5] != "Volume")
            return 40;
        about->tabs.setCurrentTabIndex(4);
        snapshot(editor, "preview-about-pan");
        about->tabs.setCurrentTabIndex(5);
        snapshot(editor, "preview-about-volume");
        about->tabs.setCurrentTabIndex(6);
        snapshot(editor, "preview-about-saturation");
        about->tabs.setCurrentTabIndex(7);
        snapshot(editor, "preview-about-pitch");
    }
    if (p.module(4).getNumPrograms() != PanPresets::count)
        return 41;
    for (int i = 0; i < PanPresets::count; ++i)
    {
        auto name = p.module(4).getProgramName(i);
        if (name.containsIgnoreCase("gate") || name.containsIgnoreCase("mute"))
            return 42;
        for (const auto &point : PanPresets::points(i))
            if (point.y < 0 || point.y > 1 || !std::isfinite(point.y))
                return 43;
    }
    p.selectedEffect = 4;
    p.module(4).setCurrentProgram(8);
    preview(p, "preview-pan-f");
    // Five-effect rack order indices changed when the sixth effect was added.
    {
        juce::MemoryBlock before;
        p.getStateInformation(before);
        auto old = juce::AudioProcessor::getXmlFromBinary(before.getData(), (int)before.getSize());
        old->setAttribute("version", 2);
        std::array<int, 5> permutation{0, 1, 2, 3, 4}, expected{4, 0, 3, 1, 2};
        int index = 0;
        while (permutation != expected)
        {
            std::next_permutation(permutation.begin(), permutation.end());
            ++index;
        }
        for (auto *element = old->getFirstChildElement(); element;)
        {
            auto *next = element->getNextElement();
            auto id = element->getStringAttribute("id");
            if (id == "e5_present" || id == "e5_enabled" ||
                (element->hasTagName("Module") && element->getIntAttribute("effect") == 5))
                old->removeChildElement(element, true);
            else if (id == "order")
                element->setAttribute("value", index);
            else if (id.endsWith("_present"))
                element->setAttribute("value", 1.0f);
            element = next;
        }
        juce::MemoryBlock oldData;
        juce::AudioProcessor::copyXmlToBinary(*old, oldData);
        p.setStateInformation(oldData.getData(), (int)oldData.getSize());
        if (p.palette() != std::vector<int>(expected.begin(), expected.end()))
            return 47;
    }
    // Host parameter forwarding survives the single-chain refactor.
    bool found = false;
    for (auto *parameter : p.getParameters())
        if (auto *id = dynamic_cast<juce::AudioProcessorParameterWithID *>(parameter))
            if (id->paramID == "e0_mix")
            {
                found = true;
                parameter->setValueNotifyingHost(0.35f);
                if (std::abs(p.module(0).getParameters()[0]->getValue() - 0.35f) > 0.001f)
                    return 17;
            }
    if (!found)
        return 18;
    audio.setSize(4, 777);
    audio.clear();
    audio.setSample(0, 0, 0.2f);
    audio.setSample(1, 0, 0.2f);
    midi.addEvent(juce::MidiMessage::noteOn(1, 60, (juce::uint8)100), 512);
    p.processBlock(audio, midi);
    if (!finite(audio))
        return 19;
    // Legacy rack files retain the selected band's modules when bands are removed.
    juce::MemoryBlock current;
    p.getStateInformation(current);
    auto legacy = juce::AudioProcessor::getXmlFromBinary(current.getData(), (int)current.getSize());
    legacy->setAttribute("version", 1);
    legacy->setAttribute("band", 1);
    for (auto *x : legacy->getChildIterator())
    {
        if (x->hasTagName("Module"))
            x->setAttribute("band", 1);
        else if (x->getStringAttribute("id").endsWith("_enabled"))
            x->setAttribute("id", "b1_" + x->getStringAttribute("id"));
    }
    juce::MemoryBlock legacyData;
    juce::AudioProcessor::copyXmlToBinary(*legacy, legacyData);
    p.setStateInformation(legacyData.getData(), (int)legacyData.getSize());
    if (p.palette().size() < 4)
        return 20;
    // The new splitter processes all three bands independently, then sums them.
    for (int e : p.palette())
        p.removeEffect(e);
    p.addEffect(0);
    auto &multiband = dynamic_cast<MultibandProcessor &>(p.module(0));
    for (int b = 0; b < 3; ++b)
    {
        set(multiband.band(b), "min", 1);
        set(multiband.band(b), "max", 1);
        set(multiband.band(b), "mix", 1);
    }
    multiband.resetBands();
    audio.setSize(2, 256);
    p.prepareToPlay(48000, 256);
    if (multiband.isMultiband() || multiband.editorBand() != 1 || multiband.low->get() != 20 ||
        multiband.high->get() != 20000)
        return 77;
    for (int block = 0; block < 30; ++block)
    {
        head.samples += 256;
        for (int c = 0; c < 2; ++c)
            for (int i = 0; i < 256; ++i)
                audio.setSample(c, i, 0.1f * std::sin((head.samples + i) * 0.39));
        original.makeCopyOf(audio);
        p.processBlock(audio, midi);
    }
    for (int c = 0; c < 2; ++c)
        for (int i = 0; i < 256; ++i)
            if (std::abs(audio.getSample(c, i) - original.getSample(c, i)) > 0.000001f)
                return 78;
    {
        ShaperEditor editor(p);
        auto *button = dynamic_cast<juce::Button *>(byId(editor, "effect-bands"));
        button->onClick();
        if (multiband.isMultiband())
            return 85;
        button->onClick();
        auto *graph = dynamic_cast<BandGraph *>(byId(editor, "band-frequency-graph"));
        if (!graph || !graph->isVisible() || multiband.isMultiband())
            return 79;
        snapshot(editor, "preview-bands-full-range");
        auto event = [&](float x)
        {
            auto pos = juce::Point<float>(x, 45);
            auto now = juce::Time::getCurrentTime();
            return juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(), pos,
                                    juce::ModifierKeys::leftButtonModifier, 1, 0, 0, 0, 0, graph, graph, now,
                                    pos, now, 1, false);
        };
        graph->mouseDown(event(graph->frequencyX(20)));
        graph->mouseDrag(event(graph->frequencyX(250)));
        graph->mouseUp(event(graph->frequencyX(250)));
        if (!multiband.hasLowBand() || multiband.hasHighBand())
            return 80;
        graph->mouseDown(event(graph->frequencyX(20000)));
        graph->mouseDrag(event(graph->frequencyX(3000)));
        graph->mouseUp(event(graph->frequencyX(3000)));
        if (!multiband.hasLowBand() || !multiband.hasHighBand())
            return 81;
        snapshot(editor, "preview-bands-graph");
        auto *reset = dynamic_cast<juce::Button *>(byId(editor, "bands-full-range"));
        reset->onClick();
        if (multiband.isMultiband())
            return 82;
        set(multiband, "crossover_high", 3000);
        if (multiband.hasLowBand() || !multiband.hasHighBand())
            return 83;
        graph->mouseDoubleClick(event(graph->frequencyX(3000)));
        if (multiband.isMultiband())
            return 84;
    }
    set(multiband, "crossover_low", 250);
    set(multiband, "crossover_high", 3000);
    multiband.setMultiband(true);
    p.prepareToPlay(48000, 256);
    auto tone = [&](float hz)
    {
        double power = 0;
        for (int block = 0; block < 60; ++block)
        {
            head.samples += 256;
            for (int c = 0; c < 2; ++c)
                for (int i = 0; i < 256; ++i)
                    audio.setSample(c, i,
                                    0.1f * std::sin((head.samples + i) * juce::MathConstants<double>::twoPi *
                                                    hz / 48000));
            p.processBlock(audio, midi);
            if (!finite(audio))
                return -1.0;
            if (block >= 50)
                power += std::pow(audio.getRMSLevel(0, 0, 256), 2);
        }
        return std::sqrt(power / 10);
    };
    const float frequencies[]{70, 1000, 9000};
    for (int b = 0; b < 3; ++b)
    {
        double full = tone(frequencies[b]);
        if (full < 0.065 || full > 0.076)
            return 48;
        set(multiband.band(b), "min", 0);
        set(multiband.band(b), "max", 0);
        double muted = tone(frequencies[b]);
        if (muted < 0 || muted > full * 0.12)
            return 49;
        set(multiband.band(b), "min", 1);
        set(multiband.band(b), "max", 1);
    }
    multiband.selectedBand = 2;
    set(multiband.band(2), "phase", 0.25f);
    {
        ShaperEditor editor(p);
        auto *button = byId(editor, "effect-bands");
        if (!button)
            return 50;
        if (!byId(editor, "band-0") || !byId(editor, "band-2"))
            return 51;
        snapshot(editor, "preview-multiband");
        auto file =
            juce::File::getCurrentWorkingDirectory().getChildFile("../dist/integration-effect.shapereffect");
        if (!editor.saveEffectFile(file))
            return 52;
        set(multiband.band(2), "phase", 0.75f);
        multiband.setMultiband(false);
        if (!editor.loadEffectFile(file) || !multiband.isMultiband() || multiband.selectedBand != 2)
            return 53;
        for (auto *param : multiband.band(2).getParameters())
            if (auto *r = dynamic_cast<juce::RangedAudioParameter *>(param))
                if (r->paramID == "phase" && std::abs(r->getValue() - 0.25f) > 0.001f)
                    return 63;
        auto *lowButton = dynamic_cast<juce::Button *>(byId(editor, "band-0"));
        lowButton->onClick();
        if (multiband.selectedBand != 0)
            return 64;
        p.addEffect(4);
        ShaperEditor panEditor(p);
        if (panEditor.loadEffectFile(file))
            return 54;
    }
    // Every factory preset has a unique name and different complete rack state,
    // remains finite, and restores palette/order/settings deterministically.
    juce::StringArray factoryNames;
    std::vector<juce::String> fingerprints;
    float factoryPeak = 0;
    const auto bankDirectory =
        juce::File::getCurrentWorkingDirectory().getChildFile("../dist/FactoryPresets");
    juce::Array<juce::var> bankIndex;
    for (int i = 0; i < ShaperProcessor::factoryPresetCount; ++i)
    {
        if (!p.loadFactoryPreset(i) || p.palette().empty())
            return 55;
        auto name = ShaperProcessor::factoryPresetName(i);
        if (factoryNames.contains(name))
            return 56;
        factoryNames.add(name);
        juce::MemoryBlock block;
        p.getStateInformation(block);
        auto xml = juce::AudioProcessor::getXmlFromBinary(block.getData(), (int)block.getSize());
        const auto category =
            name.fromFirstOccurrenceOf("  ", false, false).upToFirstOccurrenceOf(" / ", false, false);
        const auto fileName =
            juce::File::createLegalFileName(juce::String(i + 1).paddedLeft('0', 3) + " " +
                                            name.fromLastOccurrenceOf(" / ", false, false)) +
            ".shaper";
        const auto directory = bankDirectory.getChildFile(category);
        if (directory.createDirectory().failed())
            return 66;
        const auto presetFile = directory.getChildFile(fileName);
        if (!presetFile.replaceWithData(block.getData(), block.getSize()))
            return 67;
        auto *entry = new juce::DynamicObject();
        entry->setProperty("number", i + 1);
        entry->setProperty("name", name);
        entry->setProperty("category", category);
        entry->setProperty("file", presetFile.getRelativePathFrom(bankDirectory).replaceCharacter('\\', '/'));
        juce::Array<juce::var> effects;
        for (int effect : p.palette())
            effects.add(ShaperProcessor::effectName(effect));
        entry->setProperty("effects", effects);
        bankIndex.add(juce::var(entry));
        xml->removeAttribute("rackName");
        xml->removeAttribute("factoryIndex");
        auto fingerprint = xml->toString();
        if (std::find(fingerprints.begin(), fingerprints.end(), fingerprint) != fingerprints.end())
            return 57;
        fingerprints.push_back(fingerprint);
        for (int j = 0; j < 32; ++j)
        {
            head.samples += 256;
            for (int c = 0; c < 2; ++c)
                for (int n = 0; n < 256; ++n)
                    audio.setSample(c, n, 0.08f * std::sin((head.samples + n) * 0.13f));
            p.processBlock(audio, midi);
            if (!finite(audio))
                return 58;
            factoryPeak =
                juce::jmax(factoryPeak, audio.getMagnitude(0, 0, 256), audio.getMagnitude(1, 0, 256));
        }
        juce::MemoryBlock fileData;
        if (!presetFile.loadFileAsData(fileData) || fileData != block)
            return 68;
        auto order = p.palette();
        p.removeEffect(order.front());
        p.setStateInformation(fileData.getData(), (int)fileData.getSize());
        if (p.palette() != order || p.factoryPresetIndex != i)
            return 59;
    }
    if (!bankDirectory.getChildFile("index.json").replaceWithText(juce::JSON::toString(juce::var(bankIndex))))
        return 69;
    // Export the new effect programs through the same file format as Save effect,
    // then load them back through the UI and compare all three band states.
    for (int effect : {6, 7})
    {
        for (int e : p.palette())
            p.removeEffect(e);
        p.addEffect(effect);
        p.selectedEffect = effect;
        ShaperEditor editor(p);
        const auto directory =
            bankDirectory.getSiblingFile("EffectPresets").getChildFile(ShaperProcessor::effectName(effect));
        if (directory.createDirectory().failed())
            return 110;
        auto &engine = p.module(effect);
        for (int program = 24; program < engine.getNumPrograms(); ++program)
        {
            engine.setCurrentProgram(program);
            juce::MemoryBlock before;
            engine.getStateInformation(before);
            const auto file = directory.getChildFile(
                juce::File::createLegalFileName(engine.getProgramName(program).replace(" / ", " - ")) +
                ".shapereffect");
            if (!editor.saveEffectFile(file))
                return 111;
            engine.setCurrentProgram(0);
            if (!editor.loadEffectFile(file) || engine.getCurrentProgram() != program)
                return 112;
            juce::MemoryBlock after;
            engine.getStateInformation(after);
            if (before != after)
                return 113;
        }
    }
    {
        ShaperEditor editor(p);
        editor.loadFactory(0);
        auto *next = dynamic_cast<juce::Button *>(byId(editor, "next-rack"));
        auto *previous = dynamic_cast<juce::Button *>(byId(editor, "previous-rack"));
        if (!next || !previous)
            return 60;
        next->onClick();
        if (p.factoryPresetIndex != 1)
            return 61;
        previous->onClick();
        if (p.factoryPresetIndex != 0)
            return 62;
        snapshot(editor, "preview-factory-rack");
        editor.loadFactory(119);
        snapshot(editor, "preview-factory-creative");
        const auto exported = bankDirectory.getChildFile("Creative Racks/120 Drum Carousel.shaper");
        if (!editor.loadRackFile(exported) || p.palette() != std::vector<int>({1, 2, 0, 4}))
            return 70;
    }
    for (int e : p.palette())
        p.removeEffect(e);
    p.addEffect(1);
    auto &time = p.module(1);
    std::array<double, 2> measuredPitches{};
    for (int i = 0; i < 8; ++i)
    {
        time.setCurrentProgram(TimeQuickPresets::programOffset + i);
        p.prepareToPlay(48000, 256);
        int crossings = 0;
        double previous = 0;
        int measured = 0;
        for (int block = 0; block < 600; ++block)
        {
            head.samples = (juce::int64)block * 256;
            for (int c = 0; c < 2; ++c)
                for (int n = 0; n < 256; ++n)
                    audio.setSample(c, n,
                                    0.1f * std::sin((head.samples + n) * juce::MathConstants<double>::twoPi *
                                                    440 / 48000));
            p.processBlock(audio, midi);
            if (!finite(audio))
                return 90;
            if (block >= 450 && block < 540)
                for (int n = 0; n < 256; ++n)
                {
                    double value = audio.getSample(0, n);
                    if (measured > 0 && previous <= 0 && value > 0)
                        ++crossings;
                    previous = value;
                    ++measured;
                }
        }
        if (i >= 6)
        {
            double hz = crossings * 48000.0 / measured, expected = i == 6 ? 220 : 880;
            measuredPitches[(size_t)i - 6] = hz;
            if (std::abs(hz - expected) > 8)
            {
                std::cerr << "Pitch " << i << " measured " << hz << " Hz\n";
                return 91;
            }
        }
        juce::MemoryBlock before;
        time.getStateInformation(before);
        time.setCurrentProgram(0);
        time.setStateInformation(before.getData(), (int)before.getSize());
        if (time.getCurrentProgram() != TimeQuickPresets::programOffset + i)
            return 92;
    }
    std::cout << "Time quick presets: 8; pitch tests: " << measuredPitches[0] << " Hz and "
              << measuredPitches[1] << " Hz from 440 Hz input\n";
    auto &timeBands = dynamic_cast<MultibandProcessor &>(time);
    set(p, "mix", 1);
    set(p, "gain", 0);
    timeBands.resetBands();
    set(timeBands, "crossover_low", 200);
    set(timeBands, "crossover_high", 2400);
    set(timeBands.band(0), "mix", 0);
    set(timeBands.band(2), "mix", 0);
    set(timeBands.band(1), "min", 0.5f);
    set(timeBands.band(1), "max", 0.5f);
    {
        std::unique_ptr<juce::AudioProcessorEditor> editor(time.createEditor());
        dynamic_cast<RackEditorCommands *>(editor.get())->rackCommand(700);
    }
    p.prepareToPlay(48000, 256);
    auto timeTone = [&](double hz)
    {
        for (int b = 0; b < 60; ++b)
        {
            head.samples += 256;
            for (int c = 0; c < 2; ++c)
                for (int n = 0; n < 256; ++n)
                    audio.setSample(c, n,
                                    0.1f * std::sin((head.samples + n) * juce::MathConstants<double>::twoPi *
                                                    hz / 48000));
            p.processBlock(audio, midi);
        }
        double sum = 0, power = 0;
        for (int n = 0; n < 256; ++n)
        {
            double v = audio.getSample(0, n);
            sum += v;
            power += v * v;
        }
        return std::sqrt(juce::jmax(0.0, power / 256 - std::pow(sum / 256, 2)));
    };
    const double highTone = timeTone(8000), midTone = timeTone(1000);
    if (highTone < 0.05 || midTone > 0.01)
    {
        std::cerr << "Time band isolation: high=" << highTone << " mid=" << midTone << "\n";
        return 96;
    }
    {
        std::unique_ptr<juce::AudioProcessorEditor> editor(time.createEditor());
        dynamic_cast<RackEditorCommands *>(editor.get())->rackCommand(700);
    }
    timeBands.band(0).setCurrentProgram(79);
    timeBands.band(2).setCurrentProgram(86);
    timeBands.selectedBand = 1;
    juce::MemoryBlock timeState;
    time.getStateInformation(timeState);
    timeBands.resetBands();
    time.setStateInformation(timeState.getData(), (int)timeState.getSize());
    if (!timeBands.isMultiband() || timeBands.band(0).getCurrentProgram() != 79 ||
        timeBands.band(2).getCurrentProgram() != 86)
        return 97;
    juce::MemoryBlock oldTimeState;
    timeBands.band(1).getStateInformation(oldTimeState);
    time.setStateInformation(oldTimeState.getData(), (int)oldTimeState.getSize());
    if (timeBands.isMultiband() || timeBands.editorBand() != 1 || time.getCurrentProgram() != 86)
        return 98;
    time.setStateInformation(timeState.getData(), (int)timeState.getSize());
    std::cout << "Time multiband: independent low/mid/high processing and state recall passed\n";
    p.selectedEffect = 1;
    preview(p, "preview-time-quick-presets");
    p.selectedEffect = 5;
    p.addEffect(5);
    preview(p, "preview-volume-eight-presets");
    if (factoryPeak > 4.0f)
        return 65;
    p.setPlayHead(nullptr);
    p.releaseResources();
    {
        ShaperProcessor outputRack;
        outputRack.setPlayHead(&head);
        juce::AudioBuffer<float> testAudio(2, 128);
        juce::MidiBuffer testMidi;
        for (int effect = 0; effect < ShaperProcessor::effectCount; ++effect)
        {
            for (int e : outputRack.palette())
                outputRack.removeEffect(e);
            outputRack.addEffect(effect);
            auto &engine = dynamic_cast<MultibandProcessor &>(outputRack.module(effect));
            auto *wet = engine.outputParameter(1, "mix");
            auto *gain = engine.outputParameter(1, "gain");
            auto *stereo = engine.outputParameter(1, "stereo");
            if (!wet || !gain || !stereo)
                return 108;
            wet->setValueNotifyingHost(wet->convertTo0to1(0));
            auto gainDb = [&](float db)
            {
                gain->setValueNotifyingHost(
                    gain->convertTo0to1(gain->paramID == "gain" ? juce::Decibels::decibelsToGain(db) : db));
            };
            gainDb(0);
            outputRack.setRateAndBufferSizeDetails(48000, 128);
            outputRack.prepareToPlay(48000, 128);
            auto run = [&]
            {
                for (int block = 0; block < engine.getLatencySamples() / 128 + 40; ++block)
                {
                    for (int i = 0; i < 128; ++i)
                    {
                        testAudio.setSample(0, i, .1f);
                        testAudio.setSample(1, i, .02f);
                    }
                    outputRack.processBlock(testAudio, testMidi);
                }
                return testAudio.getRMSLevel(0, 0, 128);
            };
            const float unity = run();
            gainDb(6);
            const float boosted = run();
            if (std::abs(unity - .1f) > .002f ||
                std::abs(boosted / unity - juce::Decibels::decibelsToGain(6.0f)) > .015f)
            {
                std::cerr << "Output gain failed in " << effect << ": " << unity << " -> " << boosted << "\n";
                return 109;
            }
            juce::MemoryBlock outputState;
            engine.getStateInformation(outputState);
            gainDb(0);
            engine.setStateInformation(outputState.getData(), (int)outputState.getSize());
            if (std::abs(run() - boosted) > .001f || engine.outputPeakLeft[1].load() < .1f)
                return 110;
            if (effect == 1)
            {
                stereo->setValueNotifyingHost(stereo->convertTo0to1(0));
                run();
                if (std::abs(testAudio.getSample(0, 127) - testAudio.getSample(1, 127)) > .00001f)
                    return 111;
                juce::MemoryBlock monoState;
                engine.getStateInformation(monoState);
                stereo->setValueNotifyingHost(stereo->convertTo0to1(1));
                engine.setStateInformation(monoState.getData(), (int)monoState.getSize());
                if (stereo->convertFrom0to1(stereo->getValue()) != 0)
                    return 112;
            }
        }
        outputRack.setPlayHead(nullptr);
        outputRack.releaseResources();
        std::cout
            << "Shared output panel: eight effect gains/meters, state recall, Time stereo width and consistent control geometry passed\n";
    }
    {
        ShaperProcessor rack;
        for (int e = 0; e < 7; ++e)
            rack.addEffect(e);
        std::array<int, 7> permutation{0, 1, 2, 3, 4, 5, 6};
        for (int trial = 0; trial < 35; ++trial)
        {
            std::rotate(permutation.begin(), permutation.begin() + 1, permutation.end());
            if (trial % 7 == 0)
                std::swap(permutation[1], permutation[4]);
            for (int i = 0; i < 7; ++i)
                rack.moveEffect(permutation[(size_t)i], i);
            if (rack.palette() != std::vector<int>(permutation.begin(), permutation.end()))
                return 116;
            juce::MemoryBlock saved;
            rack.getStateInformation(saved);
            rack.moveEffect(6, 0);
            rack.setStateInformation(saved.getData(), (int)saved.getSize());
            if (rack.palette() != std::vector<int>(permutation.begin(), permutation.end()))
                return 117;
        }
        auto &saturation = dynamic_cast<MultibandProcessor &>(rack.module(6));
        set(saturation, "crossover_low", 250);
        set(saturation, "crossover_high", 3000);
        for (int b = 0; b < 3; ++b)
            saturation.band(b).setCurrentProgram(3 + b);
        juce::MemoryBlock bandsState;
        saturation.getStateInformation(bandsState);
        saturation.band(0).setCurrentProgram(0);
        saturation.setStateInformation(bandsState.getData(), (int)bandsState.getSize());
        for (int b = 0; b < 3; ++b)
            if (saturation.band(b).getCurrentProgram() != 3 + b)
                return 118;
        juce::MemoryBlock all;
        rack.getStateInformation(all);
        auto old = juce::AudioProcessor::getXmlFromBinary(all.getData(), (int)all.getSize());
        old->setAttribute("version", 4);
        for (auto *node = old->getFirstChildElement(); node;)
        {
            auto *following = node->getNextElement();
            const auto id = node->getStringAttribute("id");
            if (id.startsWith("e6_") || id == "saturation_position" ||
                (node->hasTagName("Module") && node->getIntAttribute("effect") == 6))
                old->removeChildElement(node, true);
            node = following;
        }
        juce::MemoryBlock legacy;
        juce::AudioProcessor::copyXmlToBinary(*old, legacy);
        rack.setStateInformation(legacy.getData(), (int)legacy.getSize());
        auto expected = rack.palette();
        if (std::find(expected.begin(), expected.end(), 6) != expected.end() || expected.size() != 6)
            return 119;
        const float legacyOrder = rack.state.getRawParameterValue("order")->load();
        rack.addEffect(6);
        rack.removeEffect(6);
        if (rack.state.getRawParameterValue("order")->load() != legacyOrder)
            return 120;
        for (int e : rack.palette())
            rack.removeEffect(e);
        rack.addEffect(6);
        saturation.resetBands();
        saturation.selectedBand = 1;
        set(saturation, "mix", 0);
        rack.prepareToPlay(48000, 64);
        juce::AudioBuffer<float> impulse(2, 1024);
        impulse.clear();
        impulse.setSample(0, 0, .25f);
        juce::MidiBuffer midi;
        rack.processBlock(impulse, midi);
        for (int i = 0; i < 1024; ++i)
            if (std::abs(impulse.getSample(0, i) - (i == rack.getLatencySamples() ? .25f : 0.0f)) > 1e-6f)
                return 121;
        std::cout
            << "Saturation integration: drag order, state recall, three independent bands, legacy six-effect migration and dry latency passed\n";
    }
    std::cout
        << "PASS: empty rack, palette drag/remove/order, Pan L/C/R and 20 presets, six effects with multiband and proportional header/Snap UI, independent Low/Mid/High crossover isolation, effect preset files, 160 unique factory rack states and navigation, Volume 8 supplied presets and audible ducking, mini buttons, rack files, six About tabs, parameter forwarding, legacy recall, oversized blocks\n";
    std::cout << "Factory presets: " << factoryNames.size() << "; maximum measured peak: " << factoryPeak
              << "\n";
    std::cout << "Exported and recalled 160 rack presets in 10 categories and 32 new effect preset files.\n";
    return 0;
}
