// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 Fazıl Irak; Shaper eklemeleri/değişiklikleri.
// Fazıl Irak tarafından oluşturuldu / değiştirildi: 23.10.2026
// Kapsam: Shaper için eklenen kod ve uyarlamalar; özgün üçüncü taraf telif/lisansları korunur.
#include "../Embedded/RackAboutPage.h"
// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: Saturation parametrelerini, presetleri ve host callback’lerini kendi DSP motoruna bağlar.
#include "PluginProcessor.h"

namespace irak
{
// Host parametre tanımlarını oluşturur; kimlik, aralık ve varsayılan sözleşmesini birlikte koruyun.
// Gövdedeki başlıca çağrılar: p.add → add. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
juce::AudioProcessorValueTreeState::ParameterLayout Processor::layout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout p;
    p.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"model", 1}, "Character",
        juce::StringArray{"Console", "Transformer", "Tape", "Triode", "Pentode"}, 0));
    auto add = [&](const char *id, const char *name, float lo, float hi, float value, float skew = 1.0f)
    {
        p.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{id, 1}, name, juce::NormalisableRange<float>(lo, hi, .001f, skew), value));
    };
    add("drive", "Drive dB", 0, 36, 6);
    add("tone", "Tone", -1, 1, 0);
    add("lowcut", "Low cut Hz", 20, 500, 20, .4f);
    add("highcut", "High cut Hz", 2000, 20000, 20000, .4f);
    add("bias", "Bias", -1, 1, 0);
    add("dynamics", "Dynamics", 0, 1, .3f);
    add("mix", "Mix", 0, 1, 1);
    add("output", "Output dB", -60, 12, 0);
    add("width", "Stereo width", 0, 2, 1);
    for (const auto &entry : std::array<std::pair<const char *, bool>, 3>{
             {{"heat", false}, {"auto_gain", true}, {"bypass", false}}})
        p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{entry.first, 1}, entry.first,
                                                         entry.second));
    return p;
}
// Processor alt adımı: Saturation parametrelerini, presetleri ve host callback’lerini kendi DSP motoruna bağlar.
Processor::Processor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      state(*this, nullptr, "IRAK_STATE", layout())
{
}
// Host parametrelerinin mevcut değerlerini DSP’nin fiziksel kontrol modeline çevirir.
// Gövdedeki başlıca çağrılar: state.getRawParameterValue → load → get. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
Settings Processor::settings() const
{
    auto get = [&](const char *id) { return state.getRawParameterValue(id)->load(); };
    Settings s;
    s.model = (int)get("model");
    s.drive = get("drive");
    s.tone = get("tone");
    s.lowCut = get("lowcut");
    s.highCut = get("highcut");
    s.bias = get("bias");
    s.dynamics = get("dynamics");
    s.mix = get("mix");
    s.output = get("output");
    s.width = get("width");
    s.heat = get("heat") > .5f;
    s.autoGain = get("auto_gain") > .5f;
    s.bypass = get("bypass") > .5f;
    return s;
}
// Host hazırlığında örnekleme, blok kapasitesi ve bildirilecek gecikmeyi kurar.
void Processor::prepareToPlay(double sr, int n)
{
    dsp.prepare(sr, n);
    setLatencySamples(dsp.getLatency());
}
// Host audio callback’ini işler; burada dosya/GUI veya bekleyen kilit eklemeyin.
void Processor::processBlock(juce::AudioBuffer<float> &audio, juce::MidiBuffer &)
{
    juce::ScopedNoDenormals guard;
    dsp.process(audio, settings());
}
// Desteklenen giriş/çıkış kanal düzenini doğrular; synth ve FX giriş sözleşmeleri farklı olabilir.
bool Processor::isBusesLayoutSupported(const BusesLayout &l) const
{
    return (l.getMainInputChannelSet() == juce::AudioChannelSet::mono() ||
            l.getMainInputChannelSet() == juce::AudioChannelSet::stereo()) &&
           l.getMainInputChannelSet() == l.getMainOutputChannelSet();
}
// Host’un sahipliğine geçecek editörü oluşturur; Processor editörden daha uzun yaşamalıdır.
juce::AudioProcessorEditor *Processor::createEditor()
{
    return nullptr;
}
// Program indeksine karşılık gelen görünür adı döndürür; ad ve değer bankası aynı sırada olmalıdır.
const juce::String Processor::getProgramName(int i)
{
    static const char *names[]{"Clean Desk",    "Warm Bus",      "Tape Glue",      "Drum Heat",
                               "Vocal Edge",    "Bass Weight",   "Parallel Crush", "Bright Console",
                               "Soft Keys",     "Vintage Break", "Triode Body",    "Pentode Bite",
                               "Magnetic Bass", "Tape Air",      "Dark Overdrive", "Total Burn"};
    return names[juce::jlimit(0, 15, i)];
}
// Fabrika/kullanıcı program seçimini mevcut parametre/durum akışına uygular.
// Gövdedeki başlıca çağrılar: juce::jlimit → state.getParameter → p->setValueNotifyingHost → p->convertTo0to1. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
void Processor::setCurrentProgram(int index)
{
    index = juce::jlimit(0, 15, index);
    struct Preset
    {
        int model;
        float drive, tone, low, high, bias, dynamics, mix;
        bool heat;
    };
    static constexpr Preset presets[]{{0, 1, 0, 20, 20000, 0, .1f, 1, false},
                                      {0, 9, -.15f, 25, 18000, 0, .35f, 1, false},
                                      {2, 10, -.1f, 20, 16000, 0, .45f, 1, false},
                                      {3, 18, .05f, 35, 18000, .1f, .65f, 1, false},
                                      {3, 12, .2f, 85, 15000, .12f, .3f, .8f, false},
                                      {1, 12, -.2f, 20, 12500, 0, .45f, 1, false},
                                      {4, 22, 0, 100, 11000, 0, .7f, .28f, true},
                                      {0, 11, .45f, 40, 20000, 0, .2f, .85f, false},
                                      {2, 6, -.25f, 30, 13500, 0, .35f, .75f, false},
                                      {2, 20, -.35f, 40, 10000, .05f, .7f, 1, false},
                                      {3, 9, -.12f, 30, 17000, .16f, .5f, 1, false},
                                      {4, 19, .2f, 90, 15000, 0, .5f, 1, false},
                                      {1, 18, -.15f, 20, 9000, -.08f, .65f, .8f, false},
                                      {2, 8, .35f, 25, 19000, 0, .25f, 1, false},
                                      {3, 26, -.65f, 100, 6500, .25f, .8f, .7f, true},
                                      {4, 32, .3f, 120, 12500, 0, 1, .65f, true}};
    const auto &s = presets[index];
    auto set = [&](const char *id, float value)
    {
        auto *p = state.getParameter(id);
        p->setValueNotifyingHost(p->convertTo0to1(value));
    };
    set("model", (float)s.model);
    set("drive", s.drive);
    set("tone", s.tone);
    set("lowcut", s.low);
    set("highcut", s.high);
    set("bias", s.bias);
    set("dynamics", s.dynamics);
    set("mix", s.mix);
    set("heat", s.heat ? 1.0f : 0.0f);
    set("auto_gain", 1);
    set("output", 0);
    set("width", 1);
    set("bypass", 0);
    program = index;
}
// Host oturumu için mevcut parametre ve kullanıcı durumunu seri hale getirir.
void Processor::getStateInformation(juce::MemoryBlock &data)
{
    auto xml = state.copyState().createXml();
    xml->setAttribute("program", program);
    xml->setAttribute("version", 1);
    copyXmlToBinary(*xml, data);
}
// Host/preset durumunu okuyup uygular; kök/sürüm denetimleri eski oturum uyumluluğu içindir.
void Processor::setStateInformation(const void *data, int size)
{
    if (auto xml = getXmlFromBinary(data, size))
        if (xml->hasTagName("IRAK_STATE"))
        {
            program = juce::jlimit(0, 15, xml->getIntAttribute("program", 0));
            state.replaceState(juce::ValueTree::fromXml(*xml));
        }
}
// createEmbeddedProcessor alt adımı: Saturation parametrelerini, presetleri ve host callback’lerini kendi DSP motoruna bağlar.
juce::AudioProcessor *createEmbeddedProcessor()
{
    return new Processor();
}
// createAboutPage alt adımı: Saturation parametrelerini, presetleri ve host callback’lerini kendi DSP motoruna bağlar.
// Ortak Shaper Hakkında sayfasını oluşturur; teknik araştırma belgelerde korunur.
juce::Component *createAboutPage()
{
    return new shaperui::RackAboutPage("Saturation", "", juce::Colour(0xffed6c46));
}

} // namespace irak
