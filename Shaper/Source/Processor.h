// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 Fazıl Irak; Shaper eklemeleri/değişiklikleri.
// Fazıl Irak tarafından oluşturuldu / değiştirildi: 23.10.2026
// Kapsam: Shaper için eklenen kod ve uyarlamalar; özgün üçüncü taraf telif/lisansları korunur.
// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: Rack efekt zinciri, MIDI/transport, preset, ses callback ve kaydedilen durum akışını yönetir.
#pragma once
#include <JuceHeader.h>
#include <array>
#include <atomic>

class ShaperProcessor final : public juce::AudioProcessor, private juce::AsyncUpdater
{
  public:
    ShaperProcessor();
    ~ShaperProcessor() override;
    void prepareToPlay(double, int) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;
    bool isBusesLayoutSupported(const BusesLayout &) const override;
    juce::AudioProcessorEditor *createEditor() override;
    // hasEditor alt adımı: Rack efekt zinciri, MIDI/transport, preset, ses callback ve kaydedilen durum akışını yönetir.
    bool hasEditor() const override
    {
        return true;
    }
    // getName: Rack efekt zinciri, MIDI/transport, preset, ses callback ve kaydedilen durum akışını yönetir. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
    const juce::String getName() const override
    {
        return "Jellybone";
    }
    // acceptsMidi alt adımı: Rack efekt zinciri, MIDI/transport, preset, ses callback ve kaydedilen durum akışını yönetir.
    bool acceptsMidi() const override
    {
        return true;
    }
    // producesMidi alt adımı: Rack efekt zinciri, MIDI/transport, preset, ses callback ve kaydedilen durum akışını yönetir.
    bool producesMidi() const override
    {
        return false;
    }
    // isMidiEffect alt adımı: Rack efekt zinciri, MIDI/transport, preset, ses callback ve kaydedilen durum akışını yönetir.
    bool isMidiEffect() const override
    {
        return false;
    }
    // getTailLengthSeconds: Rack efekt zinciri, MIDI/transport, preset, ses callback ve kaydedilen durum akışını yönetir. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
    double getTailLengthSeconds() const override
    {
        return 30.0;
    }
    // Host’a program adedini bildirir; gezinme/sınır ve fabrika bankasıyla tutarlı kalmalıdır.
    int getNumPrograms() override
    {
        return 1;
    }
    // Seçili program indeksini host’a bildirir; gösterim adı kalıcı kimlik değildir.
    int getCurrentProgram() override
    {
        return 0;
    }
    // Fabrika/kullanıcı program seçimini mevcut parametre/durum akışına uygular.
    void setCurrentProgram(int) override {}
    // Program indeksine karşılık gelen görünür adı döndürür; ad ve değer bankası aynı sırada olmalıdır.
    const juce::String getProgramName(int) override
    {
        return "Default";
    }
    // changeProgramName: Rack efekt zinciri, MIDI/transport, preset, ses callback ve kaydedilen durum akışını yönetir. Burada verilen ayar türetilmiş duruma/çıktıya aktarılır; çağrı sırası aşağıdaki gövdededir.
    void changeProgramName(int, const juce::String &) override {}
    void getStateInformation(juce::MemoryBlock &) override;
    void setStateInformation(const void *, int) override;
    static constexpr int effectCount = 8;
    static constexpr int orderCount = 720;
    // &module(int effect) alt adımı: Rack efekt zinciri, MIDI/transport, preset, ses callback ve kaydedilen durum akışını yönetir.
    juce::AudioProcessor &module(int effect)
    {
        return *modules[(size_t)effect];
    }
    juce::AudioProcessorValueTreeState state;
    // enableId alt adımı: Rack efekt zinciri, MIDI/transport, preset, ses callback ve kaydedilen durum akışını yönetir.
    static juce::String enableId(int e)
    {
        return "e" + juce::String(e) + "_enabled";
    }
    // presentId alt adımı: Rack efekt zinciri, MIDI/transport, preset, ses callback ve kaydedilen durum akışını yönetir.
    static juce::String presentId(int e)
    {
        return "e" + juce::String(e) + "_present";
    }
    std::vector<int> palette() const;
    void addEffect(int effect);
    void removeEffect(int effect);
    void moveEffect(int effect, int destination);
    static const char *effectName(int e);
    static const std::array<std::array<int, effectCount>, orderCount> &orders();
    int selectedEffect = -1;
    static constexpr int factoryPresetCount = 160;
    static juce::String factoryPresetName(int index);
    bool loadFactoryPreset(int index);
    juce::String rackPresetName;
    int factoryPresetIndex = -1;

