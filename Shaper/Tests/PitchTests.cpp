// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 Fazıl Irak; Shaper eklemeleri/değişiklikleri.
// Fazıl Irak tarafından oluşturuldu / değiştirildi: 23.10.2026
// Kapsam: Shaper için eklenen kod ve uyarlamalar; özgün üçüncü taraf telif/lisansları korunur.
// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: PitchTests regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.
#include "Pitch/PitchProcessor.h"
#include "Processor.h"
#include "MultibandProcessor.h"
#include <iostream>
#include <numeric>
#include <random>
#if JUCE_WINDOWS
#include <windows.h>
#endif
// set: PitchTests regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir. Burada verilen ayar türetilmiş duruma/çıktıya aktarılır; çağrı sırası aşağıdaki gövdededir.
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
// finite alt adımı: PitchTests regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.
static bool finite(const juce::AudioBuffer<float> &b)
{
    for (int c = 0; c < 2; ++c)
        for (int i = 0; i < b.getNumSamples(); ++i)
            if (!std::isfinite(b.getSample(c, i)) || std::abs(b.getSample(c, i)) > 2)
                return false;
    return true;
}
// main alt adımı: PitchTests regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.
// Gövdedeki başlıca çağrılar: SetErrorMode → p.setRateAndBufferSizeDetails → p.prepareToPlay → p.getLatencySamples. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
int main()
{
#if JUCE_WINDOWS
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
#endif
    juce::ScopedJuceInitialiser_GUI gui;
    juce::MidiBuffer midi;
    for (double sr : {44100.0, 48000.0, 96000.0})
    {
        pitchfx::Processor p;
        p.setRateAndBufferSizeDetails(sr, 256);
        p.prepareToPlay(sr, 256);
        const int latency = p.getLatencySamples();
        // No resampling of the timeline: equal input/output lengths, original-rate
        // sample positions, and exact delay at both unity and dry mix.
        juce::AudioBuffer<float> impulse(2, latency + 2048);
        impulse.clear();
        impulse.setSample(0, 0, .25f);
        impulse.setSample(1, 0, -.1f);
        p.processBlock(impulse, midi);
        for (int c = 0; c < 2; ++c)
            for (int i = 0; i < impulse.getNumSamples(); ++i)
                if (std::abs(impulse.getSample(c, i) - (i == latency ? (c == 0 ? .25f : -.1f) : 0)) > 1e-6)
                    return 1;
        set(p, "semitones", 12);
        set(p, "mix", 0);
        p.reset();
        impulse.clear();
        impulse.setSample(0, 0, .25f);
        p.processBlock(impulse, midi);
        for (int i = 0; i < impulse.getNumSamples(); ++i)
            if (std::abs(impulse.getSample(0, i) - (i == latency ? .25f : 0)) > 1e-6)
                return 2;
        set(p, "mix", 1);
        for (float shift : {-24.0f, -12.0f, -5.0f, -.5f, .5f, 7.0f, 12.0f, 24.0f})
        {
            const float coarse = std::trunc(shift);
            set(p, "semitones", coarse);
            set(p, "cents", (shift - coarse) * 100);
            p.reset();
            juce::AudioBuffer<float> b(2, 257);
            int crossings = 0, count = 0;
            float previous = 0;
            double power = 0, stereoError = 0;
            const int blocks = (int)std::ceil(sr * 1.2 / 257);
            for (int block = 0; block < blocks; ++block)
            {
                for (int i = 0; i < 257; ++i)
                {
                    float sample = .1f * (float)std::sin(juce::MathConstants<double>::twoPi * 440 *
                                                         (block * 257 + i) / sr);
                    b.setSample(0, i, sample);
                    b.setSample(1, i, sample);
                }
                p.processBlock(b, midi);
                if (!finite(b))
                    return 3;
                if (block * 257 > sr * .55)
                    for (int i = 0; i < 257; ++i)
                    {
                        float v = b.getSample(0, i);
                        if (count && previous <= 0 && v > 0)
                            ++crossings;
                        previous = v;
                        ++count;
                        power += v * v;
                        stereoError += std::abs(v - b.getSample(1, i));
                    }
            }
            const double expected = 440 * std::pow(2.0, shift / 12), measured = crossings * sr / count,
                         rms = std::sqrt(power / count);
            std::cout << sr << " Hz / " << shift << " st: " << measured << " Hz (expected " << expected
                      << "), RMS " << rms << ", stereo error " << stereoError / count << ", latency "
                      << latency << "\n";
            // FFT phase prediction can differ slightly between equal channels;
            // require the mean stereo residual to stay below -60 dB of the tone.
            if (std::abs(measured - expected) > 3 || rms < .025 || rms > .15 ||
                stereoError / count > rms * .001 || p.getLatencySamples() != latency)
                return 4;
        }
        // Fine detuning and formant correction use the same fixed-latency engine.
        for (int block = 0; block < 160; ++block)
        {
            set(p, "semitones", (float)((block % 49) - 24));
            set(p, "cents", (float)(block % 80) - 40);
            set(p, "formant", (float)(block % 25) - 12);
            set(p, "preserve_formants", (float)(block % 2));
            juce::AudioBuffer<float> b(2, 128);
            for (int i = 0; i < 128; ++i)
            {
                const double phase = (block * 128 + i) * juce::MathConstants<double>::twoPi * 160 / sr;
                b.setSample(0, i, .07f * (float)(std::sin(phase) + .5 * std::sin(phase * 3)));
                b.setSample(1, i, 0);
            }
            p.processBlock(b, midi);
            if (!finite(b) || p.getLatencySamples() != latency)
                return 5;
        }
        juce::AudioBuffer<float> silence(2, 256);
        for (int i = 0; i < (int)sr / 256; ++i)
        {
            silence.clear();
            p.processBlock(silence, midi);
            if (!finite(silence))
                return 6;
        }
        if (silence.getMagnitude(0, 0, 256) > 1e-5)
            return 7;
        p.releaseResources();
    }
    {
        pitchfx::Processor p;
        for (int i = 0; i < 16; ++i)
        {
            p.setCurrentProgram(i);
            juce::MemoryBlock state;
            p.getStateInformation(state);
            p.setCurrentProgram((i + 1) % 16);
            p.setStateInformation(state.getData(), (int)state.getSize());
            if (p.getCurrentProgram() != i)
                return 8;
            juce::MemoryBlock restored;
            p.getStateInformation(restored);
            if (restored != state)
                return 9;
        }
    }
    {
        ShaperProcessor rack;
        std::array<int, 8> order{0, 1, 2, 3, 4, 5, 6, 7};
        std::mt19937 random(42);
        for (int e : order)
            rack.addEffect(e);
        for (int trial = 0; trial < 100; ++trial)
        {
            std::shuffle(order.begin(), order.end(), random);
            for (int i = 0; i < 8; ++i)
                rack.moveEffect(order[(size_t)i], i);
            const std::vector<int> expected(order.begin(), order.end());
            if (rack.palette() != expected)
                return 10;
            juce::MemoryBlock state;
            rack.getStateInformation(state);
            rack.removeEffect(7);
            rack.setStateInformation(state.getData(), (int)state.getSize());
            if (rack.palette() != expected)
                return 11;
        }
        // Seven-effect v5 files never silently acquire a Pitch effect.
        juce::MemoryBlock current;
        rack.getStateInformation(current);
        auto xml = juce::AudioProcessor::getXmlFromBinary(current.getData(), (int)current.getSize());
        xml->setAttribute("version", 5);
        juce::MemoryBlock legacy;
        juce::AudioProcessor::copyXmlToBinary(*xml, legacy);
        rack.setStateInformation(legacy.getData(), (int)legacy.getSize());
        auto expected = rack.palette();
        if (expected.size() != 7 || std::find(expected.begin(), expected.end(), 7) != expected.end())
            return 12;
        auto &pitch = dynamic_cast<MultibandProcessor &>(rack.module(7));
        set(pitch, "crossover_low", 250);
        set(pitch, "crossover_high", 3500);
        for (int b = 0; b < 3; ++b)
            pitch.band(b).setCurrentProgram(b + 1);
        juce::MemoryBlock bands;
        pitch.getStateInformation(bands);
        pitch.resetBands();
        pitch.band(0).setCurrentProgram(0);
        pitch.setStateInformation(bands.getData(), (int)bands.getSize());
        if (!pitch.isMultiband())
            return 13;
        for (int b = 0; b < 3; ++b)
            if (pitch.band(b).getCurrentProgram() != b + 1)
                return 14;
        for (int e : rack.palette())
            rack.removeEffect(e);
        rack.addEffect(7);
        rack.setRateAndBufferSizeDetails(48000, 128);
        rack.prepareToPlay(48000, 128);
        juce::AudioBuffer<float> audio(2, 1025);
        for (int block = 0; block < 60; ++block)
        {
            for (int i = 0; i < 1025; ++i)
            {
                const float x = .05f * (float)std::sin((block * 1025 + i) * .09);
                audio.setSample(0, i, x);
                audio.setSample(1, i, x * .7f);
            }
            rack.processBlock(audio, midi);
            if (!finite(audio))
                return 15;
        }
        if (audio.getMagnitude(0, 0, 1025) < .005)
            return 16;
        std::cout
            << "Pitch: 16 preset recalls, 100 eight-effect order/state permutations, v5 migration, three independent band recalls and oversized multiband rendering passed\n";
    }
    std::cout
        << "PASS: Pitch frequency, unity null, exact dry latency, stereo coherence, automation and silence at 44.1/48/96 kHz\n";
    return 0;
}
