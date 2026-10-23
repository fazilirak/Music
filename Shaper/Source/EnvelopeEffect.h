// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 Fazıl Irak; Shaper eklemeleri/değişiklikleri.
// Fazıl Irak tarafından oluşturuldu / değiştirildi: 23.10.2026
// Kapsam: Shaper için eklenen kod ve uyarlamalar; özgün üçüncü taraf telif/lisansları korunur.
// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: Bağımsız efekt motorunu tempo, tetik, sequencer ve envelope modülasyonuyla sarar.
#pragma once
#include <JuceHeader.h>
#include "ForwardParameter.h"

// A Shaper-owned curve processor surrounds each native effect. It reuses the
// Gate editor, sequencer and trigger engine; the native DSP remains independent.
class EnvelopeEffect final : public juce::AudioProcessor
{
  public:
    explicit EnvelopeEffect(bool pitch);
    ~EnvelopeEffect() override;
    std::unique_ptr<juce::AudioProcessor> native;
    std::array<std::unique_ptr<juce::AudioProcessor>, 2> curves;
    juce::AudioProcessorValueTreeState state;
    const bool isPitch;
    int selectedCurve = 0;
    // &curve() alt adımı: Bağımsız efekt motorunu tempo, tetik, sequencer ve envelope modülasyonuyla sarar.
    juce::AudioProcessor &curve()
    {
        return *curves[(size_t)selectedCurve];
    }
    bool audioSettingsShown();
    void applyShape(int kind);
    void forceCV();
    void prepareToPlay(double, int) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;
    bool isBusesLayoutSupported(const BusesLayout &) const override;
    // getName: Bağımsız efekt motorunu tempo, tetik, sequencer ve envelope modülasyonuyla sarar. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
    const juce::String getName() const override
    {
        return isPitch ? "Pitch" : "Saturation";
    }
    // acceptsMidi alt adımı: Bağımsız efekt motorunu tempo, tetik, sequencer ve envelope modülasyonuyla sarar.
    bool acceptsMidi() const override
    {
        return true;
    }
    // producesMidi alt adımı: Bağımsız efekt motorunu tempo, tetik, sequencer ve envelope modülasyonuyla sarar.
    bool producesMidi() const override
    {
        return false;
    }
    // getTailLengthSeconds: Bağımsız efekt motorunu tempo, tetik, sequencer ve envelope modülasyonuyla sarar. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
    double getTailLengthSeconds() const override
    {
        return native->getTailLengthSeconds();
    }
    // hasEditor alt adımı: Bağımsız efekt motorunu tempo, tetik, sequencer ve envelope modülasyonuyla sarar.
    bool hasEditor() const override
    {
        return true;
    }
    juce::AudioProcessorEditor *createEditor() override;
    // Host’a program adedini bildirir; gezinme/sınır ve fabrika bankasıyla tutarlı kalmalıdır.
    int getNumPrograms() override
    {
        return 40;
    }
    // Seçili program indeksini host’a bildirir; gösterim adı kalıcı kimlik değildir.
    int getCurrentProgram() override
    {
        return program;
    }
    void setCurrentProgram(int) override;
    const juce::String getProgramName(int) override;
    // changeProgramName: Bağımsız efekt motorunu tempo, tetik, sequencer ve envelope modülasyonuyla sarar. Burada verilen ayar türetilmiş duruma/çıktıya aktarılır; çağrı sırası aşağıdaki gövdededir.
    void changeProgramName(int, const juce::String &) override {}
    void getStateInformation(juce::MemoryBlock &) override;
    void setStateInformation(const void *, int) override;

  private:
    std::array<juce::AudioBuffer<float>, 2> modulation;
    std::array<juce::MidiBuffer, 2> events;
    juce::AudioBuffer<float> inputDelay, curveDelay;
    int delayPosition = 0, lookahead = 0;
    int capacity = 0, program = 0;
    static juce::AudioProcessorValueTreeState::ParameterLayout layout();
};