  private:
    struct ChunkPlayHead : juce::AudioPlayHead
    {
        juce::AudioPlayHead *source = nullptr;
        int offset = 0;
        double sampleRate = 44100;
        // getPosition: Rack efekt zinciri, MIDI/transport, preset, ses callback ve kaydedilen durum akışını yönetir. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
        // Gövdedeki başlıca çağrılar: source->getPosition → position->getTimeInSamples → position->setTimeInSamples → position->getTimeInSeconds. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
        juce::Optional<PositionInfo> getPosition() const override
        {
            if (source == nullptr)
                return {};
            auto position = source->getPosition();
            if (position)
            {
                if (auto t = position->getTimeInSamples())
                    position->setTimeInSamples(*t + offset);
                if (auto t = position->getTimeInSeconds())
                    position->setTimeInSeconds(*t + offset / sampleRate);
                if (auto q = position->getPpqPosition())
                    if (auto bpm = position->getBpm())
                        position->setPpqPosition(*q + offset / sampleRate * (*bpm) / 60.0);
            }
            return position;
        }
    } chunkPlayHead;
    struct Delay
    {
        juce::AudioBuffer<float> ring;
        int write = 0;
        // Örnekleme hızı ve blok kapasitesine bağlı tampon/işleme durumunu ses başlamadan hazırlar.
        void prepare(int capacity)
        {
            ring.setSize(2, capacity);
            ring.clear();
            write = 0;
        }
        // Bir ses örneğini filtre/model hafızasından geçirir; bir sonraki örneğe taşınan durum bu adımda güncellenir.
        float sample(int channel, float input, int delay)
        {
            auto n = ring.getNumSamples();
            ring.setSample(channel, write, input);
            return ring.getSample(channel, (write + n - juce::jlimit(0, n - 1, delay)) % n);
        }
        // advance alt adımı: Rack efekt zinciri, MIDI/transport, preset, ses callback ve kaydedilen durum akışını yönetir.
        void advance()
        {
            write = (write + 1) % ring.getNumSamples();
        }
    };
    std::array<std::unique_ptr<juce::AudioProcessor>, effectCount> modules;
    juce::AudioBuffer<float> chain, scratch, dry;
    std::array<Delay, effectCount> bypassDelay;
    Delay dryDelay;
    std::array<juce::SmoothedValue<float>, effectCount> activeSmooth;
    std::array<juce::MidiBuffer, effectCount> moduleMidi;
    std::array<std::atomic<float> *, effectCount> enableValues{};
    std::array<std::atomic<float> *, effectCount> presentValues{};
    std::atomic<float> *mixValue{}, *gainValue{}, *orderValue{};
    std::atomic<float> *saturationPosition{};
    std::atomic<float> *pitchPosition{};
    std::array<int, effectCount> signalOrder() const;
    void applyPaletteOrder(const std::vector<int> &);
    juce::SmoothedValue<float> mixSmooth, gainSmooth;
    std::atomic<int> requestedLatency{0};
    int blockCapacity = 0;
    double rate = 44100;
    juce::MemoryBlock initialState;
    void handleAsyncUpdate() override;
    void render(juce::AudioBuffer<float> &, juce::MidiBuffer &, int, int);
    static juce::AudioProcessorValueTreeState::ParameterLayout layout();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ShaperProcessor)
};
