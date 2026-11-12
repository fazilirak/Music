// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 Fazıl Irak; Shaper eklemeleri/değişiklikleri.
// Fazıl Irak tarafından oluşturuldu / değiştirildi: 23.10.2026
// Kapsam: Shaper için eklenen kod ve uyarlamalar; özgün üçüncü taraf telif/lisansları korunur.
// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: Rack efekt zinciri, MIDI/transport, preset, ses callback ve kaydedilen durum akışını yönetir.
#include "Processor.h"
#include "Editor.h"
#include <algorithm>
namespace gate12
{
juce::AudioProcessor *JUCE_CALLTYPE createEmbeddedProcessor();
}
namespace time12
{
juce::AudioProcessor *JUCE_CALLTYPE createEmbeddedProcessor();
}
namespace filtr
{
juce::AudioProcessor *JUCE_CALLTYPE createEmbeddedProcessor();
}
namespace reevr
{
juce::AudioProcessor *JUCE_CALLTYPE createEmbeddedProcessor();
}
namespace pan12
{
juce::AudioProcessor *JUCE_CALLTYPE createEmbeddedProcessor();
}
namespace volumef
{
juce::AudioProcessor *JUCE_CALLTYPE createEmbeddedProcessor();
}
namespace irak
{
juce::AudioProcessor *createEmbeddedProcessor();
}
namespace pitchfx
{
juce::AudioProcessor *createEmbeddedProcessor();
}

