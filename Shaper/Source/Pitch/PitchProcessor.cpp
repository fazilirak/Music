// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 Fazıl Irak; Shaper eklemeleri/değişiklikleri.
// Fazıl Irak tarafından oluşturuldu / değiştirildi: 23.10.2026
// Kapsam: Shaper için eklenen kod ve uyarlamalar; özgün üçüncü taraf telif/lisansları korunur.
#include "../Embedded/RackAboutPage.h"
// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: Pitch/formant motorunu Signalsmith işleme durumuyla bağlar; gecikme, reset ve kanal durumunu taşır.
#include "PitchProcessor.h"
#include "signalsmith-stretch.h"

namespace pitchfx
{
namespace
{
struct Preset
{
    const char *name;
    float pitch, cents, formant, base, mix, width;
    bool preserve;
};
constexpr Preset presets[]{{"Init", 0, 0, 0, 160, 1, 1, false},
                           {"Octave Down", -12, 0, 0, 160, 1, 1, false},
                           {"Octave Up", 12, 0, 0, 160, 1, 1, false},
                           {"Fifth Above", 7, 0, 0, 160, 1, 1, false},
                           {"Fourth Below", -5, 0, 0, 160, 1, 1, false},
                           {"Double Octave Down", -24, 0, 0, 100, 1, 1, false},
                           {"Double Octave Up", 24, 0, 0, 160, 1, 1, false},
                           {"Octave Parallel", 12, 0, 0, 160, .5f, 1, false},
                           {"Warm Detune", 0, -7, 0, 160, .5f, 1.25f, false},
                           {"Bright Detune", 0, 7, 0, 160, .5f, 1.25f, false},
                           {"Vocal Up", 3, 0, 0, 200, 1, 1, true},
                           {"Vocal Down", -3, 0, 0, 140, 1, 1, true},
                           {"Deep Formant", 0, 0, -5, 140, 1, 1, false},
                           {"Air Formant", 0, 0, 4, 200, 1, 1, false},
                           {"Low Harmony", -7, 0, 0, 160, .4f, 1, false},
                           {"High Harmony", 4, 0, 0, 160, .4f, 1, false}};
} // namespace
struct Processor::Engine
{
    signalsmith::stretch::SignalsmithStretch<float> stretch{42};
    juce::AudioBuffer<float> input, wet, dryRing;
    juce::SmoothedValue<float> transpose, formant, mix, width, gain, spectral;
    int write = 0, latency = 0;
    bool ready = false;
    // Örnekleme hızı ve blok kapasitesine bağlı tampon/işleme durumunu ses başlamadan hazırlar.
    void prepare(double sr)
    {
        // Fixed quality and latency for every band, preset and automation value.
        stretch.presetDefault(2, (float)sr);
        latency = stretch.inputLatency() + stretch.outputLatency();
        input.setSize(2, 64);
        wet.setSize(2, 64);
        dryRing.setSize(2, latency + 1);
        for (auto *s : {&transpose, &formant, &mix, &width, &gain, &spectral})
            s->reset(sr, .025);
        ready = true;
    }
};
// Host parametre tanımlarını oluşturur; kimlik, aralık ve varsayılan sözleşmesini birlikte koruyun.
// Gövdedeki başlıca çağrılar: p.add → add. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
juce::AudioProcessorValueTreeState::ParameterLayout Processor::layout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout p;
    auto add = [&](const char *id, const char *name, float lo, float hi, float step, float def)
    {
        p.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{id, 1}, name,
                                                          juce::NormalisableRange<float>(lo, hi, step), def));
    };
    add("semitones", "Semitones", -24, 24, 1, 0);
    add("cents", "Fine cents", -100, 100, .1f, 0);
    add("formant", "Formant semitones", -12, 12, .1f, 0);
    add("formant_base", "Voice fundamental Hz", 60, 500, 1, 160);
    p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"preserve_formants", 1},
                                                     "Preserve formants", false));
    add("mix", "Mix", 0, 1, .001f, 1);
    add("width", "Stereo width", 0, 2, .001f, 1);
    add("output", "Output dB", -60, 12, .1f, 0);
    return p;
}
// Processor alt adımı: Pitch/formant motorunu Signalsmith işleme durumuyla bağlar; gecikme, reset ve kanal durumunu taşır.
Processor::Processor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      state(*this, nullptr, "PITCH_STATE", layout()), engine(std::make_unique<Engine>())
{
}
// ~Processor alt adımı: Pitch/formant motorunu Signalsmith işleme durumuyla bağlar; gecikme, reset ve kanal durumunu taşır.
Processor::~Processor() = default;
// Desteklenen giriş/çıkış kanal düzenini doğrular; synth ve FX giriş sözleşmeleri farklı olabilir.
bool Processor::isBusesLayoutSupported(const BusesLayout &l) const
{
    return l.getMainInputChannelSet() == juce::AudioChannelSet::stereo() &&
           l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}
