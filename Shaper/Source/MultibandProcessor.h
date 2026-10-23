// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 Fazıl Irak; Shaper eklemeleri/değişiklikleri.
// Fazıl Irak tarafından oluşturuldu / değiştirildi: 23.10.2026
// Kapsam: Shaper için eklenen kod ve uyarlamalar; özgün üçüncü taraf telif/lisansları korunur.
// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: Üç bağımsız efekt motoru, bant ayırma/birleştirme, bypass ve bant durumu saklamasını yönetir.
#pragma once
#include <JuceHeader.h>
#include <functional>
#include <array>

// Each effect has its own crossover and three independent envelope/DSP engines.
class MultibandProcessor final : public juce::AudioProcessor
{
  public:
    MultibandProcessor(std::function<juce::AudioProcessor *()> factory, int effect);
    ~MultibandProcessor() override;
    void prepareToPlay(double, int) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;
    bool isBusesLayoutSupported(const BusesLayout &) const override;
    juce::AudioProcessorEditor *createEditor() override;
    // hasEditor alt adımı: Üç bağımsız efekt motoru, bant ayırma/birleştirme, bypass ve bant durumu saklamasını yönetir.
    bool hasEditor() const override
    {
        return true;
    }
    const juce::String getName() const override;
    // acceptsMidi alt adımı: Üç bağımsız efekt motoru, bant ayırma/birleştirme, bypass ve bant durumu saklamasını yönetir.
    bool acceptsMidi() const override
    {
        return true;
    }
    // producesMidi alt adımı: Üç bağımsız efekt motoru, bant ayırma/birleştirme, bypass ve bant durumu saklamasını yönetir.
    bool producesMidi() const override
    {
        return false;
    }
    // getTailLengthSeconds: Üç bağımsız efekt motoru, bant ayırma/birleştirme, bypass ve bant durumu saklamasını yönetir. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
    double getTailLengthSeconds() const override
    {
        return 30;
    }
    // Host’a program adedini bildirir; gezinme/sınır ve fabrika bankasıyla tutarlı kalmalıdır.
    int getNumPrograms() override
    {
        return current().getNumPrograms();
    }
    // Seçili program indeksini host’a bildirir; gösterim adı kalıcı kimlik değildir.
    int getCurrentProgram() override
    {
        return current().getCurrentProgram();
    }
    void setCurrentProgram(int i) override;
    // Program indeksine karşılık gelen görünür adı döndürür; ad ve değer bankası aynı sırada olmalıdır.
    const juce::String getProgramName(int i) override
    {
        return current().getProgramName(i);
    }
    // changeProgramName: Üç bağımsız efekt motoru, bant ayırma/birleştirme, bypass ve bant durumu saklamasını yönetir. Burada verilen ayar türetilmiş duruma/çıktıya aktarılır; çağrı sırası aşağıdaki gövdededir.
    void changeProgramName(int i, const juce::String &s) override
    {
        current().changeProgramName(i, s);
    }
    void getStateInformation(juce::MemoryBlock &) override;
    void setStateInformation(const void *, int) override;
    // &band(int b) alt adımı: Üç bağımsız efekt motoru, bant ayırma/birleştirme, bypass ve bant durumu saklamasını yönetir.
    juce::AudioProcessor &band(int b)
    {
        return *engines[(size_t)b];
    }
    // &current() alt adımı: Üç bağımsız efekt motoru, bant ayırma/birleştirme, bypass ve bant durumu saklamasını yönetir.
    juce::AudioProcessor &current()
    {
        return band(editorBand());
    }
    // hasLowBand alt adımı: Üç bağımsız efekt motoru, bant ayırma/birleştirme, bypass ve bant durumu saklamasını yönetir.
    bool hasLowBand() const
    {
        return enabled->get() && low->get() > 20.5f;
    }
    // hasHighBand alt adımı: Üç bağımsız efekt motoru, bant ayırma/birleştirme, bypass ve bant durumu saklamasını yönetir.
    bool hasHighBand() const
    {
        return enabled->get() && high->get() < 19999.5f;
    }
    // isMultiband alt adımı: Üç bağımsız efekt motoru, bant ayırma/birleştirme, bypass ve bant durumu saklamasını yönetir.
    bool isMultiband() const
    {
        return hasLowBand() || hasHighBand();
    }
    // editorBand alt adımı: Üç bağımsız efekt motoru, bant ayırma/birleştirme, bypass ve bant durumu saklamasını yönetir.
    int editorBand() const
    {
        return selectedBand == 0 && !hasLowBand()    ? 1
               : selectedBand == 2 && !hasHighBand() ? 1
                                                     : selectedBand;
    }
    void setMultiband(bool value);
    void resetBands();
    juce::RangedAudioParameter *outputParameter(int band, const juce::String &kind);
    std::array<std::atomic<float>, 3> outputPeakLeft{}, outputPeakRight{};
    bool showBandsEditor = true;
    static constexpr int spectrumSize = 1024;
    bool readSpectrum(std::array<float, spectrumSize> &);
    std::atomic<bool> spectrumVisible{false};
    int selectedBand = 1;
    const int effect;
    juce::AudioParameterBool *enabled;
    juce::AudioParameterFloat *low, *high;

  private:
    std::array<std::unique_ptr<juce::AudioProcessor>, 3> engines;
    std::array<juce::AudioParameterFloat *, 3> outputGain{}, outputWidth{};
    std::array<juce::SmoothedValue<float>, 3> gainSmoothing, widthSmoothing;
    std::array<juce::AudioBuffer<float>, 3> audio;
    juce::dsp::LinkwitzRileyFilter<float> lowSplit, highSplit, lowPhase;
    juce::SmoothedValue<float> blend;
    juce::AudioBuffer<float> full;
    std::array<juce::MidiBuffer, 3> events;
    int capacity = 0;
    std::array<std::array<float, spectrumSize>, 4> spectrumFrames{};
    std::array<float, spectrumSize> spectrumPending{};
    int spectrumFill = 0;
    std::atomic<int> spectrumWrite{0}, spectrumRead{0};
};
