// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (c) 2026 Fazıl Irak
// Geliştirici: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: DSP sözleşmesi, fiziksel ayarlar, kanal/model hafızaları, hazırlanan tamponlar ve atomik gösterge alanları.
#pragma once
#include <juce_dsp/juce_dsp.h>
#include <array>

// Ses motoru: JUCE arayüzünden bağımsız durum, model ve tampon yönetimi.
// Örnek akışı ve her kontrolün etkisi için GELISTIRME-TR.md belgesine bakın.
namespace irak
{
// Model sırası preset ve DAW otomasyonunda kullanılır: Console, Transformer, Tape, Triode, Pentode.
// Mevcut sıralamayı değiştirmek kayıtlı projelerin sesini değiştirebilir.
inline constexpr int modelCount = 5;
inline constexpr const char *modelNames[]{"Console", "Transformer", "Tape", "Triode", "Pentode"};
// Bir işleme çağrısındaki parametre görünümü. dB/Hz değerleri fiziksel birimdedir;
// mix 0..1, width 0..2, tone ve bias -1..1 aralığındadır. Varsayılanlar layout ile uyumlu tutulmalıdır.
struct Settings
{
    // Model indeksi 0..4’tür. modelNames, fabrika presetleri ve UI karakter düğmeleri aynı sırayı kullanır.
    int model = 0;
    // Drive burada dB; doğrusal kazanca yalnız render içinde çevrilir. Filtre uç değerleri filtre bypass sözleşmesidir.
    float drive = 6, tone = 0, lowCut = 20, highCut = 20000, bias = 0, dynamics = .3f, mix = 1, output = 0,
          width = 1;
    bool heat = false, autoGain = true, bypass = false;
};

// Original hybrid models: see Research/DSP-RESEARCH.md for sources and limits.
// No Soundtoys, AP Mastering or SwankyAmp source is linked into this engine.
class SaturationDSP
{
    // Tanh tabanlı yumuşak kırpıcı. Önceki örneği saklayıp antitürev farkıyla aliasing azaltımı uygular.
    struct ADAA
    {
        // ADAA geçmiş örneği sıfırlanmadan yeni sinyal/karaktere taşınırsa geçişte yapay darbe oluşabilir.
        double previous = 0;
        static double logCosh(double x);
        static double shape(double x, double k, double bias);
        double process(double x, double k, double bias, bool enabled);
    };
    // Histerezis benzeri hafızalı manyetik dal. field ve magnetisation her kanal/model için ayrı saklanır.
    struct Magnet
    {
        // Manyetik alan ve mıknatıslanma ayrı hafıza durumlarıdır; statik transfer önizlemesi bu iki değeri takip etmez.
        double field = 0, magnetisation = 0;
        static double langevin(double x);
        static double langevinDerivative(double x);
        static double slope(double h, double m, double direction, double a, double k);
        double process(double h, bool tape);
    };
    // İki doğrusal olmayan kat, manyetik hafıza ve modelin filtre/grid durumları.
    // Bu durumları kanallar arasında paylaşmak istenmeyen kanal etkileşimi oluşturur.
    struct Model
    {
        ADAA first, second;
        Magnet magnetic;
        double record = 0, playback = 0, bass = 0, grid = 0;
        double process(double x, int model, const std::array<double, 5> &rates, double bias, double dynamics,
                       double sag, bool adaa);
    };
    // Her karakter ve iki ses kanalı için ayrı motor; model geçişleri weights ile çapraz soldurulur.
    std::array<std::array<Model, 2>, modelCount> models;
    std::array<juce::SmoothedValue<float>, modelCount> weights;
    // Varsayılan üç ikileme katı 8× örnekleme sağlar. Bellek prepare sırasında ayrılır.
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampling;
    // Wet kolundaki kesirli gecikmeyi tamamlar; dryDelay kuru kolu aynı tam örnek gecikmesine getirir.
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> wetFractional{16};
    juce::AudioBuffer<float> wet, dryDelay;
    // capacity hazırlanan blok kapasitesi, factor örnekleme çarpanı, latency DAW örneği cinsinden gecikmedir.
    int capacity = 0, factor = 8, latency = 0, dryPosition = 0, lastModel = 0;
    double sampleRate = 48000, internalRate = 384000;
    bool antialias = true;
    // Kanal başına Low Cut, High Cut, Tone ve DC engelleyici filtre hafızaları.
    std::array<double, 2> highpass{}, highpassInput{}, lowpass{}, toneLow{}, dcInput{}, dcOutput{};
    // İki kanalın ortak seviye zarfı Dynamics/sag etkisini sürer; stereo dengenin korunmasına yardımcı olur.
    double envelope = 0;
    std::array<double, 5> modelRates{};
    // Yalnız kontrol değiştiğinde yeniden hesaplanan filtre katsayıları için önbellek.
    float previousLow = -1, previousHigh = -1, previousTone = 2;
    double highpassPole = 0, lowpassAmount = 0, toneBassGain = 1, toneTrebleGain = 1;
    // Otomasyon sıçramalarını yumuşatan rampalar; işlem iç örnekleme hızına göre ilerletilir.
    juce::SmoothedValue<float> drive, tone, lowCut, highCut, bias, dynamics, mix, output, width, autoLevel,
        bypass;
    // İlk blokta değerler hedefe doğrudan yerleşir; sonraki değişikliklerde rampalar kullanılır.
    bool primed = false;
    void render(juce::AudioBuffer<float> &, int offset, int count, const Settings &);

  public:
    // Ses başlamadan çağrılır: örnekleme hızı, azami blok ve oversampling durumunu hazırlar.
    // Son iki seçenek ölçüm testlerinde karşılaştırma içindir; ürün varsayılanı 8× ve ADAA açıktır.
    void prepare(double sr, int maximumBlock, int oversamplingStages = 3, bool useADAA = true);
    // Filtre ve model hafızasını temizler; host reset veya yeni hazırlık sırasında kullanılır.
    void reset();
    // Yerinde ses işleme. Beklenenden büyük blokları kapasiteyi aşmadan alt bloklara böler.
    void process(juce::AudioBuffer<float> &, const Settings &);
    // Kuru/işlenmiş karışımın hizalanması için kullanılan ve DAW’a bildirilen tam örnek gecikmesi.
    // Dönüş birimi host örneğidir. DAW bildirimi ve dry/bypass darbe testi aynı gecikmeyi kullanmalıdır.
    int getLatency() const
    {
        return latency;
    }
    // Referans sinüse göre ölçülmüş Auto Gain tablosu; genel bir algısal ses yüksekliği eşitleyicisi değildir.
    static float compensationDb(int model, float drive);
    // Arayüz için yaklaşık statik transfer eğrisi; hafızalı modelin bütün dinamik davranışını göstermez.
    static float preview(float x, int model, float bias = 0);
    // Ses iş parçacığından arayüze tek yönlü seviye aktarımı; arayüz ses motorunu kilitlemez.
    std::atomic<float> inputPeak{0}, outputPeak{0}, activity{0};
};
} // namespace irak
