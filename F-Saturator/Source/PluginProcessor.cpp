// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (c) 2026 Fazıl Irak
// Geliştirici: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: Parametre aralıkları, oturum/preset kaydı, host programları ve DSP callback köprüsü.
#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace irak
{
// DAW otomasyon sözleşmesi. Kimlikleri (drive, model vb.) ve sürüm 1 değerlerini
// rastgele değiştirmeyin; eski oturum/preset uyumluluğu bunlara bağlıdır.
// Parametre eklerken dört yer eşleşmeli: layout → Settings → settings() okuması → attachment/render. Eski kimliği silmek DAW otomasyonunu koparır.
juce::AudioProcessorValueTreeState::ParameterLayout Processor::layout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout p;
    p.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"model", 1}, "Character",
        juce::StringArray{"Console", "Transformer", "Tape", "Triode", "Pentode"}, 0));
    // Fiziksel aralık, 0.001 adım ve skew eğrisiyle sürekli parametre ekler.
    // Frekans halkalarının görsel logaritmik dönüşümü ayrıca ConcentricKnob içinde yapılır.
    auto add = [&](const char *id, const char *name, float lo, float hi, float value, float skew = 1.0f)
    {
        p.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{id, 1}, name, juce::NormalisableRange<float>(lo, hi, .001f, skew), value));
    };
    // Birimler: Drive/Output dB; kesimler Hz; Tone/Bias -1..1; Mix/Dynamics 0..1; Width 0..2.
    add("drive", "Drive dB", 0, 36, 6);
    add("tone", "Tone", -1, 1, 0);
    add("lowcut", "Low cut Hz", 20, 500, 20, .4f);
    add("highcut", "High cut Hz", 2000, 20000, 20000, .4f);
    add("bias", "Bias", -1, 1, 0);
    add("dynamics", "Dynamics", 0, 1, .3f);
    add("mix", "Mix", 0, 1, 1);
    add("output", "Output dB", -60, 12, 0);
    add("width", "Stereo width", 0, 2, 1);
    // Anahtarların varsayılanları: Heat kapalı, Auto Gain açık, Bypass kapalı.
    for (const auto &entry : std::array<std::pair<const char *, bool>, 3>{
             {{"heat", false}, {"auto_gain", true}, {"bypass", false}}})
        p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{entry.first, 1}, entry.first,
                                                         entry.second));
    return p;
}
// Stereo varsayılan bus ve IRAK_STATE kökü oluşturulur. Ürün adı değişse de eski durum kimliği korunur.
Processor::Processor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      state(*this, nullptr, "IRAK_STATE", layout())
{
}
// Her çağrıda atomik parametreleri okur; DSP’nin JUCE parametre nesnelerine bağımlı olmasını önler.
// Okumalar tek tek atomiktir, bütün parametreler için kilitli bir anlık görüntü değildir.
Settings Processor::settings() const
{
    // Parametre işaretçisi araması ses callback yolundadır. Kimlik yanlışsa null erişim oluşabilir; yeni alanı aynı kimlikle layout içinde tanımlayın.
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
// Oversampling/DSP hazırlığı ve gecikme bildirimi; örnekleme hızı değişince host yeniden çağırır.
// Hazırlık sırası: DSP örnekleme/tampon/gecikme kurulumu tamamlanır, ardından DAW gecikme bildirimi yapılır.
void Processor::prepareToPlay(double sr, int n)
{
    dsp.prepare(sr, n);
    setLatencySamples(dsp.getLatency());
}
// Denormal çok küçük sayıları devre dışı bırakarak sessizlikte gereksiz işlemci yükünü azaltır.
// Gerçek zamanlı giriş: buraya dosya/JSON/preset I/O, mesaj kutusu veya bekleyen mutex eklemeyin. Editörün açık olması gerekmez.
void Processor::processBlock(juce::AudioBuffer<float> &audio, juce::MidiBuffer &)
{
    juce::ScopedNoDenormals guard;
    dsp.process(audio, settings());
}
// Yalnız eşleşen mono→mono ve stereo→stereo düzenlerine izin verir; surround desteklenmez.
bool Processor::isBusesLayoutSupported(const BusesLayout &l) const
{
    return (l.getMainInputChannelSet() == juce::AudioChannelSet::mono() ||
            l.getMainInputChannelSet() == juce::AudioChannelSet::stereo()) &&
           l.getMainInputChannelSet() == l.getMainOutputChannelSet();
}
// Host’un sahipliğini alacağı düzenleyiciyi oluşturur; ses motoru editör kapalıyken de çalışır.
juce::AudioProcessorEditor *Processor::createEditor()
{
    return new Editor(*this);
}
// Preset isim dizisi setCurrentProgram içindeki 16 değer satırıyla aynı sıradadır.
const juce::String Processor::getProgramName(int i)
{
    static const char *names[]{"Clean Desk",    "Warm Bus",      "Tape Glue",      "Drum Heat",
                               "Vocal Edge",    "Bass Weight",   "Parallel Crush", "Bright Console",
                               "Soft Keys",     "Vintage Break", "Triode Body",    "Pentode Bite",
                               "Magnetic Bass", "Tape Air",      "Dark Overdrive", "Total Burn"};
    return names[juce::jlimit(0, 15, i)];
}
// Yeni fabrika sesi eklerken isim, sayı, bu değer dizisi ve test/paket beklentilerini birlikte güncelleyin.
// Preset yüklemek tek atomik işlem değildir: her parametre host’a ayrı bildirilir; ses motorunun rampaları geçişi yumuşatır.
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
    // Fiziksel preset değerini host’un 0..1 aralığına çevirir ve otomasyona değişikliği bildirir.
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
// APVTS XML’ine program indeksi ve durum biçimi sürümü eklenir; JUCE binary XML biçimine paketlenir.
void Processor::getStateInformation(juce::MemoryBlock &data)
{
    auto xml = state.copyState().createXml();
    xml->setAttribute("program", program);
    xml->setAttribute("version", 1);
    copyXmlToBinary(*xml, data);
}
// Yalnız IRAK_STATE köküne sahip durumları kabul eder; program indeksi 0..15 sınırına çekilir.
// Dosyadan gelen binary XML başarısız ayrışırsa veya kök yanlışsa mevcut durum korunur; bu yol diske otomatik yazmaz.
void Processor::setStateInformation(const void *data, int size)
{
    if (auto xml = getXmlFromBinary(data, size))
        if (xml->hasTagName("IRAK_STATE"))
        {
            program = juce::jlimit(0, 15, xml->getIntAttribute("program", 0));
            state.replaceState(juce::ValueTree::fromXml(*xml));
        }
}
// Gömülü kullanım da aynı Processor sınıfını kullanır; ayrı bir DSP kopyası oluşturulmaz.
juce::AudioProcessor *createEmbeddedProcessor()
{
    return new Processor();
}
// Hakkında paneli: geliştirici, araştırma kaynakları, model sınırları ve JUCE lisans seçenekleri.
// Bu metin, dağıtımdaki LICENSE ve JUCE-LICENSE.md dosyalarının yerine geçmez.
juce::Component *createAboutPage()
{
    auto *text = new juce::TextEditor();
    text->setMultiLine(true);
    text->setReadOnly(true);
    text->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff151a22));
    text->setColour(juce::TextEditor::textColourId, juce::Colour(0xffffb879));
    text->setFont(juce::FontOptions(18));
    text->setText(juce::String::fromUTF8(
        "F Saturator — Analog Colour\n\nFazıl Irak / FAZIL IRAK\n\nConsole · Transformer · Tape · Triode · Pentode\n\nOriginal hybrid DSP: 8x FIR oversampling, antiderivative antialiasing, magnetic memory and dynamic stage loading.\n\nResearch: Alain Paul / AP Saturation; Resonant DSP / SwankyAmp; Parker, Zavalishin & Le Bivic (DAFx 2016); Chowdhury (DAFx 2019); Holters & Zölzer (DAFx 2016).\n\nIndependent design. No measured hardware match or Decapitator equivalence is claimed.\n\nJUCE: AGPLv3 / commercial licence."));
    return text;
}
} // namespace irak
