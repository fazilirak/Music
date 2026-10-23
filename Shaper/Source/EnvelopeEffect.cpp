// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 Fazıl Irak; Shaper eklemeleri/değişiklikleri.
// Fazıl Irak tarafından oluşturuldu / değiştirildi: 23.10.2026
// Kapsam: Shaper için eklenen kod ve uyarlamalar; özgün üçüncü taraf telif/lisansları korunur.
// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: Bağımsız efekt motorunu tempo, tetik, sequencer ve envelope modülasyonuyla sarar.
#include "EnvelopeEffect.h"
#include "EnvelopeEditor.h"
#include "Saturation/PluginProcessor.h"
#include "Pitch/PitchProcessor.h"
#include "EnvelopePresets.h"
namespace satcurve
{
juce::AudioProcessor *createEmbeddedProcessor();
void forceCV(juce::AudioProcessor &);
bool audioSettingsShown(juce::AudioProcessor &);
void shape(juce::AudioProcessor &, int, float);
} // namespace satcurve
namespace pitchcurve
{
juce::AudioProcessor *createEmbeddedProcessor();
void forceCV(juce::AudioProcessor &);
void formantView(juce::AudioProcessor &, bool);
bool audioSettingsShown(juce::AudioProcessor &);
void shape(juce::AudioProcessor &, int, float);
} // namespace pitchcurve
// Host parametre tanımlarını oluşturur; kimlik, aralık ve varsayılan sözleşmesini birlikte koruyun.
juce::AudioProcessorValueTreeState::ParameterLayout EnvelopeEffect::layout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout p;
    p.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"curve_depth", 1}, "Curve depth",
                                                      0.0f, 1.0f, 1.0f));
    return p;
}
// EnvelopeEffect alt adımı: Bağımsız efekt motorunu tempo, tetik, sequencer ve envelope modülasyonuyla sarar.
// Gövdedeki başlıca çağrılar: native.reset → pitchfx::createEmbeddedProcessor → irak::createEmbeddedProcessor → c.reset. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
EnvelopeEffect::EnvelopeEffect(bool pitch)
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withInput("Sidechain", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      state(*this, nullptr, "CURVE_SETTINGS", layout()), isPitch(pitch)
{
    native.reset(pitch ? pitchfx::createEmbeddedProcessor() : irak::createEmbeddedProcessor());
    for (auto &c : curves)
        c.reset(pitch ? pitchcurve::createEmbeddedProcessor() : satcurve::createEmbeddedProcessor());
    if (pitch)
        pitchcurve::formantView(*curves[1], true);
    for (auto *parameter : native->getParameters())
        if (auto *p = dynamic_cast<juce::RangedAudioParameter *>(parameter))
            addParameter(new ForwardParameter(p->paramID, p->getName(64), *p));
    for (int index = 0; index < (pitch ? 2 : 1); ++index)
        for (auto *parameter : curves[(size_t)index]->getParameters())
            if (auto *p = dynamic_cast<juce::RangedAudioParameter *>(parameter))
            {
                bool collision = false;
                for (auto *n : native->getParameters())
                    if (auto *r = dynamic_cast<juce::RangedAudioParameter *>(n))
                        if (r->paramID == p->paramID)
                            collision = true;
                const auto id = index == 1  ? "formant_curve_" + p->paramID
                                : collision ? "curve_" + p->paramID
                                            : p->paramID;
                addParameter(new ForwardParameter(
                    id, (index == 1 ? "Formant curve " : "Curve ") + p->getName(64), *p));
            }
    applyShape(0);
    forceCV();
}
// ~EnvelopeEffect alt adımı: Bağımsız efekt motorunu tempo, tetik, sequencer ve envelope modülasyonuyla sarar.
EnvelopeEffect::~EnvelopeEffect()
{
    for (auto *p : getParameters())
        if (auto *f = dynamic_cast<ForwardParameter *>(p))
            f->detach();
}
// audioSettingsShown alt adımı: Bağımsız efekt motorunu tempo, tetik, sequencer ve envelope modülasyonuyla sarar.
bool EnvelopeEffect::audioSettingsShown()
{
    return isPitch ? pitchcurve::audioSettingsShown(curve()) : satcurve::audioSettingsShown(curve());
}
// forceCV alt adımı: Bağımsız efekt motorunu tempo, tetik, sequencer ve envelope modülasyonuyla sarar.
void EnvelopeEffect::forceCV()
{
    for (auto &c : curves)
    {
        if (isPitch)
            pitchcurve::forceCV(*c);
        else
            satcurve::forceCV(*c);
    }
}
// applyShape: Bağımsız efekt motorunu tempo, tetik, sequencer ve envelope modülasyonuyla sarar. Burada verilen ayar türetilmiş duruma/çıktıya aktarılır; çağrı sırası aşağıdaki gövdededir.
void EnvelopeEffect::applyShape(int kind)
{
    if (isPitch)
    {
        pitchcurve::shape(*curves[0], kind, .5f);
        pitchcurve::shape(*curves[1], 0, .5f);
    }
    else
        satcurve::shape(*curves[0], kind, 1);
}
// Desteklenen giriş/çıkış kanal düzenini doğrular; synth ve FX giriş sözleşmeleri farklı olabilir.
bool EnvelopeEffect::isBusesLayoutSupported(const BusesLayout &l) const
{
    return l.getMainInputChannelSet() == juce::AudioChannelSet::stereo() &&
           l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}
// Host hazırlığında örnekleme, blok kapasitesi ve bildirilecek gecikmeyi kurar.
// Gövdedeki başlıca çağrılar: juce::jmax → native->setRateAndBufferSizeDetails → native->prepareToPlay → setRateAndBufferSizeDetails. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
void EnvelopeEffect::prepareToPlay(double sr, int n)
{
    capacity = juce::jmax(1, n);
    native->setRateAndBufferSizeDetails(sr, capacity);
    native->prepareToPlay(sr, capacity);
    for (size_t i = 0; i < 2; ++i)
    {
        curves[i]->setRateAndBufferSizeDetails(sr, capacity);
        curves[i]->prepareToPlay(sr, capacity);
        modulation[i].setSize(4, capacity);
        events[i].ensureSize(65536);
    }
    // Gate's Audio trigger uses 5 ms lookahead plus up to 10 ms anti-click.
    // Reserve that fixed amount in every band/mode so Pitch and Formant can
    // have different trigger sources without moving their audio alignment.
    lookahead = (int)std::ceil(sr * .015);
    inputDelay.setSize(2, lookahead + 1);
    inputDelay.clear();
    curveDelay.setSize(2, lookahead + 1);
    delayPosition = 0;
    for (int c = 0; c < 2; ++c)
        for (int i = 0; i < curveDelay.getNumSamples(); ++i)
            curveDelay.setSample(c, i, isPitch ? .5f : 1.0f);
    setLatencySamples(native->getLatencySamples() + lookahead);
}
// Host kaynak bırakma aşamasıdır; boşsa sahiplik nesne/yeniden hazırlık yaşam döngüsündedir.
void EnvelopeEffect::releaseResources()
{
    native->releaseResources();
    for (auto &c : curves)
        c->releaseResources();
    capacity = 0;
}
// Host audio callback’ini işler; burada dosya/GUI veya bekleyen kilit eklemeyin.
// Gövdedeki başlıca çağrılar: forceCV → cv.setSize → b.getNumSamples → b.getNumChannels. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
void EnvelopeEffect::processBlock(juce::AudioBuffer<float> &b, juce::MidiBuffer &midi)
{
    if (capacity <= 0)
        return;
    forceCV();
    for (int index = 0; index < (isPitch ? 2 : 1); ++index)
    {
        auto &c = *curves[(size_t)index];
        auto &cv = modulation[(size_t)index];
        cv.setSize(4, b.getNumSamples(), false, false, true);
        for (int channel = 0; channel < 4; ++channel)
        {
            if (channel < b.getNumChannels())
                cv.copyFrom(channel, 0, b, channel, 0, b.getNumSamples());
            else
                cv.clear(channel, 0, b.getNumSamples());
        }
        c.setPlayHead(getPlayHead());
        events[(size_t)index].clear();
        events[(size_t)index].addEvents(midi, 0, b.getNumSamples(), 0);
        c.processBlock(cv, events[(size_t)index]);
    }
    const float depth = state.getRawParameterValue("curve_depth")->load();
    const int ringSize = inputDelay.getNumSamples();
    for (int i = 0; i < b.getNumSamples(); ++i)
    {
        for (int c = 0; c < (isPitch ? 2 : 1); ++c)
        {
            curveDelay.setSample(c, delayPosition, modulation[(size_t)c].getSample(0, i));
            const int delay = juce::jlimit(0, lookahead, lookahead - curves[(size_t)c]->getLatencySamples());
            modulation[(size_t)c].setSample(
                0, i, curveDelay.getSample(c, (delayPosition + ringSize - delay) % ringSize));
        }
        for (int c = 0; c < 2; ++c)
        {
            inputDelay.setSample(c, delayPosition, b.getSample(c, i));
            b.setSample(c, i, inputDelay.getSample(c, (delayPosition + ringSize - lookahead) % ringSize));
        }
        delayPosition = (delayPosition + 1) % ringSize;
    }
    if (isPitch)
        dynamic_cast<pitchfx::Processor &>(*native).processModulated(
            b, depth > 0 ? modulation[0].getReadPointer(0) : nullptr,
            depth > 0 ? modulation[1].getReadPointer(0) : nullptr, depth);
    else
    {
        auto &p = dynamic_cast<irak::Processor &>(*native);
        if (depth > 0)
            p.dsp.processModulated(b, p.settings(), modulation[0].getReadPointer(0), depth);
        else
            p.processBlock(b, midi);
    }
}
// Program indeksine karşılık gelen görünür adı döndürür; ad ve değer bankası aynı sırada olmalıdır.
const juce::String EnvelopeEffect::getProgramName(int index)
{
    if (index >= 24)
        return envelopePresets::name(isPitch, index - 24);
    if (index < 16)
        return native->getProgramName(index);
    static const char *names[]{"Curve / Rise",      "Curve / Fall", "Curve / Vibrato",  "Curve / Alternating",
                               "Curve / Staircase", "Curve / Drop", "Curve / Up sweep", "Curve / Down sweep"};
    return names[juce::jlimit(0, 7, index - 16)];
}
// Fabrika/kullanıcı program seçimini mevcut parametre/durum akışına uygular.
// Gövdedeki başlıca çağrılar: juce::jlimit → getNumPrograms → envelopePresets::reset → state.getParameter. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
void EnvelopeEffect::setCurrentProgram(int i)
{
    i = juce::jlimit(0, getNumPrograms() - 1, i);
    for (auto &c : curves)
        envelopePresets::reset(*c);
    state.getParameter("curve_depth")->setValueNotifyingHost(1);
    selectedCurve = 0;
    native->setCurrentProgram(i < 16 ? i : 0);
    if (i >= 24)
    {
        const int v = i - 24;
        if (isPitch)
        {
            envelopePresets::set(*native, "mix", v == 12 ? .4f : v == 13 ? .55f : 1.f);
            envelopePresets::set(*native, "preserve_formants", v == 10 || v == 11 ? 1.f : 0.f);
        }
        else
        {
            constexpr int models[]{0, 2, 3, 1, 4, 3, 2, 4, 3, 4, 0, 1, 0, 4, 2, 4};
            constexpr float drives[]{12, 12, 18, 15, 22, 10, 16, 18, 20, 22, 5, 17, 14, 24, 11, 28};
            envelopePresets::set(*native, "model", (float)models[v]);
            envelopePresets::set(*native, "drive", drives[v]);
            envelopePresets::set(*native, "tone", v == 6 ? -.45f : v == 12 ? .35f : 0.f);
            envelopePresets::set(*native, "mix", v == 13 ? .3f : v == 10 ? .65f : 1.f);
            envelopePresets::set(*native, "heat", v == 4 || v == 15 ? 1.f : 0.f);
            envelopePresets::set(*native, "dynamics", v == 11 ? .65f : .35f);
        }
        envelopePresets::curve(*curves[0], isPitch, v);
        if (isPitch)
            envelopePresets::curve(*curves[1], true, v, true);
        forceCV();
        program = i;
        return;
    }
    if (i >= 16)
    {
        state.getParameter("curve_depth")->setValueNotifyingHost(1);
        applyShape(i - 15);
    }
    else
        applyShape(0);
    program = i;
}
// Host oturumu için mevcut parametre ve kullanıcı durumunu seri hale getirir.
// Gövdedeki başlıca çağrılar: xml → xml.setAttribute → xml.addChildElement → state.copyState. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
void EnvelopeEffect::getStateInformation(juce::MemoryBlock &data)
{
    juce::XmlElement xml("SHAPER_ENVELOPE_EFFECT");
    xml.setAttribute("kind", isPitch ? "Pitch" : "Saturation");
    xml.setAttribute("program", program);
    xml.setAttribute("selectedCurve", selectedCurve);
    xml.addChildElement(state.copyState().createXml().release());
    for (int i = -1; i < (isPitch ? 2 : 1); ++i)
    {
        juce::MemoryBlock b;
        (i < 0 ? *native : *curves[(size_t)i]).getStateInformation(b);
        auto *child = xml.createNewChildElement(i < 0 ? "Native" : "Curve");
        child->setAttribute("index", i);
        child->addTextElement(b.toBase64Encoding());
    }
    copyXmlToBinary(xml, data);
}
// Host/preset durumunu okuyup uygular; kök/sürüm denetimleri eski oturum uyumluluğu içindir.
// Gövdedeki başlıca çağrılar: getXmlFromBinary → xml->hasTagName → xml->getStringAttribute → xml->getChildByName. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
void EnvelopeEffect::setStateInformation(const void *d, int n)
{
    if (auto xml = getXmlFromBinary(d, n))
    {
        if (xml->hasTagName("SHAPER_ENVELOPE_EFFECT"))
        {
            if (xml->getStringAttribute("kind") != (isPitch ? "Pitch" : "Saturation"))
                return;
            if (auto *settings = xml->getChildByName("CURVE_SETTINGS"))
                state.replaceState(juce::ValueTree::fromXml(*settings));
            for (auto *node : xml->getChildIterator())
                if (node->hasTagName("Native") || node->hasTagName("Curve"))
                {
                    juce::MemoryBlock b;
                    const int i = node->getIntAttribute("index", -1);
                    if (i >= -1 && i < 2 && b.fromBase64Encoding(node->getAllSubText()))
                        (i < 0 ? *native : *curves[(size_t)i])
                            .setStateInformation(b.getData(), (int)b.getSize());
                }
            program = juce::jlimit(0, getNumPrograms() - 1, xml->getIntAttribute("program"));
            selectedCurve = isPitch ? juce::jlimit(0, 1, xml->getIntAttribute("selectedCurve")) : 0;
        }
        else if (xml->hasTagName(isPitch ? "PITCH_STATE" : "IRAK_STATE"))
        {
            native->setStateInformation(d, n);
            program = native->getCurrentProgram();
            applyShape(0);
            state.getParameter("curve_depth")->setValueNotifyingHost(0);
            selectedCurve = 0;
        }
        forceCV();
    }
}
// Host’un sahipliğine geçecek editörü oluşturur; Processor editörden daha uzun yaşamalıdır.
juce::AudioProcessorEditor *EnvelopeEffect::createEditor()
{
    return new EnvelopeEditor(*this);
}
