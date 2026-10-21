#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace irak {
// Three independent parameter targets, matching F EQ's cap / ivory / orange rings.
// Tek panelde üç bağımsız parametre halkası: 0 iç kapak, 1 orta halka, 2 dış halka.
// Halka seçimi geometriktir; her hedefin kendi Attachment nesnesi vardır.
class ConcentricKnob final : public juce::Component, public juce::SettableTooltipClient {
// APVTS’nin sahip olduğu parametrelere işaretçiler; burada silinmezler.
    std::array<juce::RangedAudioParameter*,3> parameters{};
// Parametre otomasyonunu etiketlere/çizime, GUI hareketini host’a taşır.
    std::array<std::unique_ptr<juce::ParameterAttachment>,3> attachments;
    std::array<juce::Label,3> readouts;
    std::array<juce::String,3> names,ids;
// plain fiziksel değerdir (Hz, dB veya oran); position ise çizim/sürükleme için 0..1 değeridir.
    std::array<float,3> plain{};
    juce::String title;
// -1 aktif halka yok demektir. startValue sürükleme başlangıcını tutar; tekerlek bağımsız tam gesture kullanır.
    int dragging=-1,hovered=-1;
    float startValue=0;
// Görsel yardımcılar: vurgu rengi, halka merkezi ve panele sığan yarıçap.
    static juce::Colour orange() { return juce::Colour(0xffed6c46); }
    juce::Point<float> centre() const { return {(float)getWidth()*.5f,124.0f}; }
    float radius() const { return juce::jmin(94.0f,(float)getWidth()*.34f); }
// Yalnız Low Cut ve High Cut, ekranda gerçek logaritmik frekans ölçeği kullanır.
    bool frequency(int i) const { return ids[(size_t)i]=="lowcut"||ids[(size_t)i]=="highcut"; }
// Fiziksel değeri halka konumuna dönüştürür. Frekansta log oranı kullanılır;
// bu dönüşüm DAW’ın mevcut parametre normalizasyonunu değiştirmez.
    float position(int i) const {
        auto& p=*parameters[(size_t)i];
        return frequency(i)?std::log(plain[(size_t)i]/p.getNormalisableRange().start)/std::log(p.getNormalisableRange().end/p.getNormalisableRange().start):p.convertTo0to1(plain[(size_t)i]);
    }
// Konumu fiziksel değere geri çevirir; uçlara taşmayı önlemek için önce 0..1’e sınırlar.
    float fromPosition(int i,float v) const {
        auto& p=*parameters[(size_t)i]; v=juce::jlimit(0.0f,1.0f,v);
        return frequency(i)?p.getNormalisableRange().start*std::pow(p.getNormalisableRange().end/p.getNormalisableRange().start,v):p.convertFrom0to1(v);
    }
// Kullanıcı gösterimi: Drive/Output dB, kesimler Hz/kHz, diğer kontroller yüzde.
// Yüzde değerleri DSP’ye giderken tekrar orana dönüştürülür.
    juce::String formatted(int i) const {
        const auto& id=ids[(size_t)i]; const float v=plain[(size_t)i];
        if(id=="drive"||id=="output") return juce::String(v,1)+" dB";
        if(frequency(i)) return v>=1000?juce::String(v/1000,2)+" kHz":juce::String(juce::roundToInt(v))+" Hz";
        return juce::String(juce::roundToInt(v*100))+"%";
    }
// Parametre bildirimi geldiğinde saklanan fiziksel değeri ve etiket yazısını günceller.
    void update(int i,float value) { plain[(size_t)i]=value; readouts[(size_t)i].setText(names[(size_t)i]+"   "+formatted(i),juce::dontSendNotification); repaint(); }
// JUCE parametresinin tanımlı varsayılanını fiziksel değere çevirip tek host gesture ile uygular.
    void resetPart(int i) { if(i>=0) attachments[(size_t)i]->setValueAsCompleteGesture(parameters[(size_t)i]->convertFrom0to1(parameters[(size_t)i]->getDefaultValue())); }
// 0..1 konumunu toplam 270 derecelik gösterge açısına çevirir.
    void pointer(juce::Graphics& g,float r,float v,juce::Colour colour,float width) {
        const float a=(-225+270*v)*juce::MathConstants<float>::pi/180; const auto c=centre();
        g.setColour(colour); g.drawLine(c.x+std::cos(a)*r*.78f,c.y+std::sin(a)*r*.78f,c.x+std::cos(a)*r*.95f,c.y+std::sin(a)*r*.95f,width);
    }
public:
// Her halka için etiket, çift tıklamayla sayısal giriş ve otomasyon bağlantısı oluşturur.
    ConcentricKnob(juce::AudioProcessorValueTreeState& state,juce::String heading,std::array<const char*,3> parameterIds,std::array<const char*,3> labels):title(heading) {
        setWantsKeyboardFocus(true);
        for(int i=0;i<3;++i) {
            const auto n=(size_t)i; ids[n]=parameterIds[n]; names[n]=labels[n]; parameters[n]=state.getParameter(ids[n]);
            auto& label=readouts[n]; addAndMakeVisible(label); label.setComponentID("irak-"+ids[n]); label.setJustificationType(juce::Justification::centred); label.setFont(juce::FontOptions(13));
            label.setColour(juce::Label::textColourId,i==2?orange():i==1?juce::Colour(0xffe5e7df):juce::Colour(0xffc3cbc7));
            label.setEditable(false,true,false); label.setTooltip("Double-click to enter "+names[n]);
// Sayısal düzenleyici açılırken yalnız biçimlendirilmiş değeri seçer; başlığı girişten çıkarır.
            label.onEditorShow=[this,i] { if(auto* editor=readouts[(size_t)i].getCurrentTextEditor()) { editor->setText(formatted(i),false); editor->selectAll(); } };
// kHz girişi 1000 ile, yüzde girişi 0.01 ile ölçeklenir; son değer parametre sınırlarına çekilir.
            label.onTextChange=[this,i] { const auto text=readouts[(size_t)i].getText().fromLastOccurrenceOf("   ",false,false); float v=text.getFloatValue(); if(frequency(i)&&text.containsIgnoreCase("khz")) v*=1000; else if(!frequency(i)&&ids[(size_t)i]!="drive"&&ids[(size_t)i]!="output") v*=.01f; const auto& range=parameters[(size_t)i]->getNormalisableRange(); attachments[(size_t)i]->setValueAsCompleteGesture(juce::jlimit(range.start,range.end,v)); update(i,plain[(size_t)i]); };
// İlk bildirim, çizim başlamadan etiketlerin mevcut parametre değerlerini almasını sağlar.
            attachments[n]=std::make_unique<juce::ParameterAttachment>(*parameters[n],[this,i](float v) { update(i,v); }); attachments[n]->sendInitialUpdate();
        }
        setTooltip("Inner: "+names[0]+" / Middle: "+names[1]+" / Outer: "+names[2]+". Drag up/down or scroll. Shift: fine. Double-click: reset.");
    }
// Sürükleme sırasında kapanırsa açık otomasyon gesture’ını sonlandırır.
    ~ConcentricKnob() override { if(dragging>=0) attachments[(size_t)dragging]->endGesture(); }
// Merkeze uzaklığa göre yalnız hedef halkayı seçer; halkanın dışındaki tıklama -1 döner.
    int partAt(juce::Point<float> point) const {
        const float d=point.getDistanceFrom(centre()),r=radius();
        return d<=r*.54f?0:d<=r*.79f?1:d<=r+8?2:-1;
    }
// Üç sayısal etiket halkanın altında sabit satırlarda yer alır.
    void resized() override { for(int i=0;i<3;++i) readouts[(size_t)i].setBounds(8,226+i*25,getWidth()-16,24); }
// Dış turuncu, orta kırık beyaz ve yivli iç kapak arkadan öne çizilir; hover hedefi vurgulanır.
    void paint(juce::Graphics& g) override {
        const auto c=centre(); const float r=radius();
        g.setColour(juce::Colour(0xffc3cbc7)); g.setFont(juce::FontOptions(12)); g.drawText(title,0,0,getWidth(),22,juce::Justification::centred);
        g.setColour(juce::Colour(0xff525b5e));
        for(int j=0;j<=20;++j) { const float a=(-225+j*270.0f/20)*juce::MathConstants<float>::pi/180; g.drawLine(c.x+std::cos(a)*(r+5),c.y+std::sin(a)*(r+5),c.x+std::cos(a)*(r+10),c.y+std::sin(a)*(r+10),1.2f); }
        auto disk=[&](float rad,juce::Colour colour) { g.setColour(colour); g.fillEllipse(c.x-rad,c.y-rad,2*rad,2*rad); };
        disk(r,juce::Colour(0xff101415)); disk(r-4,orange()); pointer(g,r-4,position(2),juce::Colour(0xff101415),3.5f);
        disk(r*.8f,juce::Colour(0xff101415)); disk(r*.75f,juce::Colour(0xffe5e7df)); pointer(g,r*.75f,position(1),juce::Colour(0xff292f31),3);
        disk(r*.55f,juce::Colour(0xff101415));
        juce::Path cap; const float rotation=(-225+270*position(0))*juce::MathConstants<float>::pi/180;
        for(int j=0;j<=192;++j) { const float a=j*juce::MathConstants<float>::twoPi/192,rad=r*(.48f+.022f*std::cos(12*(a-rotation))); const auto p=c+juce::Point<float>(std::cos(a)*rad,std::sin(a)*rad); if(j==0) cap.startNewSubPath(p); else cap.lineTo(p); }
        cap.closeSubPath(); g.setColour(juce::Colour(0xff292f31)); g.fillPath(cap); g.setColour(juce::Colour(0xff3e474a)); g.strokePath(cap,juce::PathStrokeType(1.2f)); pointer(g,r*.48f,position(0),orange(),3);
        if(hovered>=0) { g.setColour(juce::Colour(0xffe5e7df).withAlpha(.5f)); const float rad=hovered==0?r*.55f:hovered==1?r*.8f:r; g.drawEllipse(c.x-rad,c.y-rad,rad*2,rad*2,1); }
    }
// Fare üzerinde gezinirken hedef halkayı aydınlat; ses parametresini değiştirmez.
    void mouseMove(const juce::MouseEvent& e) override { hovered=partAt(e.position); repaint(); }
// Fare çıkınca hover vurgusunu temizler.
    void mouseExit(const juce::MouseEvent&) override { hovered=-1; repaint(); }
// Ctrl/Alt tıklama sıfırlar; normal tıklama host gesture ve sürükleme başlangıcını kaydeder.
    void mouseDown(const juce::MouseEvent& e) override { const int part=partAt(e.position); if(part<0) return; if(e.mods.isCtrlDown()||e.mods.isAltDown()) { resetPart(part); return; } dragging=part; hovered=part; startValue=position(part); attachments[(size_t)part]->beginGesture(); }
// Yukarı sürükleme değeri artırır; Shift hassasiyeti onda bire düşürür.
    void mouseDrag(const juce::MouseEvent& e) override { if(dragging>=0) attachments[(size_t)dragging]->setValueAsPartOfGesture(fromPosition(dragging,startValue-e.getDistanceFromDragStartY()*(e.mods.isShiftDown()?.0005f:.005f))); }
// Sürükleme bitince host otomasyon gesture’ını kapatır.
    void mouseUp(const juce::MouseEvent&) override { if(dragging>=0) attachments[(size_t)dragging]->endGesture(); dragging=-1; }
// Halkanın çift tıklaması varsayılanı yükler; etiketin çift tıklaması ayrı olarak sayısal giriş açar.
    void mouseDoubleClick(const juce::MouseEvent& e) override { resetPart(partAt(e.position)); }
// Tekerlek yönünü ve ters kaydırma tercihini dikkate alır; yalnız seçili halkayı tek gesture ile değiştirir.
    void mouseWheelMove(const juce::MouseEvent& e,const juce::MouseWheelDetails& wheel) override { const int part=partAt(e.position); if(part<0) return; const float delta=wheel.deltaY!=0?wheel.deltaY:wheel.deltaX; attachments[(size_t)part]->setValueAsCompleteGesture(fromPosition(part,position(part)+delta*(wheel.isReversed?-1:1)*(e.mods.isShiftDown()?.02f:.2f))); }
};
}
