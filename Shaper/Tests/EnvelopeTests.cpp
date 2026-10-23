// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 Fazıl Irak; Shaper eklemeleri/değişiklikleri.
// Fazıl Irak tarafından oluşturuldu / değiştirildi: 23.10.2026
// Kapsam: Shaper için eklenen kod ve uyarlamalar; özgün üçüncü taraf telif/lisansları korunur.
// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: EnvelopeTests regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.
#include "EnvelopeEffect.h"
#include "EnvelopeEditor.h"
#include "Saturation/PluginProcessor.h"
#include "Pitch/PitchProcessor.h"
#include <iostream>
#if JUCE_WINDOWS
#include <windows.h>
#endif
// set: EnvelopeTests regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir. Burada verilen ayar türetilmiş duruma/çıktıya aktarılır; çağrı sırası aşağıdaki gövdededir.
static void set(juce::AudioProcessor &p, const char *id, float v)
{
    for (auto *param : p.getParameters())
        if (auto *r = dynamic_cast<juce::RangedAudioParameter *>(param))
            if (r->paramID == id)
            {
                r->setValueNotifyingHost(r->convertTo0to1(v));
                return;
            }
    throw std::runtime_error(id);
}
// find alt adımı: EnvelopeTests regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.
static juce::Component *find(juce::Component &c, const juce::String &id)
{
    if (c.getComponentID() == id)
        return &c;
    for (auto *child : c.getChildren())
        if (auto *match = find(*child, id))
            return match;
    return nullptr;
}
struct Head : juce::AudioPlayHead
{
    juce::int64 time = 0;
    // getPosition: EnvelopeTests regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
    juce::Optional<PositionInfo> getPosition() const override
    {
        PositionInfo p;
        p.setIsPlaying(true);
        p.setBpm(120);
        p.setTimeInSamples(time);
        p.setTimeInSeconds(time / 48000.0);
        p.setPpqPosition(time / 24000.0);
        return p;
    }
};
// main alt adımı: EnvelopeTests regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.
// Gövdedeki başlıca çağrılar: SetErrorMode → p → p.setRateAndBufferSizeDetails → p.prepareToPlay. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
int main()
{
#if JUCE_WINDOWS
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
#endif
    juce::ScopedJuceInitialiser_GUI gui;
    juce::MidiBuffer midi;
    for (bool isPitch : {false, true})
    {
        EnvelopeEffect p(isPitch);
        p.setRateAndBufferSizeDetails(48000, 256);
        p.prepareToPlay(48000, 256);
        std::unique_ptr<juce::AudioProcessorEditor> editor(p.createEditor());
        auto *commands = dynamic_cast<RackEditorCommands *>(editor.get());
        if (!commands || !commands->rackCanEdit() || !find(*editor, "effect-envelope-view") ||
            !find(*editor, "effect-tempo") || !find(*editor, "effect-pattern-sync"))
            return 1;
        const char *satIds[]{"drive", "tone", "lowcut", "highcut", "bias", "dynamics"};
        const char *pitchIds[]{"semitones", "cents", "formant", "formant_base"};
        for (int i = 0; i < (isPitch ? 4 : 6); ++i)
        {
            auto *k = dynamic_cast<shaperui::RackKnob *>(find(
                *editor, (isPitch ? "pitch-" : "irak-") + juce::String(isPitch ? pitchIds[i] : satIds[i])));
            if (!k || k->getWidth() != 68 || k->getHeight() != 64)
                return 2;
        }
        if (isPitch)
        {
            auto *tab = dynamic_cast<juce::Button *>(find(*editor, "formant-curve-tab"));
            if (!tab)
                return 3;
            tab->onClick();
            if (p.selectedCurve != 1)
                return 4;
        }
        for (int command : {53, 55, 56, 57, 52, 54})
            commands->rackCommand(command);
        editor.reset();
        if (isPitch)
        {
            set(*p.curves[0], "phase", .25f);
            set(*p.curves[1], "phase", .75f);
            juce::MemoryBlock state;
            p.getStateInformation(state);
            set(*p.curves[0], "phase", 0);
            set(*p.curves[1], "phase", 0);
            p.setStateInformation(state.getData(), (int)state.getSize());
            for (int c = 0; c < 2; ++c)
            {
                bool found = false;
                for (auto *param : p.curves[(size_t)c]->getParameters())
                    if (auto *ranged = dynamic_cast<juce::RangedAudioParameter *>(param))
                        if (ranged->paramID == "phase")
                        {
                            found = true;
                            if (std::abs(ranged->convertFrom0to1(ranged->getValue()) -
                                         (c == 0 ? .25f : .75f)) > 1e-5f)
                                return 11;
                        }
                if (!found)
                    return 12;
            }
        }
        std::vector<juce::String> motionStates;
        Head motionHead;
        p.setPlayHead(&motionHead);
        for (int i = 0; i < p.getNumPrograms(); ++i)
        {
            p.setCurrentProgram(i);
            juce::MemoryBlock before;
            p.getStateInformation(before);
            p.setCurrentProgram((i + 1) % p.getNumPrograms());
            p.setStateInformation(before.getData(), (int)before.getSize());
            if (p.getCurrentProgram() != i)
                return 5;
            if (i >= 24)
            {
                auto xml = juce::AudioProcessor::getXmlFromBinary(before.getData(), (int)before.getSize());
                xml->removeAttribute("program");
                const auto fingerprint = xml->toString();
                if (std::find(motionStates.begin(), motionStates.end(), fingerprint) != motionStates.end())
                    return 15;
                motionStates.push_back(fingerprint);
                juce::MemoryBlock again;
                p.getStateInformation(again);
                if (before != again)
                    return 16;
                p.prepareToPlay(48000, 256);
                juce::AudioBuffer<float> audio(2, 256);
                for (int block = 0; block < 64; ++block)
                {
                    motionHead.time = block * 256;
                    for (int c = 0; c < 2; ++c)
                        for (int n = 0; n < 256; ++n)
                            audio.setSample(c, n, .1f * std::sin((block * 256 + n) * .0576f));
                    p.processBlock(audio, midi);
                    for (int c = 0; c < 2; ++c)
                        for (int n = 0; n < 256; ++n)
                            if (!std::isfinite(audio.getSample(c, n)))
                                return 17;
                }
            }
        }
        p.setPlayHead(nullptr);
        juce::MemoryBlock legacy;
        p.native->getStateInformation(legacy);
        p.setStateInformation(legacy.getData(), (int)legacy.getSize());
        if (p.state.getRawParameterValue("curve_depth")->load() != 0)
            return 6;
        p.setCurrentProgram(0);
        set(*p.native, "mix", 0);
        p.prepareToPlay(48000, 256);
        const int latency = p.getLatencySamples();
        if (latency != p.native->getLatencySamples() + 720)
            return 13;
        juce::AudioBuffer<float> impulse(2, 256);
        for (int block = 0; block < (latency + 512) / 256 + 1; ++block)
        {
            impulse.clear();
            if (block == 0)
                impulse.setSample(0, 0, .25f);
            p.processBlock(impulse, midi);
            for (int i = 0; i < 256; ++i)
                if (std::abs(impulse.getSample(0, i) - (block * 256 + i == latency ? .25f : 0)) > 1e-6f)
                    return 14;
        }
        std::cout
            << (isPitch ? "Pitch" : "Saturation")
            << ": Gate editor and commands, standard knob geometry, 40 presets and legacy recall passed\n";
    }
    {
        EnvelopeEffect pitch(true);
        Head head;
        pitch.setPlayHead(&head);
        pitch.setRateAndBufferSizeDetails(48000, 256);
        for (float value : {.25f, .5f, .75f})
        {
            set(*pitch.curves[0], "min", value);
            set(*pitch.curves[0], "max", value);
            pitch.prepareToPlay(48000, 256);
            juce::AudioBuffer<float> b(2, 256);
            int crossings = 0, count = 0;
            float previous = 0;
            for (int block = 0; block < 220; ++block)
            {
                head.time = block * 256;
                for (int i = 0; i < 256; ++i)
                    for (int c = 0; c < 2; ++c)
                        b.setSample(c, i,
                                    .1f * (float)std::sin((head.time + i) *
                                                          juce::MathConstants<double>::twoPi * 440 / 48000));
                pitch.processBlock(b, midi);
                if (block > 110)
                    for (int i = 0; i < 256; ++i)
                    {
                        const float v = b.getSample(0, i);
                        if (!std::isfinite(v))
                            return 7;
                        if (count && previous <= 0 && v > 0)
                            ++crossings;
                        previous = v;
                        ++count;
                    }
            }
            const double hz = crossings * 48000.0 / count, expected = value == .25f  ? 220
                                                                      : value == .5f ? 440
                                                                                     : 880;
            std::cout << "Pitch curve " << value << ": " << hz << " Hz / expected " << expected << "\n";
            if (std::abs(hz - expected) > 4)
                return 8;
        }
        pitch.applyShape(1);
        set(*pitch.curves[0], "min", 0);
        set(*pitch.curves[0], "max", 1);
        pitch.prepareToPlay(48000, 256);
        juce::AudioBuffer<float> b(2, 256);
        std::array<int, 2> crossings{}, counts{};
        std::array<float, 2> previous{};
        for (int block = 0; block < 340; ++block)
        {
            head.time = block * 256;
            for (int i = 0; i < 256; ++i)
                for (int c = 0; c < 2; ++c)
                    b.setSample(c, i,
                                .1f * (float)std::sin((head.time + i) * juce::MathConstants<double>::twoPi *
                                                      440 / 48000));
            pitch.processBlock(b, midi);
            for (int w = 0; w < 2; ++w)
                if (block > (w == 0 ? 110 : 280) && block < (w == 0 ? 150 : 320))
                    for (int i = 0; i < 256; ++i)
                    {
                        float v = b.getSample(0, i);
                        if (counts[(size_t)w] && previous[(size_t)w] <= 0 && v > 0)
                            ++crossings[(size_t)w];
                        previous[(size_t)w] = v;
                        ++counts[(size_t)w];
                    }
        }
        const double start = crossings[0] * 48000.0 / counts[0], end = crossings[1] * 48000.0 / counts[1];
        std::cout << "Drawn rise: " << start << " -> " << end << " Hz\n";
        if (end < start * 2)
            return 9;
        pitch.setPlayHead(nullptr);
    }
    {
        EnvelopeEffect sat(false);
        Head head;
        sat.setPlayHead(&head);
        set(*sat.native, "drive", 24);
        set(*sat.native, "auto_gain", 0);
        sat.setRateAndBufferSizeDetails(48000, 256);
        std::array<double, 2> power{};
        for (int curve = 0; curve < 2; ++curve)
        {
            set(*sat.curves[0], "min", (float)curve);
            set(*sat.curves[0], "max", (float)curve);
            sat.prepareToPlay(48000, 256);
            juce::AudioBuffer<float> b(2, 256);
            for (int block = 0; block < 120; ++block)
            {
                head.time = block * 256;
                for (int i = 0; i < 256; ++i)
                    for (int c = 0; c < 2; ++c)
                        b.setSample(c, i, .1f * (float)std::sin((head.time + i) * .0576));
                sat.processBlock(b, midi);
                if (block > 60)
                    for (int i = 0; i < 256; ++i)
                        power[(size_t)curve] += b.getSample(0, i) * b.getSample(0, i);
            }
        }
        std::cout << "Saturation curve energy ratio: " << power[1] / power[0] << "\n";
        if (power[1] < power[0] * 2)
            return 10;
        sat.setPlayHead(nullptr);
    }
    // Deliver notifications queued before the temporary curve editors/processors
    // were destroyed. Closing a tab must not leave a callback with a dangling UI.
    juce::MessageManager::callAsync([] { juce::MessageManager::getInstance()->stopDispatchLoop(); });
    juce::MessageManager::getInstance()->runDispatchLoop();
    std::cout
        << "PASS: drawable saturation, audible pitch ramps, independent pitch/formant UI, standard controls and pending UI callback lifetime\n";
}
