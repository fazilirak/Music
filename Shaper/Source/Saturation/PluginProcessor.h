// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 Fazıl Irak; Shaper eklemeleri/değişiklikleri.
// Fazıl Irak tarafından oluşturuldu / değiştirildi: 23.10.2026
// Kapsam: Shaper için eklenen kod ve uyarlamalar; özgün üçüncü taraf telif/lisansları korunur.
// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: Saturation parametrelerini, presetleri ve host callback’lerini kendi DSP motoruna bağlar.
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "SaturationDSP.h"

namespace irak
{
class Processor final : public juce::AudioProcessor
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout();
    int program = 0;

  public:
    Processor();
    juce::AudioProcessorValueTreeState state;
    SaturationDSP dsp;
    Settings settings() const;
    void prepareToPlay(double, int) override;
    // Host kaynak bırakma aşamasıdır; boşsa sahiplik nesne/yeniden hazırlık yaşam döngüsündedir.
    void releaseResources() override {}
    // Örnek/filtre/faz gibi çalışma hafızasını yeniden başlangıca alır; parametre tanımıyla aynı işlem değildir.
    void reset() override
    {
        dsp.reset();
    }
    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;
    bool isBusesLayoutSupported(const BusesLayout &) const override;
    // getName: Saturation parametrelerini, presetleri ve host callback’lerini kendi DSP motoruna bağlar. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
    const juce::String getName() const override
    {
        return "Saturation";
    }
    // acceptsMidi alt adımı: Saturation parametrelerini, presetleri ve host callback’lerini kendi DSP motoruna bağlar.
    bool acceptsMidi() const override
    {
        return false;
    }
    // producesMidi alt adımı: Saturation parametrelerini, presetleri ve host callback’lerini kendi DSP motoruna bağlar.
    bool producesMidi() const override
    {
        return false;
    }
    // getTailLengthSeconds: Saturation parametrelerini, presetleri ve host callback’lerini kendi DSP motoruna bağlar. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
    double getTailLengthSeconds() const override
    {
        return .5;
    }
    // hasEditor alt adımı: Saturation parametrelerini, presetleri ve host callback’lerini kendi DSP motoruna bağlar.
    bool hasEditor() const override
    {
        return false;
    }
    juce::AudioProcessorEditor *createEditor() override;
    // Host’a program adedini bildirir; gezinme/sınır ve fabrika bankasıyla tutarlı kalmalıdır.
    int getNumPrograms() override
    {
        return 16;
    }
    // Seçili program indeksini host’a bildirir; gösterim adı kalıcı kimlik değildir.
    int getCurrentProgram() override
    {
        return program;
    }
    void setCurrentProgram(int) override;
    const juce::String getProgramName(int) override;
    // changeProgramName: Saturation parametrelerini, presetleri ve host callback’lerini kendi DSP motoruna bağlar. Burada verilen ayar türetilmiş duruma/çıktıya aktarılır; çağrı sırası aşağıdaki gövdededir.
    void changeProgramName(int, const juce::String &) override {}
    void getStateInformation(juce::MemoryBlock &) override;
    void setStateInformation(const void *, int) override;
};
juce::AudioProcessor *createEmbeddedProcessor();
juce::Component *createAboutPage();
} // namespace irak
