// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (c) 2026 Fazıl Irak
// Geliştirici: Fazıl Irak
// Son düzenleme: 26.10.2026
// Dosyanın görevi: Arayüz kurulumu, yerleşim, meter/transfer çizimi, model/preset eylemleri ve dosya penceresi.
#include "PluginEditor.h"
#include "FSaturatorFonts.h"
namespace irak
{
// Görsel palet: turuncu vurgu, kırık beyaz yazı, koyu panel. Ses motorunu etkilemez.
namespace
{
const juce::Colour accent(0xffed6c46), ivory(0xffe5e7df), muted(0xff8d9799), panel(0xff1b2123),
    background(0xff141819);
}
// Fontlar binary data içinden yüklenir; kullanıcı bilgisayarındaki fontlara ihtiyaç yoktur.
Editor::Theme::Theme()
{
    book =
        juce::Typeface::createSystemTypefaceFor(fsaturator_fonts::Book_otf, fsaturator_fonts::Book_otfSize);
    demi =
        juce::Typeface::createSystemTypefaceFor(fsaturator_fonts::Demi_otf, fsaturator_fonts::Demi_otfSize);
    setColour(juce::TextButton::buttonColourId, juce::Colour(0xff222729));
    setColour(juce::TextButton::buttonOnColourId, accent);
    setColour(juce::TextButton::textColourOffId, juce::Colour(0xffc3cbc7));
    setColour(juce::TextButton::textColourOnId, juce::Colour(0xff101415));
    setColour(juce::ToggleButton::textColourId, ivory);
    setColour(juce::ToggleButton::tickColourId, accent);
    setColour(juce::ToggleButton::tickDisabledColourId, muted);
    setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff222729));
    setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff343b3e));
    setColour(juce::ComboBox::textColourId, ivory);
    setColour(juce::Label::textColourId, ivory);
    setColour(juce::TextEditor::backgroundColourId, background);
    setColour(juce::TextEditor::textColourId, ivory);
}
// Seçili model turuncu; hover/basılı durum koyu gri çizilir.
void Editor::Theme::drawButtonBackground(juce::Graphics &g, juce::Button &b, const juce::Colour &, bool over,
                                         bool down)
{
    if (b.getComponentID().startsWith("desk-"))
    {
        // Fast/Tight seçenekleri küçük LED anahtarlardır: dolu turuncu=açık, koyu=kapalı.
        const auto centre=juce::Point<float>(5.0f,b.getHeight()*.5f);
        g.setColour(b.getToggleState()?accent:juce::Colour(0xff293235));
        g.fillEllipse(centre.x-3,centre.y-3,6,6);
        g.setColour(b.getToggleState()?accent.brighter(.15f):juce::Colour(over||down?0xffa8b3b5:0xff596568));
        g.drawEllipse(centre.x-3.5f,centre.y-3.5f,7,7,.8f);
        return;
    }
    const auto r = b.getLocalBounds().toFloat().reduced(.7f);
    g.setColour(b.getToggleState() ? accent : juce::Colour(over || down ? 0xff343b3e : 0xff222729));
    g.fillRoundedRectangle(r, 4);
    g.setColour(b.getToggleState() ? accent : juce::Colour(0xff343b3e));
    g.drawRoundedRectangle(r, 4, 1.3f);
}
// LED etiketinin de tıklanabilen alana dahil olması küçük hedeflerin kullanımını kolaylaştırır.
void Editor::Theme::drawButtonText(juce::Graphics& g,juce::TextButton& b,bool over,bool down)
{
    if (!b.getComponentID().startsWith("desk-"))
    { juce::LookAndFeel_V4::drawButtonText(g,b,over,down); return; }
    g.setColour(b.getToggleState()?ivory:muted);
    g.setFont(juce::FontOptions(9));
    g.drawText(b.getButtonText(),13,0,b.getWidth()-13,b.getHeight(),juce::Justification::centredLeft);
}
// Alt şerit için F Saturator paletine uygun tek düğme. Orta işaret nötr, turuncu ibre geçerli değerdir.
void Editor::Theme::drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float pos,float start,float end,juce::Slider&)
{
    const float radius=juce::jmin((float)w,(float)h)*.43f;
    const juce::Point<float> centre{(float)x+w*.5f,(float)y+h*.5f};
    g.setColour(juce::Colour(0xff101415));g.fillEllipse(centre.x-radius,centre.y-radius,radius*2,radius*2);
    g.setColour(juce::Colour(0xff525b5e));g.drawEllipse(centre.x-radius,centre.y-radius,radius*2,radius*2,2);
    g.setColour(juce::Colour(0xff292f31));g.fillEllipse(centre.x-radius+5,centre.y-radius+5,radius*2-10,radius*2-10);
    const float a=start+pos*(end-start);
    g.setColour(accent);g.drawLine(centre.x+std::sin(a)*radius*.35f,centre.y-std::cos(a)*radius*.35f,
                                centre.x+std::sin(a)*radius*.82f,centre.y-std::cos(a)*radius*.82f,3);
}
// Kontrolleri oluştur, APVTS ile bağla ve callback’leri kur.
// Arayüz metinlerini değiştirmek parametre kimliklerini değiştirmeyi gerektirmez.
Editor::Editor(Processor &p) : AudioProcessorEditor(p), processor(p)
{
    setLookAndFeel(&theme);
    setComponentID("irak-editor");
    // Fazıl Irak: Character grubu Saturation ile Output arasındadır.
    // Her grubun sırası içten dışadır; eski parametre kimlikleri değişmez.
    groups[0] = std::make_unique<ConcentricKnob>(p.state, "COLOUR",
                                                 std::array<const char *, 3>{"tone", "lowcut", "highcut"},
                                                 std::array<const char *, 3>{"Tone", "Low Cut", "High Cut"});
    groups[1] = std::make_unique<ConcentricKnob>(p.state, "SATURATION",
                                                 std::array<const char *, 3>{"drive", "bias", "dynamics"},
                                                 std::array<const char *, 3>{"Drive", "Bias", "Dynamics"});
    groups[2] = std::make_unique<ConcentricKnob>(p.state, "CHARACTER",
                                                 std::array<const char *, 3>{"character", "harmonics", "drift"},
                                                 std::array<const char *, 3>{"Character", "Harmonic Balance", "Analog Drift"});
    groups[3] = std::make_unique<ConcentricKnob>(p.state, "OUTPUT",
                                                 std::array<const char *, 3>{"mix", "output", "width"},
                                                 std::array<const char *, 3>{"Mix", "Output", "Stereo"});
    for (int i = 0; i < 4; ++i)
    {
        groups[(size_t)i]->setComponentID("saturator-group-" + juce::String(i));
        addAndMakeVisible(*groups[(size_t)i]);
    }
    // Model düğmesi bir host gesture başlatıp sonlandırır; böylece DAW otomasyonu değişikliği takip edebilir.
    for (int i = 0; i < 5; ++i)
    {
        auto &b = characters[(size_t)i];
        b.setButtonText(modelNames[i]);
        b.setComponentID("irak-character-" + juce::String(i));
        addAndMakeVisible(b);
        b.onClick = [this, i]
        {
            auto *v = processor.state.getParameter("model");
            v->beginChangeGesture();
            v->setValueNotifyingHost(v->convertTo0to1((float)i));
            v->endChangeGesture();
        };
    }
    const char* stripIds[]{"transient", "comp", "bass", "treble"};
    const char* stripNames[]{"TRANSIENT", "COMP", "BASS", "TREBLE"};
    const char* stripTips[]{"Soft (-) / Sharp (+): shapes attacks before saturation. Double-click resets.",
        "Stereo-linked soft-knee feedback compression. 0% = off. Fast ATT / REL change timing.",
        "Low shelf +/-12 dB. Tight controls sub-bass and low-mid build-up.",
        "High shelf +/-12 dB. Tight shifts the curve and preserves presence when cutting."};
    for (int i=0;i<4;++i)
    {
        deskKnobs[(size_t)i]=std::make_unique<DeskKnob>(p.state,stripIds[i],stripNames[i],stripTips[i]);
        addAndMakeVisible(*deskKnobs[(size_t)i]);
    }
    const char* switchIds[]{"fast_attack", "fast_release", "bass_tight", "treble_tight"};
    const char* switchText[]{"ATT", "REL", "TIGHT", "TIGHT"};
    for(int i=0;i<4;++i)
    {
        auto& b=deskSwitches[(size_t)i]; b.setButtonText(switchText[i]);b.setClickingTogglesState(true);
        b.setComponentID("desk-"+juce::String(switchIds[i]));addAndMakeVisible(b);
        deskAttachments[(size_t)i]=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.state,switchIds[i],b);
    }
    deskSwitches[0].setTooltip("Comp attack: 22 ms normal / 1.5 ms fast.");
    deskSwitches[1].setTooltip("Comp release: 220 ms normal / 55 ms fast.");
    deskSwitches[2].setTooltip("Bass Tight: sub-bass control and broad low-mid attenuation when boosting.");
    deskSwitches[3].setTooltip("Treble Tight: lower shelf transition and a presence contour when cutting.");
    addAndMakeVisible(heat);
    addAndMakeVisible(autoGain);
    addAndMakeVisible(bypass);
    heat.setComponentID("saturator-heat");
    autoGain.setComponentID("saturator-auto-gain");
    bypass.setComponentID("saturator-bypass");
    heat.setTooltip("Adds 20 dB before saturation.");
    autoGain.setTooltip("Drive compensation calibrated on a -18 dBFS reference tone.");
    // JUCE Attachment, anahtarın kullanıcı ve otomasyon tarafından yapılan değişikliklerini çift yönlü eşler.
    // Attachment’lar kontrolleri ve Processor state’ini referanslar; bu üç nesne ömrü boyunca geçerli kalmalıdır.
    heatAttachment =
        std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.state, "heat", heat);
    autoAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        p.state, "auto_gain", autoGain);
    bypassAttachment =
        std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.state, "bypass", bypass);
    // JUCE ComboBox kimlikleri 1’den, DSP preset indeksleri 0’dan başlar; +1/-1 dönüşümünü koruyun.
    for (int i = 0; i < p.getNumPrograms(); ++i)
        presets.addItem(p.getProgramName(i), i + 1);
    addAndMakeVisible(presets);
    presets.setComponentID("saturator-preset");
    for (auto *b : {&previous, &next, &save, &load})
        addAndMakeVisible(b);
    previous.setComponentID("saturator-previous");
    next.setComponentID("saturator-next");
    presets.onChange = [this]
    {
        if (presets.getSelectedId() > 0)
            processor.setCurrentProgram(presets.getSelectedId() - 1);
    };
    // Preset bankasında başa/sona sararak gezinir; 16 sayısı fabrika preset sayısıyla eşleşir.
    previous.onClick = [this]
    { presets.setSelectedItemIndex((presets.getSelectedItemIndex() + 15) % 16, juce::sendNotificationSync); };
    next.onClick = [this]
    { presets.setSelectedItemIndex((presets.getSelectedItemIndex() + 1) % 16, juce::sendNotificationSync); };
    save.onClick = [this] { choosePreset(true); };
    load.onClick = [this] { choosePreset(false); };
    // Pencere 960×640 ile 1440×900 arasında; varsayılan 1040×640. Yeni küçük şerit eski yüksekliğe sığar. Timer yalnız GUI yenilemesi içindir.
    setResizable(true, false);
    setResizeLimits(960, 640, 1440, 900);
    setSize(1040, 640);
    timerCallback();
    startTimerHz(30);
}
// Timer ve grupları tema nesnesinden önce bırakır; sarkan LookAndFeel bağlantısını temizler.
Editor::~Editor()
{
    stopTimer();
    for (auto &group : groups)
        group.reset();
    for(auto& knob : deskKnobs) knob.reset();
    setLookAndFeel(nullptr);
}
// Host tarafından değiştirilen modeli/preseti ekrana yansıtır; yeni parametre bildirimi üretmez.
// Tepe göstergeleri görsel olarak sönümlenir; gösterge okuması DSP’nin ses çıkışını değiştirmez.
// 30 Hz GUI güncellemesi DSP’den atomik tepe değerleri okur; bu bir audio callback değildir. repaint ses işleme başlatmaz.
void Editor::timerCallback()
{
    const auto s = processor.settings();
    for (int i = 0; i < 5; ++i)
        characters[(size_t)i].setToggleState(i == s.model, juce::dontSendNotification);
    const int current = processor.getCurrentProgram();
    if (current != displayedProgram && !presets.isTextEditable())
        presets.setSelectedId(current + 1, juce::dontSendNotification);
    displayedProgram = current;
    inputLevel = juce::jmax(processor.dsp.inputPeak.load(), inputLevel * .88f);
    outputLevel = juce::jmax(processor.dsp.outputPeak.load(), outputLevel * .88f);
    repaint();
}
// Transfer eğrisinin statik önizlemesi; spektrum analizörü değildir. Drive/Heat/Bias görünümünü gösterir.
void Editor::curve(juce::Graphics &g, juce::Rectangle<float> r)
{
    g.setColour(juce::Colour(0xff101415));
    g.fillRoundedRectangle(r, 4);
    g.setColour(juce::Colour(0xff343c3f));
    g.drawRoundedRectangle(r, 4, 1);
    g.setFont(juce::FontOptions(9));
    g.setColour(muted);
    g.drawText("SATURATION CURVE", r.reduced(9).removeFromTop(14), juce::Justification::centredLeft);
    r = r.reduced(12, 22);
    g.setColour(juce::Colour(0xff293234));
    for (int i = 1; i < 8; ++i)
        g.drawVerticalLine((int)(r.getX() + r.getWidth() * i / 8), r.getY(), r.getBottom());
    for (int i = 1; i < 4; ++i)
        g.drawHorizontalLine((int)(r.getY() + r.getHeight() * i / 4), r.getX(), r.getRight());
    g.setColour(juce::Colour(0xff525b5e));
    g.drawLine(r.getX(), r.getBottom(), r.getRight(), r.getY(), 1);
    const auto s = processor.settings();
    const float drive = juce::Decibels::decibelsToGain(s.drive + (s.heat ? 20 : 0));
    juce::Path path;
    for (int i = 0; i <= 256; ++i)
    {
        const float x = i / 128.0f - 1, y = s.bypass ? x : SaturationDSP::preview(x * drive, s.model, s.bias, s.character, s.harmonics);
        const auto p =
            juce::Point<float>(r.getX() + r.getWidth() * i / 256,
                               r.getCentreY() - juce::jlimit(-1.0f, 1.0f, y) * r.getHeight() * .46f);
        if (!i)
            path.startNewSubPath(p);
        else
            path.lineTo(p);
    }
    g.setColour(accent);
    g.strokePath(path, juce::PathStrokeType(2));
}
// 0..-60 dBFS seviye çubuğu; 1.0 ve üzerindeki genlikte turuncu gösterim kullanılır.
void Editor::meter(juce::Graphics &g, juce::Rectangle<float> r, float level, const char *name)
{
    g.setFont(juce::FontOptions(10));
    g.setColour(muted);
    g.drawText(name, r.withY(r.getY() - 19).withHeight(15), juce::Justification::centred);
    g.setColour(juce::Colour(0xff101415));
    g.fillRoundedRectangle(r, 2);
    g.setColour(juce::Colour(0xff343c3f));
    g.drawRoundedRectangle(r, 2, 1);
    const float db = juce::Decibels::gainToDecibels(level, -60.0f),
                fill = juce::jlimit(0.0f, 1.0f, (db + 60) / 60);
    auto bar = r.reduced(4);
    bar.removeFromTop(bar.getHeight() * (1 - fill));
    g.setColour(level >= 1 ? accent : juce::Colour(0xff8faa95));
    g.fillRect(bar);
    g.setColour(juce::Colour(0xff343c3f));
    for (int i = 1; i < 8; ++i)
        g.drawHorizontalLine((int)(r.getY() + r.getHeight() * i / 8), r.getX() + 2, r.getRight() - 2);
    g.setColour(level >= 1 ? accent : muted);
    g.setFont(juce::FontOptions(9));
    g.drawText(db <= -60 ? "-inf" : juce::String(db, 1),
               r.withY(r.getBottom() + 3).withHeight(16).expanded(9, 0), juce::Justification::centred);
}
// Arka plan, başlık, ayırıcılar ve motor göstergelerini çizer; kontroller kendi paint yöntemlerini kullanır.
void Editor::paint(juce::Graphics &g)
{
    g.fillAll(background);
    const int divider = dockLeft ? 210 : 166;
    if (!dockLeft)
    {
        g.setColour(ivory);
        g.setFont(juce::FontOptions(37));
        g.drawText("F Saturator", 28, 38, 290, 58, juce::Justification::centredLeft);
        g.setColour(muted);
        g.setFont(juce::FontOptions(10));
        g.drawText("FAZIL IRAK / ANALOG COLOUR", 30, 101, 285, 20, juce::Justification::centredLeft);
    }
    curve(g, graphBounds);
    meter(g, inputMeter, inputLevel, "IN");
    meter(g, outputMeter, outputLevel, "OUT");
    g.setColour(accent);
    g.fillRect(28, divider, getWidth() - 56, 2);
    g.setColour(ivory);
    g.fillRect(28, divider + 5, getWidth() - 56, 2);
    g.setColour(panel);
    g.fillRoundedRectangle(22, (float)divider + 20, (float)getWidth() - 44,
                           (float)getHeight() - divider - (dockLeft ? 8 : 54), 8);
    const float column = (getWidth() - 56) / 4.0f;
    g.setColour(juce::Colour(0xff30383a));
    for (int i = 1; i < 4; ++i)
        g.drawVerticalLine((int)(28 + column * i), (float)divider + 36,
                           (float)divider + 326);
    // Fazıl Irak: ek çerçeveler ve bölüm başlığı kaldırıldı; tek kompakt satır eski model bankasını korur.
    const int stripY=getHeight()-(dockLeft?88:140), sideW=juce::jmin(184,(getWidth()-56)/5);
    g.setColour(muted);g.setFont(juce::FontOptions(8));
    g.drawText("SOFT          SHARP",29,stripY+70,sideW/2-2,13,juce::Justification::centred);
    if (!dockLeft)
    {
        g.setFont(juce::FontOptions(10));
        g.setColour(muted);
        g.drawText("8x FIR / ADAA", getWidth() - 195, getHeight() - 43, 165, 20,
                   juce::Justification::centredRight);
    }
}
// Tüm bileşenlerin piksel yerleşimi burada. dockLeft gömme alanı için ikinci yerleşimi seçer.
void Editor::resized()
{
    const float top = dockLeft ? 88.0f : 26.0f, right = (float)getWidth() - 30,
                graphLeft = dockLeft ? (float)dockLeft + 14 : 338;
    graphBounds = {graphLeft, top, juce::jmax(210.0f, right - graphLeft - 286), 112};
    const int checkX = (int)graphBounds.getRight() + 17;
    heat.setBounds(checkX, (int)top + 3, 140, 27);
    autoGain.setBounds(checkX, (int)top + 38, 140, 27);
    bypass.setBounds(checkX, (int)top + 73, 140, 27);
    inputMeter = {right - 98, top + 17, 29, 76};
    outputMeter = {right - 41, top + 17, 29, 76};
    const int divider = dockLeft ? 210 : 166, groupWidth = (getWidth() - 56) / 4;
    for (int i = 0; i < 4; ++i)
        groups[(size_t)i]->setBounds(28 + i * groupWidth, divider + 26, groupWidth, 304);
    // Fazıl Irak: 140 px yüksekliğindeki kutular yerine 84 px şerit, yaklaşık 40 px çaplı yeni düğmeler.
    const int stripY=getHeight()-(dockLeft?88:140), sideW=juce::jmin(184,(getWidth()-56)/5), knobW=sideW/2;
    const int rightX=getWidth()-28-sideW;
    deskKnobs[0]->setBounds(29,stripY,knobW-2,70);
    deskKnobs[1]->setBounds(29+knobW,stripY,knobW-2,70);
    deskKnobs[2]->setBounds(rightX+1,stripY,knobW-2,70);
    deskKnobs[3]->setBounds(rightX+1+knobW,stripY,knobW-2,70);
    const int fastW=(knobW-8)/2;
    deskSwitches[0].setBounds(29+knobW+4,stripY+70,fastW,13);
    deskSwitches[1].setBounds(29+knobW+5+fastW,stripY+70,fastW,13);
    deskSwitches[2].setBounds(rightX+knobW/2-24,stripY+70,50,13);
    deskSwitches[3].setBounds(rightX+knobW+knobW/2-24,stripY+70,50,13);
    const int bankWidth = getWidth()-56-2*sideW-20, buttonWidth = (bankWidth-24)/5, bankY=stripY+28;
    for (int i = 0; i < 5; ++i)
        characters[(size_t)i].setBounds((getWidth() - bankWidth) / 2 + i * (buttonWidth + 6), bankY,
                                        buttonWidth, 32);
    for (auto *c : {(juce::Component *)&presets, (juce::Component *)&previous, (juce::Component *)&next,
                    (juce::Component *)&save, (juce::Component *)&load})
        c->setVisible(dockLeft == 0);
    const int bottom = getHeight() - 48;
    previous.setBounds(28, bottom, 30, 28);
    presets.setBounds(66, bottom, 280, 28);
    next.setBounds(354, bottom, 30, 28);
    save.setBounds(400, bottom, 62, 28);
    load.setBounds(470, bottom, 62, 28);
}
// Gömülü kullanımın sağ tık/menü seçenekleri; 9001=Auto Gain, 9002=Heat, 9003=Bypass.
juce::PopupMenu Editor::rackOptions()
{
    juce::PopupMenu p;
    const auto s = processor.settings();
    p.addItem(9001, "Auto gain", true, s.autoGain);
    p.addItem(9002, "Heat +20 dB", true, s.heat);
    p.addItem(9003, "Bypass", true, s.bypass);
    return p;
}
// Menü seçimini ilgili bool parametrede host’a bildirilen bir aç/kapa hareketine dönüştürür.
void Editor::rackCommand(int command)
{
    const char *id = command == 9001   ? "auto_gain"
                     : command == 9002 ? "heat"
                     : command == 9003 ? "bypass"
                                       : nullptr;
    if (id)
    {
        auto *p = processor.state.getParameter(id);
        p->beginChangeGesture();
        p->setValueNotifyingHost(p->getValue() > .5f ? 0 : 1);
        p->endChangeGesture();
    }
}
// Belgeler/F Saturator Presets klasöründe asenkron dosya seçimi.
// .irakpreset dosyası Processor’ün DAW kaydıyla aynı binary XML durumunu içerir.
void Editor::choosePreset(bool saving)
{
    // Preset I/O yalnız mesaj iş parçacığında yürür. createDirectory/replaceWithData hata dönüşleri bu sürümde her durumda ayrı kullanıcı mesajına çevrilmiyor.
    const auto directory = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                               .getChildFile("F Saturator Presets");
    directory.createDirectory();
    chooser =
        std::make_unique<juce::FileChooser>(saving ? "Save F Saturator preset" : "Load F Saturator preset",
                                            directory.getChildFile("F Saturator.irakpreset"), "*.irakpreset");
    juce::Component::SafePointer<Editor> safe(this);
    // SafePointer, dosya penceresi açıkken editör kapanırsa artık geçersiz editöre erişimi önler.
    // Kaydet mevcut dosya için uyarı ister; yükleme yalnız IRAK_STATE kökünü kabul eder.
    // Asenkron callback Editor’den sonra çalışabilir; bu yüzden SafePointer kontrolü ilk koşuldur. Ham this yakalamasıyla değiştirmeyin.
    chooser->launchAsync(
        juce::FileBrowserComponent::canSelectFiles |
            (saving ? juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::warnAboutOverwriting
                    : juce::FileBrowserComponent::openMode),
        [safe, saving](const juce::FileChooser &fc)
        {
            if (!safe || fc.getResult() == juce::File())
                return;
            if (saving)
            {
                juce::MemoryBlock data;
                safe->processor.getStateInformation(data);
                fc.getResult().replaceWithData(data.getData(), data.getSize());
            }
            else
            {
                juce::MemoryBlock data;
                if (fc.getResult().loadFileAsData(data))
                    if (auto xml =
                            juce::AudioProcessor::getXmlFromBinary(data.getData(), (int)data.getSize()))
                        if (xml->hasTagName("IRAK_STATE"))
                        {
                            safe->processor.setStateInformation(data.getData(), (int)data.getSize());
                            safe->presets.setEditableText(true);
                            safe->presets.setText(fc.getResult().getFileNameWithoutExtension(),
                                                  juce::dontSendNotification);
                        }
            }
        });
}
} // namespace irak