#include "ForwardParameter.h"
#include "MultibandProcessor.h"
#include "EnvelopeEffect.h"
#include "UtilityEffect.h"
#include "DelayEffect.h"
// effectName alt adımı: Rack efekt zinciri, MIDI/transport, preset, ses callback ve kaydedilen durum akışını yönetir.
const char *ShaperProcessor::effectName(int e)
{
    static const char *names[]{"Gate", "Time", "Filter", "Reverb", "Pan", "Volume", "Saturation", "Pitch", "Width", "FX", "Delay"};
    return names[e];
}
const std::array<std::array<int, ShaperProcessor::effectCount>, ShaperProcessor::orderCount> &
ShaperProcessor::orders()
{
    static const auto result = []
    {
        // Keep the original six-effect parameter's 720 choices and indices.
        std::array<std::array<int, effectCount>, orderCount> a{};
        std::array<int, 6> p{0, 1, 2, 3, 4, 5};
        int i = 0;
        do
        {
            std::copy(p.begin(), p.end(), a[(size_t)i].begin());
            a[(size_t)i][6] = 6;
            a[(size_t)i][7] = 7; a[(size_t)i][8]=8; a[(size_t)i][9]=9; a[(size_t)i++][10]=10;
        } while (std::next_permutation(p.begin(), p.end()));
        return a;
    }();
    return result;
}
// Host parametre tanımlarını oluşturur; kimlik, aralık ve varsayılan sözleşmesini birlikte koruyun.
// Gövdedeki başlıca çağrılar: p.add → orders → s.isNotEmpty → effectName. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
juce::AudioProcessorValueTreeState::ParameterLayout ShaperProcessor::layout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout p;
    p.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"mix", 1}, "Master mix", 0.0f, 1.0f,
                                                      1.0f));
    p.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"gain", 1}, "Output dB", -24.0f,
                                                      12.0f, 0.0f));
    juce::StringArray choices;
    for (auto &sequence : orders())
    {
        juce::String s;
        for (int e : sequence)
            if (e < 6)
            {
                if (s.isNotEmpty())
                    s += " > ";
                s += effectName(e);
            }
        choices.add(s);
    }
    p.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"order", 1}, "Effect order", choices,
                                                       0));
    p.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{"saturation_position", 1},
                                                    "Saturation chain position", 0, 6, 6));
    p.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{"pitch_position", 1},
                                                    "Pitch chain position", 0, 7, 7));
    p.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{"width_position",1},"Width chain position",0,8,8));
    p.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{"fx_position",1},"FX chain position",0,9,9));
    for (int e = 0; e < effectCount; ++e)
    {
        p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{enableId(e), 1},
                                                         juce::String(effectName(e)) + " Enabled", true));
        p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{presentId(e), 1},
                                                         juce::String(effectName(e)) + " In palette", false));
    }
    p.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{"delay_position",1},"Delay chain position",0,10,10));
    return p;
}
// ShaperProcessor alt adımı: Rack efekt zinciri, MIDI/transport, preset, ses callback ve kaydedilen durum akışını yönetir.
// Gövdedeki başlıca çağrılar: state.getRawParameterValue → EnvelopeEffect → enableId → presentId. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
ShaperProcessor::ShaperProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withInput("Sidechain", juce::AudioChannelSet::stereo(), false)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      state(*this, nullptr, "SHAPER", layout())
{
    // Fazıl Irak · 03.11.2026: ilk kurulum %75; yeni instance son pencere ölçüsünü okur.
    juce::PropertiesFile::Options uiOptions;
    uiOptions.applicationName="Jellybone"; uiOptions.filenameSuffix="settings";
    uiOptions.folderName="Fazil Irak"; uiOptions.osxLibrarySubFolder="Application Support";
    juce::PropertiesFile uiPreferences(uiOptions);
    const int rememberedWidth=juce::jlimit(550,2200,uiPreferences.getIntValue("editorWidth",825));
    editorWidth.store(rememberedWidth);editorHeight.store(juce::roundToInt(rememberedWidth*960.0/1100.0));
    mixValue = state.getRawParameterValue("mix");
    gainValue = state.getRawParameterValue("gain");
    orderValue = state.getRawParameterValue("order");
    saturationPosition = state.getRawParameterValue("saturation_position");
    modules[0] = std::make_unique<MultibandProcessor>(gate12::createEmbeddedProcessor, 0);
    modules[1] = std::make_unique<MultibandProcessor>(time12::createEmbeddedProcessor, 1);
    modules[2] = std::make_unique<MultibandProcessor>(filtr::createEmbeddedProcessor, 2);
    modules[3] = std::make_unique<MultibandProcessor>(reevr::createEmbeddedProcessor, 3);
    modules[4] = std::make_unique<MultibandProcessor>(pan12::createEmbeddedProcessor, 4);
    modules[5] = std::make_unique<MultibandProcessor>(volumef::createEmbeddedProcessor, 5);
    modules[6] = std::make_unique<MultibandProcessor>([] { return new EnvelopeEffect(false); }, 6);
    modules[7] = std::make_unique<MultibandProcessor>([] { return new EnvelopeEffect(true); }, 7);
    modules[8]=std::make_unique<MultibandProcessor>([]{return new UtilityEffect(false);},8);
    // Bir FX modülünde tek plugin: bant başına dört instance oluşturulmaz.
    modules[9]=std::make_unique<UtilityEffect>(true);
    modules[10]=std::make_unique<MultibandProcessor>([]{return new DelayEffect();},10);
    pitchPosition = state.getRawParameterValue("pitch_position");
    for (int e = 0; e < effectCount; ++e)
    {
        enableValues[e] = state.getRawParameterValue(enableId(e));
        presentValues[e] = state.getRawParameterValue(presentId(e));
        for (auto *parameter : modules[e]->getParameters())
            if (auto *p = dynamic_cast<juce::RangedAudioParameter *>(parameter))
                addParameter(new ForwardParameter("e" + juce::String(e) + "_" + p->paramID,
                                                  juce::String(effectName(e)) + " " + p->getName(64), *p));
    }
    getStateInformation(initialState);
}
// palette alt adımı: Rack efekt zinciri, MIDI/transport, preset, ses callback ve kaydedilen durum akışını yönetir.
std::vector<int> ShaperProcessor::palette() const
{
    std::vector<int> result;
    for (int e : signalOrder())
        if (presentValues[e]->load() > 0.5f)
            result.push_back(e);
    return result;
}
// addEffect alt adımı: Rack efekt zinciri, MIDI/transport, preset, ses callback ve kaydedilen durum akışını yönetir.
void ShaperProcessor::addEffect(int effect)
{
    if (effect < 0 || effect >= effectCount || presentValues[effect]->load() > 0.5f)
        return;
    auto installed = palette();
    installed.push_back(effect);
    applyPaletteOrder(installed);
    state.getParameter(enableId(effect))->setValueNotifyingHost(1);
    state.getParameter(presentId(effect))->setValueNotifyingHost(1);
    selectedEffect = effect;
}
// removeEffect alt adımı: Rack efekt zinciri, MIDI/transport, preset, ses callback ve kaydedilen durum akışını yönetir.
void ShaperProcessor::removeEffect(int effect)
{
    if (effect < 0 || effect >= effectCount)
        return;
    state.getParameter(presentId(effect))->setValueNotifyingHost(0);
    auto installed = palette();
    if (selectedEffect == effect)
        selectedEffect = installed.empty() ? -1 : installed.front();
}
// moveEffect alt adımı: Rack efekt zinciri, MIDI/transport, preset, ses callback ve kaydedilen durum akışını yönetir.
void ShaperProcessor::moveEffect(int effect, int destination)
{
    auto installed = palette();
    auto at = std::find(installed.begin(), installed.end(), effect);
    if (at == installed.end())
        return;
    installed.erase(at);
    destination = juce::jlimit(0, (int)installed.size(), destination);
    installed.insert(installed.begin() + destination, effect);
    applyPaletteOrder(installed);
}
// signalOrder alt adımı: Rack efekt zinciri, MIDI/transport, preset, ses callback ve kaydedilen durum akışını yönetir.
// Gövdedeki başlıca çağrılar: orders → juce::jlimit → orderValue->load → saturationPosition->load. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
std::array<int, ShaperProcessor::effectCount> ShaperProcessor::signalOrder() const
{
    auto original = orders()[(size_t)juce::jlimit(0, orderCount - 1, (int)orderValue->load())];
    const char* positions[]{"saturation_position","pitch_position","width_position","fx_position","delay_position"};
    for(int e=6;e<effectCount;++e){
        const int position=juce::jlimit(0,e,(int)state.getRawParameterValue(positions[e-6])->load());
        for(int i=e;i>position;--i)original[(size_t)i]=original[(size_t)i-1];
        original[(size_t)position]=e;
    }
    return original;
}
// applyPaletteOrder: Rack efekt zinciri, MIDI/transport, preset, ses callback ve kaydedilen durum akışını yönetir. Burada verilen ayar türetilmiş duruma/çıktıya aktarılır; çağrı sırası aşağıdaki gövdededir.
// Gövdedeki başlıca çağrılar: std::find → installed.begin → installed.end → orders. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
void ShaperProcessor::applyPaletteOrder(const std::vector<int> &installed)
{
    // Fazıl Irak · 03.11.2026: eski 720 sıra kimliği korunur; yeni efektler ayrı ekleme konumları kullanır.
    auto all=installed;
    for(int e=0;e<effectCount;++e)if(std::find(all.begin(),all.end(),e)==all.end())all.push_back(e);
    std::array<int,effectCount> sequence{};int index=0;
    for(int e:all)if(e<6)sequence[(size_t)index++]=e;
    for(int e=6;e<effectCount;++e)sequence[(size_t)e]=e;
    for(int i=0;i<orderCount;++i)if(orders()[(size_t)i]==sequence){auto* p=state.getParameter("order");p->setValueNotifyingHost(p->convertTo0to1((float)i));break;}
    const char* positions[]{"saturation_position","pitch_position","width_position","fx_position","delay_position"};
    for(int e=6;e<effectCount;++e){int slot=0;for(int item:all){if(item==e)break;if(item<e)++slot;}
        auto* p=state.getParameter(positions[e-6]);p->setValueNotifyingHost(p->convertTo0to1((float)slot));}
}
// ~ShaperProcessor alt adımı: Rack efekt zinciri, MIDI/transport, preset, ses callback ve kaydedilen durum akışını yönetir.
ShaperProcessor::~ShaperProcessor()
{
    cancelPendingUpdate();
    for (auto *p : getParameters())
        if (auto *f = dynamic_cast<ForwardParameter *>(p))
            f->detach();
}
// Desteklenen giriş/çıkış kanal düzenini doğrular; synth ve FX giriş sözleşmeleri farklı olabilir.
bool ShaperProcessor::isBusesLayoutSupported(const BusesLayout &l) const
{
    return l.getMainInputChannelSet() == juce::AudioChannelSet::stereo() &&
           l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo() &&
           (l.getChannelSet(true, 1).isDisabled() ||
            l.getChannelSet(true, 1) == juce::AudioChannelSet::stereo());
}
// Host hazırlığında örnekleme, blok kapasitesi ve bildirilecek gecikmeyi kurar.
// Gövdedeki başlıca çağrılar: juce::jmax → chain.setSize → scratch.setSize → dry.setSize. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
void ShaperProcessor::prepareToPlay(double sr, int size)
{
    rate = sr;
    blockCapacity = juce::jmax(1, size);
    chain.setSize(4, blockCapacity);
    scratch.setSize(2, blockCapacity);
    dry.setSize(2, blockCapacity);
    const int capacity = (int)(sr * 2) + blockCapacity + 1;
    int latency = 0;
    for (int e = 0; e < effectCount; ++e)
    {
        auto &p = *modules[e];
        p.setRateAndBufferSizeDetails(sr, blockCapacity);
        p.prepareToPlay(sr, blockCapacity);
        if (presentValues[e]->load() > 0.5f)
            latency += p.getLatencySamples();
        bypassDelay[e].prepare(capacity);
        activeSmooth[e].reset(sr, 0.01);
        activeSmooth[e].setCurrentAndTargetValue(enableValues[e]->load());
        moduleMidi[e].ensureSize(65536);
    }
    dryDelay.prepare(capacity);
    mixSmooth.reset(sr, 0.02);
    gainSmooth.reset(sr, 0.02);
    mixSmooth.setCurrentAndTargetValue(mixValue->load());
    gainSmooth.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(gainValue->load()));
    requestedLatency.store(latency);
    setLatencySamples(latency);
}
// Host kaynak bırakma aşamasıdır; boşsa sahiplik nesne/yeniden hazırlık yaşam döngüsündedir.
void ShaperProcessor::releaseResources()
{
    for (auto &m : modules)
        m->releaseResources();
    blockCapacity = 0;
}
// handleAsyncUpdate alt adımı: Rack efekt zinciri, MIDI/transport, preset, ses callback ve kaydedilen durum akışını yönetir.
void ShaperProcessor::handleAsyncUpdate()
{
    setLatencySamples(requestedLatency.load());
}
// Host audio callback’ini işler; burada dosya/GUI veya bekleyen kilit eklemeyin.
void ShaperProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midi)
{
    juce::ScopedNoDenormals guard;
    // Fazıl Irak · 12.11.2026: state yüklenirken ses thread'i beklemez; o blok kuru geçer.
    const juce::ScopedTryLock stateGuard(getCallbackLock());
    if(!stateGuard.isLocked()) return;
    if (blockCapacity == 0)
        return;
    for (int offset = 0; offset < buffer.getNumSamples(); offset += blockCapacity)
        render(buffer, midi, offset, juce::jmin(blockCapacity, buffer.getNumSamples() - offset));
}
// Hazırlanmış durumdan ses veya panel çıktısını üretir; bu dosyanın DSP/GUI bağlamını aşağıdaki gövdeden ayırın.
// Gövdedeki başlıca çağrılar: getPlayHead → mixSmooth.setTargetValue → mixValue->load → gainSmooth.setTargetValue. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
// Eşzamanlılık sınırı: bu kilidin hangi thread’de alındığını kontrol edin; audio callback’e yeni bekleyen kilit taşımayın.
void ShaperProcessor::render(juce::AudioBuffer<float> &output, juce::MidiBuffer &midi, int offset, int n)
{
    chunkPlayHead.source = getPlayHead();
    chunkPlayHead.offset = offset;
    chunkPlayHead.sampleRate = rate;
    mixSmooth.setTargetValue(mixValue->load());
    gainSmooth.setTargetValue(juce::Decibels::decibelsToGain(gainValue->load()));
    chain.setSize(4, n, false, false, true);
    for (int c = 0; c < 2; ++c)
    {
        chain.copyFrom(c, 0, output, c, offset, n);
        dry.copyFrom(c, 0, output, c, offset, n);
    }
    const auto sequence = signalOrder();
    int latency = 0;
    for (int e : sequence)
    {
        if (presentValues[e]->load() < 0.5f)
            continue;
        auto &p = *modules[e];
        p.setPlayHead(chunkPlayHead.source ? &chunkPlayHead : nullptr);
        for (int c = 0; c < 2; ++c)
        {
            scratch.copyFrom(c, 0, chain, c, 0, n);
            if (output.getNumChannels() >= 4)
                chain.copyFrom(c + 2, 0, output, c + 2, offset, n);
            else
                chain.clear(c + 2, 0, n);
        }
        auto &events = moduleMidi[e];
        events.clear();
        events.addEvents(midi, offset, n, -offset);
        {
            const juce::ScopedLock lock(p.getCallbackLock());
            p.processBlock(chain, events);
        }
        const int moduleLatency = p.getLatencySamples();
        latency += moduleLatency;
        auto &smooth = activeSmooth[e];
        smooth.setTargetValue(enableValues[e]->load());
        auto &delay = bypassDelay[e];
        for (int i = 0; i < n; ++i)
        {
            float wet = smooth.getNextValue();
            for (int c = 0; c < 2; ++c)
            {
                float d = delay.sample(c, scratch.getSample(c, i), moduleLatency);
                chain.setSample(c, i, d + wet * (chain.getSample(c, i) - d));
            }
            delay.advance();
        }
    }
    if (requestedLatency.exchange(latency) != latency)
        triggerAsyncUpdate();
    for (int i = 0; i < n; ++i)
    {
        float mix = mixSmooth.getNextValue(), gain = gainSmooth.getNextValue();
        for (int c = 0; c < 2; ++c)
        {
            float d = dryDelay.sample(c, dry.getSample(c, i), latency);
            output.setSample(c, offset + i, gain * (d + mix * (chain.getSample(c, i) - d)));
        }
        dryDelay.advance();
    }
}
// Host oturumu için mevcut parametre ve kullanıcı durumunu seri hale getirir.
// Gövdedeki başlıca çağrılar: lock → getCallbackLock → state.copyState → createXml. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
// Eşzamanlılık sınırı: bu kilidin hangi thread’de alındığını kontrol edin; audio callback’e yeni bekleyen kilit taşımayın.
void ShaperProcessor::getRackStateInformation(juce::MemoryBlock &data)
{
    const juce::ScopedLock lock(getCallbackLock());
    auto xml = state.copyState().createXml();
    xml->setAttribute("version", 8);
    xml->setAttribute("editorWidth", editorWidth.load());
    xml->setAttribute("editorHeight", editorHeight.load());
    xml->setAttribute("effect", selectedEffect);
    xml->setAttribute("rackName", rackPresetName);
    xml->setAttribute("factoryIndex", factoryPresetIndex);
    for (int e = 0; e < effectCount; ++e)
    {
        juce::MemoryBlock block;
        modules[e]->getStateInformation(block);
        auto *x = xml->createNewChildElement("Module");
        x->setAttribute("effect", e);
        x->addTextElement(block.toBase64Encoding());
    }
    copyXmlToBinary(*xml, data);
}
// Host/preset durumunu okuyup uygular; kök/sürüm denetimleri eski oturum uyumluluğu içindir.
// Gövdedeki başlıca çağrılar: lock → getCallbackLock → getXmlFromBinary → xml->hasTagName. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
// Eşzamanlılık sınırı: bu kilidin hangi thread’de alındığını kontrol edin; audio callback’e yeni bekleyen kilit taşımayın.
void ShaperProcessor::setRackStateInformation(const void *data, int size)
{
    const juce::ScopedLock lock(getCallbackLock());
    if (auto xml = getXmlFromBinary(data, size))
        if (xml->hasTagName("SHAPER"))
        {
            const int version = xml->getIntAttribute("version", 1);
            // Fazıl Irak, 02.11.2026: eski state boyut içermez; mevcut tercih korunur.
            editorWidth.store(juce::jlimit(550, 2200, xml->getIntAttribute("editorWidth", editorWidth.load())));
            editorHeight.store(juce::jlimit(480, 1920, xml->getIntAttribute("editorHeight", editorHeight.load())));
            rackPresetName = xml->getStringAttribute("rackName");
            factoryPresetIndex = xml->getIntAttribute("factoryIndex", -1);
            const bool legacy = version < 2;
            const int legacyBand = juce::jlimit(0, 2, xml->getIntAttribute("band", 1));
            for (auto *x : xml->getChildIterator())
                if (x->hasTagName("Module"))
                {
                    int e = x->getIntAttribute("effect", -1);
                    juce::MemoryBlock block;
                    if ((!legacy || x->getIntAttribute("band", -1) == legacyBand) && e >= 0 &&
                        e < effectCount && block.fromBase64Encoding(x->getAllSubText()))
                        modules[e]->setStateInformation(block.getData(), (int)block.getSize());
                }
            selectedEffect = juce::jlimit(-1, effectCount - 1, xml->getIntAttribute("effect", -1));
            auto tree = juce::ValueTree::fromXml(*xml);
            for (int i = tree.getNumChildren(); --i >= 0;)
                if (tree.getChild(i).hasType("Module") || tree.getChild(i).hasType("CompareBank"))
                    tree.removeChild(i, nullptr);
            if(version<8){
                for(const auto& entry:std::array<std::pair<juce::String,float>,3>{{{presentId(10),0.f},{enableId(10),1.f},{"delay_position",10.f}}}){
                    for(int i=tree.getNumChildren();--i>=0;)if(tree.getChild(i).getProperty("id").toString()==entry.first)tree.removeChild(i,nullptr);
                    juce::ValueTree param("PARAM");param.setProperty("id",entry.first,nullptr);param.setProperty("value",entry.second,nullptr);tree.appendChild(param,nullptr);
                }
            }
            if(version<7){
                for(const auto& entry:std::array<std::pair<juce::String,float>,4>{{{presentId(8),0.f},{presentId(9),0.f},{"width_position",8.f},{"fx_position",9.f}}}){
                    for(int i=tree.getNumChildren();--i>=0;)if(tree.getChild(i).getProperty("id").toString()==entry.first)tree.removeChild(i,nullptr);
                    juce::ValueTree param("PARAM");param.setProperty("id",entry.first,nullptr);param.setProperty("value",entry.second,nullptr);tree.appendChild(param,nullptr);
                }
            }
            if (version < 6)
            {
                for (const auto &entry : std::array<std::pair<juce::String, float>, 2>{
                         {{presentId(7), 0.0f}, {"pitch_position", 7.0f}}})
                {
                    for (int i = tree.getNumChildren(); --i >= 0;)
                        if (tree.getChild(i).getProperty("id").toString() == entry.first)
                            tree.removeChild(i, nullptr);
                    juce::ValueTree parameter("PARAM");
                    parameter.setProperty("id", entry.first, nullptr);
                    parameter.setProperty("value", entry.second, nullptr);
                    tree.appendChild(parameter, nullptr);
                }
            }
            if (version < 5)
            {
                for (const auto &entry : std::array<std::pair<juce::String, float>, 2>{
                         {{presentId(6), 0.0f}, {"saturation_position", 6.0f}}})
                {
                    for (int i = tree.getNumChildren(); --i >= 0;)
                        if (tree.getChild(i).getProperty("id").toString() == entry.first)
                            tree.removeChild(i, nullptr);
                    juce::ValueTree parameter("PARAM");
                    parameter.setProperty("id", entry.first, nullptr);
                    parameter.setProperty("value", entry.second, nullptr);
                    tree.appendChild(parameter, nullptr);
                }
            }
            if (legacy)
            {
                // With the outer bands removed, retain the previously selected band's chain.
                auto migrated = state.copyState();
                for (int i = 0; i < tree.getNumChildren(); ++i)
                {
                    auto old = tree.getChild(i);
                    auto id = old.getProperty("id").toString();
                    juce::String target = id;
                    for (int e = 0; e < 4; ++e)
                        if (id == "b" + juce::String(legacyBand) + "_e" + juce::String(e) + "_enabled")
                            target = enableId(e);
                    if (id == "order")
                    {
                        int oldIndex = juce::jlimit(0, 23, (int)old.getProperty("value"));
                        std::array<int, 4> oldOrder{0, 1, 2, 3};
                        while (oldIndex-- > 0)
                            std::next_permutation(oldOrder.begin(), oldOrder.end());
                        for (int j = 0; j < orderCount; ++j)
                            if (std::equal(oldOrder.begin(), oldOrder.end(), orders()[j].begin()))
                            {
                                old.setProperty("value", j, nullptr);
                                break;
                            }
                    }
                    for (int j = 0; j < migrated.getNumChildren(); ++j)
                        if (migrated.getChild(j).getProperty("id").toString() == target)
                            migrated.getChild(j).setProperty("value", old.getProperty("value"), nullptr);
                }
                for (int e = 0; e < effectCount; ++e)
                    for (int j = 0; j < migrated.getNumChildren(); ++j)
                        if (migrated.getChild(j).getProperty("id").toString() == presentId(e))
                            migrated.getChild(j).setProperty("value", e < 4 ? 1.0f : 0.0f, nullptr);
                state.replaceState(migrated);
            }
            else
            {
                if (version == 2)
                {
                    for (int i = 0; i < tree.getNumChildren(); ++i)
                        if (tree.getChild(i).getProperty("id").toString() == "order")
                        {
                            auto entry = tree.getChild(i);
                            int previous = juce::jlimit(0, 119, (int)entry.getProperty("value"));
                            std::array<int, 5> old{0, 1, 2, 3, 4};
                            while (previous-- > 0)
                                std::next_permutation(old.begin(), old.end());
                            for (int j = 0; j < orderCount; ++j)
                                if (std::equal(old.begin(), old.end(), orders()[j].begin()))
                                {
                                    entry.setProperty("value", j, nullptr);
                                    break;
                                }
                        }
                    auto present = juce::ValueTree("PARAM");
                    present.setProperty("id", presentId(5), nullptr);
                    present.setProperty("value", 0.0f, nullptr);
                    tree.appendChild(present, nullptr);
                }
                state.replaceState(tree);
            }
            auto installed = palette();
            if (std::find(installed.begin(), installed.end(), selectedEffect) == installed.end())
                selectedEffect = installed.empty() ? -1 : installed.front();
        }
}
// Host’un sahipliğine geçecek editörü oluşturur; Processor editörden daha uzun yaşamalıdır.
juce::AudioProcessorEditor *ShaperProcessor::createEditor()
{
    return new ShaperEditor(*this);
}
// JUCE_CALLTYPE createPluginFilter alt adımı: Rack efekt zinciri, MIDI/transport, preset, ses callback ve kaydedilen durum akışını yönetir.
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new ShaperProcessor();
}

