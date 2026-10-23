// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 Fazıl Irak; Shaper eklemeleri/değişiklikleri.
// Fazıl Irak tarafından oluşturuldu / değiştirildi: 23.10.2026
// Kapsam: Shaper için eklenen kod ve uyarlamalar; özgün üçüncü taraf telif/lisansları korunur.
// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: Pitch/formant motorunu Signalsmith işleme durumuyla bağlar; gecikme, reset ve kanal durumunu taşır.
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace pitchfx
{
class Processor final : public juce::AudioProcessor
{
  public:
    Processor();
    ~Processor() override;
    juce::AudioProcessorValueTreeState state;
    void prepareToPlay(double, int) override;
    void releaseResources() override;
    void reset() override;
    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;
    void processModulated(juce::AudioBuffer<float> &, const float *pitchCurve, const float *formantCurve,
                          float depth);
    bool isBusesLayoutSupported(const BusesLayout &) const override;
    // getName: Pitch/formant motorunu Signalsmith işleme durumuyla bağlar; gecikme, reset ve kanal durumunu taşır. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
    const juce::String getName() const override
    {
        return "Pitch";
    }
    // acceptsMidi alt adımı: Pitch/formant motorunu Signalsmith işleme durumuyla bağlar; gecikme, reset ve kanal durumunu taşır.
    bool acceptsMidi() const override
    {
        return false;
    }
    // producesMidi alt adımı: Pitch/formant motorunu Signalsmith işleme durumuyla bağlar; gecikme, reset ve kanal durumunu taşır.
    bool producesMidi() const override
    {
        return false;
    }
    // getTailLengthSeconds: Pitch/formant motorunu Signalsmith işleme durumuyla bağlar; gecikme, reset ve kanal durumunu taşır. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
    double getTailLengthSeconds() const override
    {
        return .2;
    }
    // hasEditor alt adımı: Pitch/formant motorunu Signalsmith işleme durumuyla bağlar; gecikme, reset ve kanal durumunu taşır.
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
        return program.load();
    }
    void setCurrentProgram(int) override;
    const juce::String getProgramName(int) override;
    // changeProgramName: Pitch/formant motorunu Signalsmith işleme durumuyla bağlar; gecikme, reset ve kanal durumunu taşır. Burada verilen ayar türetilmiş duruma/çıktıya aktarılır; çağrı sırası aşağıdaki gövdededir.
    void changeProgramName(int, const juce::String &) override {}
    void getStateInformation(juce::MemoryBlock &) override;
    void setStateInformation(const void *, int) override;

  private:
    struct Engine;
    std::unique_ptr<Engine> engine;
    std::atomic<int> program{0};
    static juce::AudioProcessorValueTreeState::ParameterLayout layout();
};
juce::AudioProcessor *createEmbeddedProcessor();
juce::Component *createAboutPage();
} // namespace pitchfx
