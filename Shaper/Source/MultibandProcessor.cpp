// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 Fazıl Irak; Shaper eklemeleri/değişiklikleri.
// Fazıl Irak tarafından oluşturuldu / değiştirildi: 23.10.2026
// Kapsam: Shaper için eklenen kod ve uyarlamalar; özgün üçüncü taraf telif/lisansları korunur.
// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: Üç bağımsız efekt motoru, bant ayırma/birleştirme, bypass ve bant durumu saklamasını yönetir.
#include "MultibandProcessor.h"
#include "ForwardParameter.h"
#include "BandGraph.h"
#include "Processor.h"
#include "EffectPalette.h"
#include "Embedded/RackEditorCommands.h"
#include "EffectOutputPanel.h"

namespace
{
// setPlain: Üç bağımsız efekt motoru, bant ayırma/birleştirme, bypass ve bant durumu saklamasını yönetir. Burada verilen ayar türetilmiş duruma/çıktıya aktarılır; çağrı sırası aşağıdaki gövdededir.
void setPlain(juce::AudioProcessor &engine, const char *id, float value)
{
    for (auto *p : engine.getParameters())
        if (auto *r = dynamic_cast<juce::RangedAudioParameter *>(p))
            if (r->paramID == id)
                r->setValueNotifyingHost(r->convertTo0to1(value));
}
class BandEditor final : public juce::AudioProcessorEditor, public RackEditorCommands, private juce::Timer
{
    MultibandProcessor &owner;
    juce::TextButton bands{"Bands"};
    std::array<juce::TextButton, 3> tabs;
    BandGraph graph;
    EffectOutputPanel output;
    juce::TextButton fullRange{"Full range"};
    juce::Label hint;
    std::unique_ptr<juce::AudioProcessorEditor> child;
    bool shown = false;
    int bandShown = -1;
    // Mevcut ayarlara bağlı türetilmiş görünüm/katsayı/cache durumunu yeniden kurar.
    // Gövdedeki başlıca çağrılar: owner.editorBand → child.reset → owner.band → createEditor. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    void rebuild()
    {
        shown = owner.showBandsEditor;
        bandShown = owner.editorBand();
        child.reset();
        child.reset(owner.band(bandShown).createEditor());
        addAndMakeVisible(*child);
        for (int b = 0; b < 3; ++b)
        {
            tabs[b].setVisible(shown);
            tabs[b].setEnabled(b == 1 || (b == 0 ? owner.hasLowBand() : owner.hasHighBand()));
            tabs[b].setToggleState(b == bandShown, juce::dontSendNotification);
        }
        graph.setVisible(shown);
        fullRange.setVisible(shown);
        bands.setToggleState(shown, juce::dontSendNotification);
        hint.setText(shown ? "Drag the edges to add Low / High. Double-click an edge to reset."
                           : "Mid processes the full range until you move a band edge.",
                     juce::dontSendNotification);
        if (auto *c = commands())
            c->rackDockHeader(panelWidth() + 16);
        for (auto *c : {(juce::Component *)&graph, (juce::Component *)&bands, (juce::Component *)&tabs[0],
                        (juce::Component *)&tabs[1], (juce::Component *)&tabs[2],
                        (juce::Component *)&fullRange, (juce::Component *)&hint, (juce::Component *)&output})
            c->toFront(false);
        resized();
        repaint();
    }
    // panelWidth alt adımı: Üç bağımsız efekt motoru, bant ayırma/birleştirme, bypass ve bant durumu saklamasını yönetir.
    int panelWidth() const
    {
        return 300;
    }
    // commands alt adımı: Üç bağımsız efekt motoru, bant ayırma/birleştirme, bypass ve bant durumu saklamasını yönetir.
    RackEditorCommands *commands() const
    {
        return dynamic_cast<RackEditorCommands *>(child.get());
    }
    // Mesaj iş parçacığındaki periyodik görünüm eşitlemesini yapar; audio callback değildir.
    void timerCallback() override
    {
        if (shown != owner.showBandsEditor || bandShown != owner.editorBand())
            rebuild();
        for (int b = 0; b < 3; ++b)
            tabs[b].setEnabled(b == 1 || (b == 0 ? owner.hasLowBand() : owner.hasHighBand()));
    }

