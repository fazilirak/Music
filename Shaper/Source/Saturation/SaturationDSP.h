// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 Fazıl Irak; Shaper eklemeleri/değişiklikleri.
// Fazıl Irak tarafından oluşturuldu / değiştirildi: 23.10.2026
// Kapsam: Shaper için eklenen kod ve uyarlamalar; özgün üçüncü taraf telif/lisansları korunur.
// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: Doygunluk karakterleri, oversampling, ADAA, dinamik hafıza, filtre ve hizalı dry/wet zincirini uygular.
#pragma once
#include <juce_dsp/juce_dsp.h>
#include <array>

namespace irak
{
inline constexpr int modelCount = 5;
inline constexpr const char *modelNames[]{"Console", "Transformer", "Tape", "Triode", "Pentode"};
struct Settings
{
    int model = 0;
    float drive = 6, tone = 0, lowCut = 20, highCut = 20000, bias = 0, dynamics = .3f, mix = 1, output = 0,
          width = 1;
    bool heat = false, autoGain = true, bypass = false;
};

// Original hybrid models: see Research/DSP-RESEARCH.md for sources and limits.
// No Soundtoys, AP Mastering or SwankyAmp source is linked into this engine.
class SaturationDSP
{
    struct ADAA
    {
        double previous = 0;
        static double logCosh(double x);
        static double shape(double x, double k, double bias);
        double process(double x, double k, double bias, bool enabled);
    };
    struct Magnet
    {
        double field = 0, magnetisation = 0;
        static double langevin(double x);
        static double langevinDerivative(double x);
        static double slope(double h, double m, double direction, double a, double k);
        double process(double h, bool tape);
    };
    struct Model
    {
        ADAA first, second;
        Magnet magnetic;
        double record = 0, playback = 0, bass = 0, grid = 0;
        double process(double x, int model, const std::array<double, 5> &rates, double bias, double dynamics,
                       double sag, bool adaa);
    };
    std::array<std::array<Model, 2>, modelCount> models;
    std::array<juce::SmoothedValue<float>, modelCount> weights;
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampling;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> wetFractional{16};
    juce::AudioBuffer<float> wet, dryDelay;
    int capacity = 0, factor = 8, latency = 0, dryPosition = 0, lastModel = 0;
    double sampleRate = 48000, internalRate = 384000;
    bool antialias = true;
    std::array<double, 2> highpass{}, highpassInput{}, lowpass{}, toneLow{}, dcInput{}, dcOutput{};
    double envelope = 0;
    std::array<double, 5> modelRates{};
    float previousLow = -1, previousHigh = -1, previousTone = 2;
    double highpassPole = 0, lowpassAmount = 0, toneBassGain = 1, toneTrebleGain = 1;
    juce::SmoothedValue<float> drive, tone, lowCut, highCut, bias, dynamics, mix, output, width, autoLevel,
        bypass;
    bool primed = false;
    void render(juce::AudioBuffer<float> &, int offset, int count, const Settings &,
                const float *modulation = nullptr, float depth = 0);

  public:
    void prepare(double sr, int maximumBlock, int oversamplingStages = 3, bool useADAA = true);
    void reset();
    void process(juce::AudioBuffer<float> &, const Settings &);
    void processModulated(juce::AudioBuffer<float> &, const Settings &, const float *envelope, float depth);
    // getLatency: Doygunluk karakterleri, oversampling, ADAA, dinamik hafıza, filtre ve hizalı dry/wet zincirini uygular. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
    int getLatency() const
    {
        return latency;
    }
    static float compensationDb(int model, float drive);
    static float preview(float x, int model, float bias = 0);
    std::atomic<float> inputPeak{0}, outputPeak{0}, activity{0};
};
} // namespace irak