// Fazıl Irak · 03.11.2026: İç snapshot A/B içermez; böylece sonsuz iç içe serileştirme oluşmaz.
void ShaperProcessor::getStateInformation(juce::MemoryBlock& data) {
    const juce::ScopedLock transaction(getCallbackLock());
    getRackStateInformation(data);
    if(auto xml=getXmlFromBinary(data.getData(),(int)data.getSize())) {
        xml->setAttribute("activeAB",activeAB);
        for(int i=0;i<2;++i) {
            auto* node=xml->createNewChildElement("CompareBank");node->setAttribute("index",i);
            node->addTextElement((i==activeAB?data:abStates[(size_t)i]).toBase64Encoding());
        }
        copyXmlToBinary(*xml,data);
    }
}
void ShaperProcessor::setStateInformation(const void* data,int size) {
    const juce::ScopedLock transaction(getCallbackLock());
    setRackStateInformation(data,size);
    if(auto xml=getXmlFromBinary(data,size)) if(xml->hasTagName("SHAPER")) {
        activeAB=juce::jlimit(0,1,xml->getIntAttribute("activeAB",0));
        for(auto& b:abStates)b.reset();
        for(auto* node:xml->getChildIterator()) if(node->hasTagName("CompareBank")) {
            int i=node->getIntAttribute("index",-1);if(i>=0&&i<2)abStates[(size_t)i].fromBase64Encoding(node->getAllSubText());
        }
    }
}
void ShaperProcessor::switchAB(int bank) {
    // Banka yakalama ve geri yükleme tek işlem; audio thread bu süre boyunca kuru geçer.
    const juce::ScopedLock transaction(getCallbackLock());
    bank=juce::jlimit(0,1,bank);if(bank==activeAB)return;
    getRackStateInformation(abStates[(size_t)activeAB]);
    if(abStates[(size_t)bank].isEmpty())abStates[(size_t)bank]=abStates[(size_t)activeAB];
    const int w=editorWidth.load(),h=editorHeight.load();
    setRackStateInformation(abStates[(size_t)bank].getData(),(int)abStates[(size_t)bank].getSize());
    editorWidth.store(w);editorHeight.store(h);activeAB=bank;
}