  public:
    // BandEditor alt adımı: Üç bağımsız efekt motoru, bant ayırma/birleştirme, bypass ve bant durumu saklamasını yönetir.
    // Gövdedeki başlıca çağrılar: setComponentID → bands.setComponentID → addAndMakeVisible → bands.setTooltip. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    // Eşzamanlılık sınırı: bu kilidin hangi thread’de alındığını kontrol edin; audio callback’e yeni bekleyen kilit taşımayın.
    explicit BandEditor(MultibandProcessor &p) : AudioProcessorEditor(p), owner(p), graph(p), output(p)
    {
        setComponentID("multiband-editor");
        bands.setComponentID("effect-bands");
        addAndMakeVisible(bands);
        bands.setTooltip("Show the graphical band editor (opening it does not change the sound)");
        bands.onClick = [this]
        {
            const juce::ScopedLock lock(owner.getCallbackLock());
            owner.showBandsEditor = !owner.showBandsEditor;
            rebuild();
        };
        const char *names[]{"Low", "Mid", "High"};
        for (int b = 0; b < 3; ++b)
        {
            tabs[b].setButtonText(names[b]);
            tabs[b].setComponentID("band-" + juce::String(b));
            addAndMakeVisible(tabs[b]);
            tabs[b].onClick = [this, b]
            {
                owner.selectedBand = b;
                rebuild();
            };
        }
        for (auto *button : {&bands, &tabs[0], &tabs[1], &tabs[2]})
        {
            button->setColour(juce::TextButton::textColourOffId, rackEffectColour(owner.effect));
            button->setColour(juce::TextButton::buttonOnColourId,
                              rackEffectColour(owner.effect).withMultipliedBrightness(0.35f));
        }
        addAndMakeVisible(graph);
        addAndMakeVisible(fullRange);
        addAndMakeVisible(hint);
        addAndMakeVisible(output);
        fullRange.setComponentID("bands-full-range");
        fullRange.onClick = [this]
        {
            owner.resetBands();
            rebuild();
        };
        graph.onSelection = [this]
        {
            if (bandShown != owner.editorBand())
                rebuild();
            else
            {
                for (int b = 0; b < 3; ++b)
                    tabs[b].setEnabled(b == 1 || (b == 0 ? owner.hasLowBand() : owner.hasHighBand()));
            }
        };
        setSize(1060, 700);
        rebuild();
        startTimerHz(10);
    }
    // ~BandEditor alt adımı: Üç bağımsız efekt motoru, bant ayırma/birleştirme, bypass ve bant durumu saklamasını yönetir.
    ~BandEditor() override
    {
        stopTimer();
        child.reset();
    }
    // Mevcut kontrol/durumun görselini çizer; ses callback’inde çağrılmamalıdır.
    void paint(juce::Graphics &g) override
    {
        g.fillAll(juce::Colour(0xff141820));
    }
    // Pencere/kontrol boyutuna bağlı yerleşimi yeniden hesaplar; hit-test aynı geometriyi izlemelidir.
    // Gövdedeki başlıca çağrılar: panelWidth → commands → rackHeaderHeight → child->setBounds. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    void resized() override
    {
        const int width = panelWidth(), header = commands() ? commands()->rackHeaderHeight() : 200;
        if (child)
        {
            child->setBounds(getLocalBounds());
            if (auto *c = commands())
                c->rackDockHeader(width + 16);
        }
        bands.setBounds(0, 5, 50, 24);
        for (int b = 0; b < 3; ++b)
            tabs[b].setBounds(56 + b * 46, 5, 42, 24);
        fullRange.setBounds(width - 106, 5, 106, 24);
        graph.setBounds(0, 36, width, juce::jmax(55, header - 70));
        hint.setBounds(0, header - 29, width, 24);
        hint.setText(shown ? "Drag edges to add bands / double-click to reset" : "Mid / Full range",
                     juce::dontSendNotification);
        hint.setFont(juce::FontOptions(12));
        output.setBounds(getWidth() - 184, 2, 180, 76);
    }
    // rackOptions alt adımı: Üç bağımsız efekt motoru, bant ayırma/birleştirme, bypass ve bant durumu saklamasını yönetir.
    juce::PopupMenu rackOptions() override
    {
        return commands() ? commands()->rackOptions() : juce::PopupMenu{};
    }
    // rackCommand alt adımı: Üç bağımsız efekt motoru, bant ayırma/birleştirme, bypass ve bant durumu saklamasını yönetir.
    void rackCommand(int c) override
    {
        if (commands())
            commands()->rackCommand(c);
    }
    // rackCanEdit alt adımı: Üç bağımsız efekt motoru, bant ayırma/birleştirme, bypass ve bant durumu saklamasını yönetir.
    bool rackCanEdit() const override
    {
        return commands() && commands()->rackCanEdit();
    }
    // rackIsSequencing alt adımı: Üç bağımsız efekt motoru, bant ayırma/birleştirme, bypass ve bant durumu saklamasını yönetir.
    bool rackIsSequencing() const override
    {
        return commands() && commands()->rackIsSequencing();
    }
};
} // namespace

