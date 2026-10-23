// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 Fazıl Irak; Shaper eklemeleri/değişiklikleri.
// Fazıl Irak tarafından oluşturuldu / değiştirildi: 23.10.2026
// Kapsam: Shaper için eklenen kod ve uyarlamalar; özgün üçüncü taraf telif/lisansları korunur.
// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: Envelope biçimlerini ve hazır desen verilerini saklar; sıra/alanlar kaydedilen durumla uyumludur.
#pragma once
#include <JuceHeader.h>
#include <cmath>

// Original Shaper presets: no third-party preset files or curves are embedded.
namespace envelopePresets
{
// name alt adımı: Envelope biçimlerini ve hazır desen verilerini saklar; sıra/alanlar kaydedilen durumla uyumludur.
// Gövdedeki başlıca çağrılar: juce::jlimit. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
inline const char *name(bool pitch, int i)
{
    static constexpr const char *saturation[]{
        "Motion / Warm Quarter",      "Motion / Tape Breathing",   "Motion / Triode Eighths",
        "Motion / Transformer Steps", "Motion / Pentode Accents",  "Motion / Soft Swell",
        "Motion / Dark Decay",        "Motion / Triplet Grit",     "Motion / Offbeat Heat",
        "Motion / Broken Drive",      "Motion / Gentle Bus Pulse", "Motion / Bass Growl",
        "Motion / Bright Rise",       "Motion / Parallel Bite",    "Motion / Slow Magnetic",
        "Motion / Last Beat Burn"};
    static constexpr const char *pitches[]{
        "Motion / Gentle Vibrato", "Motion / Tape Wow",         "Motion / Octave Bounce",
        "Motion / Fifth Arpeggio", "Motion / Downward Glide",   "Motion / Upward Glide",
        "Motion / Falling Octave", "Motion / Rising Fifth",     "Motion / Minor Steps",
        "Motion / Major Steps",    "Motion / Formant Drift",    "Motion / Talking Formants",
        "Motion / Parallel Fifth", "Motion / Low Octave Pulse", "Motion / Triplet Pitch",
        "Motion / Final Rise"};
    return (pitch ? pitches : saturation)[juce::jlimit(0, 15, i)];
}
// set: Envelope biçimlerini ve hazır desen verilerini saklar; sıra/alanlar kaydedilen durumla uyumludur. Burada verilen ayar türetilmiş duruma/çıktıya aktarılır; çağrı sırası aşağıdaki gövdededir.
inline void set(juce::AudioProcessor &engine, const char *id, float value)
{
    for (auto *p : engine.getParameters())
        if (auto *r = dynamic_cast<juce::RangedAudioParameter *>(p))
            if (r->paramID == id)
            {
                r->setValueNotifyingHost(r->convertTo0to1(value));
                return;
            }
}
// Örnek/filtre/faz gibi çalışma hafızasını yeniden başlangıca alır; parametre tanımıyla aynı işlem değildir.
inline void reset(juce::AudioProcessor &engine)
{
    for (auto *p : engine.getParameters())
        p->setValueNotifyingHost(p->getDefaultValue());
}
// value alt adımı: Envelope biçimlerini ve hazır desen verilerini saklar; sıra/alanlar kaydedilen durumla uyumludur.
// Gövdedeki başlıca çağrılar: std::sin → juce::jmin → std::cos → std::floor. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
inline float value(bool pitch, int index, double x, bool formant)
{
    const double pi = juce::MathConstants<double>::pi;
    if (pitch)
    {
        double semitones = 0;
        if (formant)
        {
            if (index == 10)
                semitones = 2.5 * std::sin(2 * pi * x);
            if (index == 11)
                semitones = std::sin(4 * pi * x) >= 0 ? 3 : -3;
            return (float)(.5 + semitones / 24);
        }
        switch (index)
        {
        case 0:
            semitones = .18 * std::sin(8 * pi * x);
            break;
        case 1:
            semitones = .35 * std::sin(2 * pi * x) + .08 * std::sin(10 * pi * x);
            break;
        case 2:
            semitones = ((int)(x * 4) % 2) ? 12 : 0;
            break;
        case 3:
        {
            constexpr double notes[]{0, 7, 12, 7};
            semitones = notes[juce::jmin(3, (int)(x * 4))];
            break;
        }
        case 4:
            semitones = -7 * x;
            break;
        case 5:
            semitones = 7 * x;
            break;
        case 6:
            semitones = -12 * x;
            break;
        case 7:
            semitones = 7 * (.5 - .5 * std::cos(2 * pi * x));
            break;
        case 8:
        {
            constexpr double notes[]{0, 3, 7, 12, 7, 3, 0, -5};
            semitones = notes[juce::jmin(7, (int)(x * 8))];
            break;
        }
        case 9:
        {
            constexpr double notes[]{0, 4, 7, 12, 7, 4, 0, 7};
            semitones = notes[juce::jmin(7, (int)(x * 8))];
            break;
        }
        case 10:
        case 11:
            break;
        case 12:
            semitones = x < .5 ? 0 : 7;
            break;
        case 13:
            semitones = ((int)(x * 4) % 2) ? 0 : -12;
            break;
        case 14:
        {
            constexpr double notes[]{0, 7, 12};
            semitones = notes[juce::jmin(2, (int)(x * 3))];
            break;
        }
        case 15:
            semitones = x < .75 ? 0 : 12 * (x - .75) * 4;
            break;
        }
        return (float)(.5 + semitones / 48);
    }
    const double pulse = x * 4 - std::floor(x * 4);
    switch (index)
    {
    case 0:
        return (float)(.25 + .75 * std::exp(-5 * pulse));
    case 1:
        return (float)(.45 + .35 * std::sin(2 * pi * x));
    case 2:
        return ((int)(x * 8) % 2) ? .25f : 1.f;
    case 3:
        return .25f + .2f * juce::jmin(3, (int)(x * 4));
    case 4:
        return pulse < .2 ? 1.f : .2f;
    case 5:
        return (float)(.15 + .85 * (.5 - .5 * std::cos(2 * pi * x)));
    case 6:
        return (float)(.1 + .9 * std::exp(-4 * x));
    case 7:
        return ((int)(x * 6) % 2) ? .2f : .9f;
    case 8:
        return pulse < .5 ? .15f : 1.f;
    case 9:
    {
        constexpr float levels[]{.9f, .2f, .4f, .8f, .15f, .6f, 1.f, .3f};
        return levels[juce::jmin(7, (int)(x * 8))];
    }
    case 10:
        return (float)(.7 + .15 * std::sin(2 * pi * x));
    case 11:
        return (float)(.45 + .5 * (.5 - .5 * std::cos(4 * pi * x)));
    case 12:
        return (float)(.15 + .85 * x);
    case 13:
        return pulse < .35 ? 1.f : .1f;
    case 14:
        return (float)(.4 + .45 * std::sin(pi * x));
    default:
        return x < .75 ? .15f : (float)(.15 + .85 * (x - .75) * 4);
    }
}
// curve alt adımı: Envelope biçimlerini ve hazır desen verilerini saklar; sıra/alanlar kaydedilen durumla uyumludur.
// Gövdedeki başlıca çağrılar: engine.getStateInformation → juce::AudioProcessor::getXmlFromBinary → data.getData → data.getSize. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
inline void curve(juce::AudioProcessor &engine, bool pitch, int index, bool formant = false)
{
    juce::MemoryBlock data;
    engine.getStateInformation(data);
    if (auto xml = juce::AudioProcessor::getXmlFromBinary(data.getData(), (int)data.getSize()))
    {
        juce::String points;
        const bool stepped = pitch ? (index == 2 || index == 3 || index == 8 || index == 9 || index == 12 ||
                                      index == 13 || index == 14 || (formant && index == 11))
                                   : (index == 2 || index == 3 || index == 4 || index == 7 || index == 8 ||
                                      index == 9 || index == 13);
        for (int i = 0; i <= 96; ++i)
        {
            const double x = i / 96.;
            points += juce::String(x, 9) + " " + juce::String(1 - value(pitch, index, x, formant), 9) +
                      " 0 " + (stepped ? "0 " : "1 ");
        }
        for (int i = 0; i < 12; ++i)
            xml->setAttribute("pattern" + juce::String(i), points);
        xml->setAttribute("currpattern", 1);
        xml->setAttribute("currentProgram", 0);
        xml->setAttribute("alwaysPlaying", false);
        juce::AudioProcessor::copyXmlToBinary(*xml, data);
        engine.setStateInformation(data.getData(), (int)data.getSize());
    }
    set(engine, "pattern", 1);
    set(engine, "trigger", 0);
    set(engine, "sync",
        pitch ? (index == 0                  ? 7
                 : index == 1 || index == 14 ? 11
                                             : 9)
              : (index == 14 ? 11 : 9));
    set(engine, "min", 0);
    set(engine, "max", 1);
    set(engine, "phase", 0);
    set(engine, "mix", 1);
    set(engine, "attack", .003f);
    set(engine, "release", .008f);
}
} // namespace envelopePresets