// Host hazırlığında örnekleme, blok kapasitesi ve bildirilecek gecikmeyi kurar.
void Processor::prepareToPlay(double sr, int)
{
    engine->prepare(sr);
    setLatencySamples(engine->latency);
    reset();
}
// Host kaynak bırakma aşamasıdır; boşsa sahiplik nesne/yeniden hazırlık yaşam döngüsündedir.
void Processor::releaseResources()
{
    engine->ready = false;
}
// Örnek/filtre/faz gibi çalışma hafızasını yeniden başlangıca alır; parametre tanımıyla aynı işlem değildir.
// Gövdedeki başlıca çağrılar: e.stretch.reset → e.dryRing.clear → state.getRawParameterValue → load. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
void Processor::reset()
{
    auto &e = *engine;
    if (!e.ready)
        return;
    e.stretch.reset();
    e.dryRing.clear();
    e.write = 0;
    auto value = [&](const char *id) { return state.getRawParameterValue(id)->load(); };
    const float pitch = value("semitones") + value("cents") * .01f, f = value("formant");
    e.transpose.setCurrentAndTargetValue(pitch);
    e.formant.setCurrentAndTargetValue(f);
    e.mix.setCurrentAndTargetValue(value("mix"));
    e.width.setCurrentAndTargetValue(value("width"));
    e.gain.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(value("output")));
    e.spectral.setCurrentAndTargetValue(std::abs(pitch) > .0001f || std::abs(f) > .0001f ? 1.0f : 0.0f);
}
// Host audio callback’ini işler; burada dosya/GUI veya bekleyen kilit eklemeyin.
void Processor::processBlock(juce::AudioBuffer<float> &b, juce::MidiBuffer &)
{
    processModulated(b, nullptr, nullptr, 0);
}
// processModulated alt adımı: Pitch/formant motorunu Signalsmith işleme durumuyla bağlar; gecikme, reset ve kanal durumunu taşır.
// Gövdedeki başlıca çağrılar: b.getNumChannels → state.getRawParameterValue → load → value. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
void Processor::processModulated(juce::AudioBuffer<float> &b, const float *pitchCurve,
                                 const float *formantCurve, float depth)
{
    juce::ScopedNoDenormals guard;
    auto &e = *engine;
    if (!e.ready || b.getNumChannels() < 2)
        return;
    auto value = [&](const char *id) { return state.getRawParameterValue(id)->load(); };
    const float pitch = value("semitones") + value("cents") * .01f, f = value("formant");
    e.transpose.setTargetValue(pitch);
    e.formant.setTargetValue(f);
    e.mix.setTargetValue(value("mix"));
    e.width.setTargetValue(value("width"));
    e.gain.setTargetValue(juce::Decibels::decibelsToGain(value("output")));
    // At unity use an exactly delayed copy; keep the spectral engine warm.
    e.spectral.setTargetValue(std::abs(pitch) > .0001f || std::abs(f) > .0001f ? 1.0f : 0.0f);
    e.stretch.setFormantBase(value("formant_base") / (float)getSampleRate());
    for (int offset = 0; offset < b.getNumSamples(); offset += 64)
    {
        const int n = juce::jmin(64, b.getNumSamples() - offset);
        for (int c = 0; c < 2; ++c)
            e.input.copyFrom(c, 0, b, c, offset, n);
        const float pmod =
            pitchCurve ? (juce::jlimit(0.0f, 1.0f, pitchCurve[offset + n / 2]) - .5f) * 48 * depth : 0;
        const float fmod =
            formantCurve ? (juce::jlimit(0.0f, 1.0f, formantCurve[offset + n / 2]) - .5f) * 24 * depth : 0;
        e.spectral.setTargetValue(std::abs(pitch + pmod) > .0001f || std::abs(f + fmod) > .0001f ? 1.0f
                                                                                                 : 0.0f);
        e.stretch.setTransposeSemitones(juce::jlimit(-24.0f, 24.0f, e.transpose.skip(n) + pmod));
        e.stretch.setFormantSemitones(juce::jlimit(-12.0f, 12.0f, e.formant.skip(n) + fmod),
                                      value("preserve_formants") > .5f);
        e.stretch.process(e.input.getArrayOfReadPointers(), n, e.wet.getArrayOfWritePointers(), n);
        for (int i = 0; i < n; ++i)
        {
            const float amount = e.mix.getNextValue() * e.spectral.getNextValue(),
                        width = e.width.getNextValue(), gain = e.gain.getNextValue();
            float out[2];
            for (int c = 0; c < 2; ++c)
            {
                e.dryRing.setSample(c, e.write, e.input.getSample(c, i));
                const float dry = e.dryRing.getSample(c, (e.write + 1) % e.dryRing.getNumSamples());
                out[c] = dry + amount * (e.wet.getSample(c, i) - dry);
            }
            const float mid = (out[0] + out[1]) * .5f, side = (out[0] - out[1]) * .5f * width;
            b.setSample(0, offset + i, (mid + side) * gain);
            b.setSample(1, offset + i, (mid - side) * gain);
            e.write = (e.write + 1) % e.dryRing.getNumSamples();
        }
    }
}
// Program indeksine karşılık gelen görünür adı döndürür; ad ve değer bankası aynı sırada olmalıdır.
const juce::String Processor::getProgramName(int i)
{
    return presets[juce::jlimit(0, 15, i)].name;
}
// Fabrika/kullanıcı program seçimini mevcut parametre/durum akışına uygular.
// Gövdedeki başlıca çağrılar: juce::jlimit → state.getParameter → param->setValueNotifyingHost → param->convertTo0to1. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
void Processor::setCurrentProgram(int i)
{
    const auto &p = presets[juce::jlimit(0, 15, i)];
    const char *ids[]{"semitones", "cents", "formant",           "formant_base",
                      "mix",       "width", "preserve_formants", "output"};
    const float values[]{p.pitch, p.cents, p.formant, p.base, p.mix, p.width, p.preserve ? 1.0f : 0.0f, 0};
    for (int k = 0; k < 8; ++k)
    {
        auto *param = state.getParameter(ids[k]);
        param->setValueNotifyingHost(param->convertTo0to1(values[k]));
    }
    program.store(juce::jlimit(0, 15, i));
}
// Host oturumu için mevcut parametre ve kullanıcı durumunu seri hale getirir.
void Processor::getStateInformation(juce::MemoryBlock &b)
{
    auto xml = state.copyState().createXml();
    xml->setAttribute("program", program.load());
    xml->setAttribute("version", 1);
    copyXmlToBinary(*xml, b);
}
// Host/preset durumunu okuyup uygular; kök/sürüm denetimleri eski oturum uyumluluğu içindir.
void Processor::setStateInformation(const void *d, int n)
{
    if (auto xml = getXmlFromBinary(d, n))
        if (xml->hasTagName("PITCH_STATE"))
        {
            state.replaceState(juce::ValueTree::fromXml(*xml));
            program.store(juce::jlimit(0, 15, xml->getIntAttribute("program")));
        }
}
// Host’un sahipliğine geçecek editörü oluşturur; Processor editörden daha uzun yaşamalıdır.
juce::AudioProcessorEditor *Processor::createEditor()
{
    return nullptr;
}
// createEmbeddedProcessor alt adımı: Pitch/formant motorunu Signalsmith işleme durumuyla bağlar; gecikme, reset ve kanal durumunu taşır.
juce::AudioProcessor *createEmbeddedProcessor()
{
    return new Processor();
}
// createAboutPage alt adımı: Pitch/formant motorunu Signalsmith işleme durumuyla bağlar; gecikme, reset ve kanal durumunu taşır.
juce::Component *createAboutPage()
{
    return new shaperui::RackAboutPage("Pitch", "", juce::Colour(0xff54dfdb));
}

} // namespace pitchfx
