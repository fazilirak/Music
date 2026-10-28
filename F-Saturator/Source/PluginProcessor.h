// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (c) 2026 Fazıl Irak
// Geliştirici: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: JUCE/DAW callback sözleşmesi ve APVTS parametre/durum sahipliği.
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "SaturationDSP.h"

namespace irak
{
// DAW ile ses motoru arasındaki köprü: parametreler, bus düzeni, presetler ve oturum durumu.
// Ses formülleri SaturationDSP içinde; görünüm PluginEditor içindedir.
class Processor final : public juce::AudioProcessor
{
    // Kalıcı parametre kimliklerini ve aralıklarını tek noktada tanımlar.
    juce::AudioProcessorValueTreeState::ParameterLayout layout();
    // Seçili fabrika presetinin indeksi; kullanıcı dosyası yüklendiğinde XML içinden geri alınır.
    int program = 0;

  public:
    Processor();
    // APVTS hem GUI otomasyonunu hem oturum kaydını yönetir; IRAK_STATE adını koruyun.
    juce::AudioProcessorValueTreeState state;
    SaturationDSP dsp;
    // Parametre atomiklerini fiziksel değerlerle okuyarak DSP ayarlarına dönüştürür.
    Settings settings() const;
    // Host örnekleme hızını ve blok kapasitesini bildirir; motor gecikmesi DAW’a iletilir.
    void prepareToPlay(double, int) override;
    // Host kaynağı bırakma callback’i burada bilinçli olarak boştur; DSP belleği nesne/yeniden prepare yaşam döngüsündedir.
    void releaseResources() override {}
    // Host transport/reset isteğini aynı DSP reset’ine iletir; ses geçmişi temizlenir, APVTS parametreleri korunur.
    void reset() override
    {
        dsp.reset();
    }
    // Ses callback’i. MIDI kullanılmaz; mono veya stereo ses yerinde işlenir.
    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;
    bool isBusesLayoutSupported(const BusesLayout &) const override;
    const juce::String getName() const override
    {
        return "F-Saturator";
    }
    // Bu efekt MIDI almıyor/üretmiyor. Yalnız UI’de klavye veya kontrol eklemek host MIDI yeteneğini değiştirmez.
    bool acceptsMidi() const override
    {
        return false;
    }
    bool producesMidi() const override
    {
        return false;
    }
    // Host’a bildirilen kuyruk 0,5 saniyedir; sinyalin gerçekten sıfıra yaklaşması sessiz kuyruk testiyle ayrıca kontrol edilir.
    double getTailLengthSeconds() const override
    {
        return .5;
    }
    bool hasEditor() const override
    {
        return true;
    }
    juce::AudioProcessorEditor *createEditor() override;
    // 16 fabrika presetinin DAW program arayüzü; isim ve değer dizileriyle birlikte güncellenmelidir.
    int getNumPrograms() override
    {
        return 16;
    }
    int getCurrentProgram() override
    {
        return program;
    }
    void setCurrentProgram(int) override;
    const juce::String getProgramName(int) override;
    // Host program adı değiştirme isteği desteklenmez; fabrika isimleri getProgramName dizisinden gelir.
    void changeProgramName(int, const juce::String &) override {}
    // DAW oturumları ve .irakpreset dosyaları için aynı XML/binary durum biçimi kullanılır.
    void getStateInformation(juce::MemoryBlock &) override;
    void setStateInformation(const void *, int) override;
};
// Aynı motoru başka bir uygulama içinde oluşturmak için yardımcı giriş; bağımsız derlemeyi etkilemez.
juce::AudioProcessor *createEmbeddedProcessor();
juce::Component *createAboutPage();
} // namespace irak