// MultibandProcessor alt adımı: Üç bağımsız efekt motoru, bant ayırma/birleştirme, bypass ve bant durumu saklamasını yönetir.
// Gövdedeki başlıca çağrılar: m.reset → factory → band → getParameters. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
MultibandProcessor::MultibandProcessor(std::function<juce::AudioProcessor *()> factory, int e)
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withInput("Sidechain", juce::AudioChannelSet::stereo(), false)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      effect(e)
{
    for (auto &m : engines)
        m.reset(factory());
    // Preserve all existing single-band host parameter IDs for the Mid/full engine.
    for (int b : {1, 0, 2})
        for (auto *p : band(b).getParameters())
            if (auto *r = dynamic_cast<juce::RangedAudioParameter *>(p))
                addParameter(new ForwardParameter((b == 1   ? juce::String{}
                                                   : b == 0 ? "low_"
                                                            : "high_") +
                                                      r->paramID,
                                                  (b == 0   ? "Low "
                                                   : b == 2 ? "High "
                                                            : "") +
                                                      r->getName(64),
                                                  *r));
    addParameter(enabled =
                     new juce::AudioParameterBool(juce::ParameterID{"multiband", 1}, "Multiband", true));
    addParameter(low = new juce::AudioParameterFloat(juce::ParameterID{"crossover_low", 1}, "Low crossover",
                                                     juce::NormalisableRange<float>(20, 20000, 1, 0.3f), 20));
    addParameter(
        high = new juce::AudioParameterFloat(juce::ParameterID{"crossover_high", 1}, "High crossover",
                                             juce::NormalisableRange<float>(20, 20000, 1, 0.3f), 20000));
    for (int b = 0; b < 3; ++b)
    {
        const juce::String prefix = b == 0 ? "low_" : b == 2 ? "high_" : "";
        if (effect < 6)
            addParameter(outputGain[(size_t)b] =
                             new juce::AudioParameterFloat(juce::ParameterID{prefix + "output_gain", 1},
                                                           prefix + "Output gain", -60.0f, 12.0f, 0.0f));
        if (effect == 1)
            addParameter(outputWidth[(size_t)b] =
                             new juce::AudioParameterFloat(juce::ParameterID{prefix + "stereo_width", 1},
                                                           prefix + "Stereo width", 0.0f, 2.0f, 1.0f));
    }
}
// outputParameter alt adımı: Üç bağımsız efekt motoru, bant ayırma/birleştirme, bypass ve bant durumu saklamasını yönetir.
// Gövdedeki başlıca çağrılar: band → getParameters. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
juce::RangedAudioParameter *MultibandProcessor::outputParameter(int b, const juce::String &kind)
{
    if (kind == "gain" && effect < 6)
        return outputGain[(size_t)b];
    const auto id = kind == "mix"      ? (effect == 3 ? "drywet" : "mix")
                    : kind == "stereo" ? (effect == 3 || effect >= 6 ? "width" : "stereo")
                    : effect >= 6      ? "output"
                                       : "gain";
    for (auto *parameter : band(b).getParameters())
        if (auto *p = dynamic_cast<juce::RangedAudioParameter *>(parameter))
            if (p->paramID == id)
                return p;
    return kind == "gain" ? outputGain[(size_t)b] : kind == "stereo" ? outputWidth[(size_t)b] : nullptr;
}
// ~MultibandProcessor alt adımı: Üç bağımsız efekt motoru, bant ayırma/birleştirme, bypass ve bant durumu saklamasını yönetir.
MultibandProcessor::~MultibandProcessor()
{
    for (auto *p : getParameters())
        if (auto *f = dynamic_cast<ForwardParameter *>(p))
            f->detach();
}
// getName: Üç bağımsız efekt motoru, bant ayırma/birleştirme, bypass ve bant durumu saklamasını yönetir. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
const juce::String MultibandProcessor::getName() const
{
    return ShaperProcessor::effectName(effect);
}
// Fabrika/kullanıcı program seçimini mevcut parametre/durum akışına uygular.
void MultibandProcessor::setCurrentProgram(int i)
{
    current().setCurrentProgram(i);
    setPlain(current(), "split_low", 20);
    setPlain(current(), "split_high", 20000);
}
// Desteklenen giriş/çıkış kanal düzenini doğrular; synth ve FX giriş sözleşmeleri farklı olabilir.
bool MultibandProcessor::isBusesLayoutSupported(const BusesLayout &l) const
{
    return l.getMainInputChannelSet() == juce::AudioChannelSet::stereo() &&
           l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}
// setMultiband: Üç bağımsız efekt motoru, bant ayırma/birleştirme, bypass ve bant durumu saklamasını yönetir. Burada verilen ayar türetilmiş duruma/çıktıya aktarılır; çağrı sırası aşağıdaki gövdededir.
void MultibandProcessor::setMultiband(bool value)
{
    if (value)
        for (auto &engine : engines)
        {
            setPlain(*engine, "split_low", 20);
            setPlain(*engine, "split_high", 20000);
        }
    enabled->setValueNotifyingHost(value ? 1.0f : 0.0f);
}
// Host hazırlığında örnekleme, blok kapasitesi ve bildirilecek gecikmeyi kurar.
// Gövdedeki başlıca çağrılar: b.setSize → full.setSize → m->setRateAndBufferSizeDetails → m->prepareToPlay. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
void MultibandProcessor::prepareToPlay(double sr, int n)
{
    capacity = n;
    for (auto &b : audio)
        b.setSize(4, n);
    full.setSize(4, n);
    for (auto &m : engines)
    {
        m->setRateAndBufferSizeDetails(sr, n);
        m->prepareToPlay(sr, n);
    }
    for (auto &m : events)
        m.ensureSize(65536);
    juce::dsp::ProcessSpec spec{sr, (juce::uint32)n, 2};
    for (auto *f : {&lowSplit, &highSplit, &lowPhase})
    {
        f->prepare(spec);
        f->reset();
    }
    lowPhase.setType(juce::dsp::LinkwitzRileyFilterType::allpass);
    blend.reset(sr, 0.02);
    blend.setCurrentAndTargetValue(isMultiband() ? 1.0f : 0.0f);
    setLatencySamples(band(1).getLatencySamples());
    for (int b = 0; b < 3; ++b)
    {
        gainSmoothing[(size_t)b].reset(sr, .02);
        widthSmoothing[(size_t)b].reset(sr, .02);
        gainSmoothing[(size_t)b].setCurrentAndTargetValue(
            outputGain[(size_t)b] ? juce::Decibels::decibelsToGain(outputGain[(size_t)b]->get()) : 1.0f);
        widthSmoothing[(size_t)b].setCurrentAndTargetValue(
            outputWidth[(size_t)b] ? outputWidth[(size_t)b]->get() : 1.0f);
    }
}
// Host kaynak bırakma aşamasıdır; boşsa sahiplik nesne/yeniden hazırlık yaşam döngüsündedir.
void MultibandProcessor::releaseResources()
{
    for (auto &m : engines)
        m->releaseResources();
    capacity = 0;
}
// Host audio callback’ini işler; burada dosya/GUI veya bekleyen kilit eklemeyin.
// Gövdedeki başlıca çağrılar: buffer.getNumSamples → spectrumVisible.load → buffer.getSample → spectrumWrite.load. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
void MultibandProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midi)
{
    if (capacity == 0)
        return;
    const int n = buffer.getNumSamples();
    if (spectrumVisible.load(std::memory_order_relaxed))
        for (int i = 0; i < n; ++i)
        {
            spectrumPending[(size_t)spectrumFill++] =
                0.5f * (buffer.getSample(0, i) + buffer.getSample(1, i));
            if (spectrumFill == spectrumSize)
            {
                int write = spectrumWrite.load(std::memory_order_relaxed), next = (write + 1) % 4;
                if (next != spectrumRead.load(std::memory_order_acquire))
                {
                    spectrumFrames[(size_t)write] = spectrumPending;
                    spectrumWrite.store(next, std::memory_order_release);
                }
                spectrumFill = 0;
            }
        }
    const bool useLow = hasLowBand(), useHigh = hasHighBand(), active = useLow || useHigh;
    // The untouched full-range route never enters a crossover. With two splits,
    // the low band gets the high crossover's allpass phase before recombination.
    const float upper = juce::jmin(juce::jmax(high->get(), low->get() + 1), (float)getSampleRate() * 0.45f);
    lowSplit.setCutoffFrequency(juce::jmin(low->get(), upper - 1));
    highSplit.setCutoffFrequency(upper);
    lowPhase.setCutoffFrequency(upper);
    for (auto &b : audio)
        b.setSize(4, n, false, false, true);
    full.setSize(4, n, false, false, true);
    for (int c = 0; c < 2; ++c)
        for (int i = 0; i < n; ++i)
        {
            const float input = buffer.getSample(c, i);
            float l = 0, rest = input, m = input, h = 0;
            if (useLow)
                lowSplit.processSample(c, input, l, rest);
            if (useHigh)
            {
                highSplit.processSample(c, rest, m, h);
                if (useLow)
                    l = lowPhase.processSample(c, l);
            }
            else
                m = rest;
            audio[0].setSample(c, i, l);
            audio[1].setSample(c, i, m);
            audio[2].setSample(c, i, h);
            full.setSample(c, i, buffer.getSample(c, i));
        }
    for (int c = 2; c < 4; ++c)
        for (int i = 0; i < n; ++i)
        {
            auto value = c < buffer.getNumChannels() ? buffer.getSample(c, i) : 0.0f;
            for (auto &b : audio)
                b.setSample(c, i, value);
            full.setSample(c, i, value);
        }
    blend.setTargetValue(active ? 1.0f : 0.0f);
    const bool processSides = active || blend.isSmoothing();
    // No extra full-range engine: switch routing at the input of the Mid engine,
    // then fade side bands in/out. Each band has its own persistent DSP history.
    for (int i = 0; i < n; ++i)
    {
        auto v = blend.getNextValue();
        for (int c = 0; c < 2; ++c)
        {
            audio[1].setSample(c, i, full.getSample(c, i) * (1 - v) + audio[1].getSample(c, i) * v);
            audio[0].setSample(c, i, audio[0].getSample(c, i) * v);
            audio[2].setSample(c, i, audio[2].getSample(c, i) * v);
        }
    }
    for (int b = 0; b < 3; ++b)
    {
        if ((b == 0 && !useLow) || (b == 2 && !useHigh))
        {
            outputPeakLeft[(size_t)b].store(0, std::memory_order_relaxed);
            outputPeakRight[(size_t)b].store(0, std::memory_order_relaxed);
            continue;
        }
        band(b).setPlayHead(getPlayHead());
        events[b].clear();
        events[b].addEvents(midi, 0, n, 0);
        band(b).processBlock(audio[b], events[b]);
        auto &gain = gainSmoothing[(size_t)b];
        auto &width = widthSmoothing[(size_t)b];
        gain.setTargetValue(
            outputGain[(size_t)b] ? juce::Decibels::decibelsToGain(outputGain[(size_t)b]->get()) : 1.0f);
        width.setTargetValue(outputWidth[(size_t)b] ? outputWidth[(size_t)b]->get() : 1.0f);
        for (int i = 0; i < n; ++i)
        {
            float level = gain.getNextValue(), stereo = width.getNextValue();
            float l = audio[b].getSample(0, i), r = audio[b].getSample(1, i);
            if (stereo != 1.0f)
            {
                const float mid = (l + r) * .5f, side = (l - r) * .5f * stereo;
                l = mid + side;
                r = mid - side;
            }
            audio[b].setSample(0, i, l * level);
            audio[b].setSample(1, i, r * level);
        }
        outputPeakLeft[(size_t)b].store(audio[b].getMagnitude(0, 0, n), std::memory_order_relaxed);
        outputPeakRight[(size_t)b].store(audio[b].getMagnitude(1, 0, n), std::memory_order_relaxed);
    }
    for (int c = 0; c < 2; ++c)
    {
        buffer.copyFrom(c, 0, audio[1], c, 0, n);
        if (processSides)
        {
            if (useLow)
                buffer.addFrom(c, 0, audio[0], c, 0, n);
            if (useHigh)
                buffer.addFrom(c, 0, audio[2], c, 0, n);
        }
    }
}
// Host’un sahipliğine geçecek editörü oluşturur; Processor editörden daha uzun yaşamalıdır.
juce::AudioProcessorEditor *MultibandProcessor::createEditor()
{
    return new BandEditor(*this);
}
// Host oturumu için mevcut parametre ve kullanıcı durumunu seri hale getirir.
// Gövdedeki başlıca çağrılar: xml → xml.setAttribute → enabled->get → low->get. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
void MultibandProcessor::getStateInformation(juce::MemoryBlock &data)
{
    juce::XmlElement xml("MULTIBAND");
    xml.setAttribute("bandFormat", 2);
    xml.setAttribute("enabled", enabled->get());
    xml.setAttribute("graphVisible", showBandsEditor);
    xml.setAttribute("low", low->get());
    xml.setAttribute("high", high->get());
    xml.setAttribute("selected", selectedBand);
    for (int b = 0; b < 3; ++b)
    {
        juce::MemoryBlock block;
        band(b).getStateInformation(block);
        auto *node = xml.createNewChildElement("Band");
        node->setAttribute("index", b);
        if (outputGain[(size_t)b])
            node->setAttribute("outputGain", outputGain[(size_t)b]->get());
        if (outputWidth[(size_t)b])
            node->setAttribute("outputWidth", outputWidth[(size_t)b]->get());
        node->addTextElement(block.toBase64Encoding());
    }
    copyXmlToBinary(xml, data);
}
// Host/preset durumunu okuyup uygular; kök/sürüm denetimleri eski oturum uyumluluğu içindir.
// Gövdedeki başlıca çağrılar: getXmlFromBinary → xml->hasTagName → band → p->setValueNotifyingHost. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
void MultibandProcessor::setStateInformation(const void *data, int size)
{
    if (auto xml = getXmlFromBinary(data, size))
    {
        if (!xml->hasTagName("MULTIBAND"))
        {
            if (xml->hasTagName("PluginState"))
            {
                band(1).setStateInformation(data, size);
                for (int b = 0; b < 3; ++b)
                {
                    if (auto *p = outputGain[(size_t)b])
                        p->setValueNotifyingHost(p->convertTo0to1(0));
                    if (auto *p = outputWidth[(size_t)b])
                        p->setValueNotifyingHost(p->convertTo0to1(1));
                }
                setMultiband(false);
                selectedBand = 1;
            }
            return;
        }
        if (!xml->hasAttribute("bandFormat") && !xml->getBoolAttribute("enabled", false))
        {
            xml->setAttribute("low", 20);
            xml->setAttribute("high", 20000);
            xml->setAttribute("enabled", true);
        }
        for (int b = 0; b < 3; ++b)
        {
            if (auto *p = outputGain[(size_t)b])
                p->setValueNotifyingHost(p->convertTo0to1(0));
            if (auto *p = outputWidth[(size_t)b])
                p->setValueNotifyingHost(p->convertTo0to1(1));
        }
        for (auto *node : xml->getChildIterator())
        {
            juce::MemoryBlock block;
            int b = node->getIntAttribute("index", -1);
            if (b >= 0 && b < 3 && block.fromBase64Encoding(node->getAllSubText()))
            {
                band(b).setStateInformation(block.getData(), (int)block.getSize());
                if (auto *p = outputGain[(size_t)b])
                    p->setValueNotifyingHost(
                        p->convertTo0to1((float)node->getDoubleAttribute("outputGain", 0)));
                if (auto *p = outputWidth[(size_t)b])
                    p->setValueNotifyingHost(
                        p->convertTo0to1((float)node->getDoubleAttribute("outputWidth", 1)));
            }
        }
        low->setValueNotifyingHost(low->convertTo0to1((float)xml->getDoubleAttribute("low", 20)));
        high->setValueNotifyingHost(high->convertTo0to1((float)xml->getDoubleAttribute("high", 20000)));
        selectedBand = juce::jlimit(0, 2, xml->getIntAttribute("selected", 1));
        setMultiband(xml->getBoolAttribute("enabled", true));
        showBandsEditor = xml->getBoolAttribute("graphVisible", true);
    }
}

// resetBands alt adımı: Üç bağımsız efekt motoru, bant ayırma/birleştirme, bypass ve bant durumu saklamasını yönetir.
void MultibandProcessor::resetBands()
{
    low->beginChangeGesture();
    low->setValueNotifyingHost(low->convertTo0to1(20));
    low->endChangeGesture();
    high->beginChangeGesture();
    high->setValueNotifyingHost(high->convertTo0to1(20000));
    high->endChangeGesture();
    selectedBand = 1;
    setMultiband(true);
}
// readSpectrum: Üç bağımsız efekt motoru, bant ayırma/birleştirme, bypass ve bant durumu saklamasını yönetir. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
bool MultibandProcessor::readSpectrum(std::array<float, spectrumSize> &result)
{
    bool got = false;
    int read = spectrumRead.load(std::memory_order_relaxed);
    const int write = spectrumWrite.load(std::memory_order_acquire);
    while (read != write)
    {
        result = spectrumFrames[(size_t)read];
        read = (read + 1) % 4;
        got = true;
    }
    spectrumRead.store(read, std::memory_order_release);
    return got;
}
