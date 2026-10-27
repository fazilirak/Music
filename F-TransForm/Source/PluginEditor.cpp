// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (c) 2026 Fazıl Irak
// Geliştirici: Fazıl Irak | Oluşturma / son düzenleme: 27.10.2026
// Dosyanın görevi: F-Saturator paletiyle donanım hissi veren üç kontrollü paneli çizmek ve bağlamak.
#include "PluginEditor.h"
#include "FTFonts.h"
namespace ft {
namespace {
const juce::Colour bg(0xff141819),panel(0xff1b2123),accent(0xffed6c46),ivory(0xffe5e7df),muted(0xff8d9799);
void text(juce::Graphics& g,juce::String s,float x,float y,float w,float h,float size,juce::Colour col=ivory,int align=juce::Justification::centred) {
 g.setColour(col);g.setFont(juce::FontOptions(size));g.drawText(s,juce::Rectangle<float>(x,y,w,h),align);
}
}
Editor::Theme::Theme() {
 // Gömülü URW Gothic Book/Demi, F serisinin tipografisini tüm bilgisayarlarda aynı tutar.
 book=juce::Typeface::createSystemTypefaceFor(ft_fonts::Book_otf,ft_fonts::Book_otfSize);
 demi=juce::Typeface::createSystemTypefaceFor(ft_fonts::Demi_otf,ft_fonts::Demi_otfSize);
 setColour(juce::Slider::textBoxTextColourId,ivory);
 setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);
 setColour(juce::Slider::textBoxBackgroundColourId,juce::Colours::transparentBlack);
 setColour(juce::TextButton::buttonColourId,panel);setColour(juce::TextButton::buttonOnColourId,accent);
 setColour(juce::TextButton::textColourOffId,ivory);setColour(juce::TextButton::textColourOnId,bg);
 setColour(juce::ComboBox::backgroundColourId,panel);setColour(juce::ComboBox::textColourId,ivory);
 setColour(juce::ComboBox::outlineColourId,muted);
}
void Editor::Theme::drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float p,float start,float end,juce::Slider& slider) {
 // Aynı çizici üç düğmeyi çizer. Büyük merkez knob sürekli yay, seçiciler ayrık çentik gösterir.
 auto centre=juce::Point<float>(x+w*.5f,y+h*.5f); float r=juce::jmin(w,h)*.44f;
 bool big=slider.getComponentID()=="intensity";
 for(int i=0;i<=(big?40:slider.getComponentID()=="technique"?techniqueCount-1:4);++i){
   int n=big?40:slider.getComponentID()=="technique"?techniqueCount-1:4;
   float a=start+(end-start)*i/n;
   g.setColour(i<=(int)(p*n+.01f)?accent:juce::Colour(0xff485254));
   g.drawLine(centre.x+std::sin(a)*(r+5),centre.y-std::cos(a)*(r+5),
              centre.x+std::sin(a)*(r+10),centre.y-std::cos(a)*(r+10),big?1.4f:2.f);
 }
 juce::Path arc;arc.addCentredArc(centre.x,centre.y,r-2,r-2,0,start,end,true);
 g.setColour(juce::Colour(0xff30383a));g.strokePath(arc,juce::PathStrokeType(big?7.f:3.f));
 juce::Path active;active.addCentredArc(centre.x,centre.y,r-2,r-2,0,start,start+p*(end-start),true);
 g.setColour(accent);g.strokePath(active,juce::PathStrokeType(big?7.f:3.f));
 g.setColour(juce::Colours::black.withAlpha(.45f));g.fillEllipse(centre.x-r+10,centre.y-r+15,2*r-20,2*r-20);
 g.setGradientFill(juce::ColourGradient(juce::Colour(0xff778183),centre.x-r,centre.y-r,
   juce::Colour(0xff30383b),centre.x+r,centre.y+r,false));
 g.fillEllipse(centre.x-r+10,centre.y-r+10,2*r-20,2*r-20);
 g.setColour(ivory.withAlpha(.7f));g.drawEllipse(centre.x-r+10,centre.y-r+10,2*r-20,2*r-20,1);
 g.setColour(juce::Colour(0xff202729));g.fillEllipse(centre.x-r+18,centre.y-r+18,2*r-36,2*r-36);
 // Geniş tutamak donanım seçici hissi verir; merkez knob uzun hareket göstergesi taşır.
 float a=start+p*(end-start);
 juce::Path grip;float length=r-(big?24:19);
 grip.addRoundedRectangle(big?-5.f:-9.f,-length,big?10.f:18.f,length+(big?12.f:25.f),4);
 g.setColour(juce::Colour(0xff101516));g.fillPath(grip,juce::AffineTransform::rotation(a).translated(centre.x,centre.y));
 g.setColour(accent);g.drawLine(centre.x+std::sin(a)*(r*.48f),centre.y-std::cos(a)*(r*.48f),
   centre.x+std::sin(a)*(r-24),centre.y-std::cos(a)*(r-24),big?4.f:3.f);
}
Editor::Editor(Processor& p):AudioProcessorEditor(p),processor(p) {
 // SliderAttachment iki yönlü bağ kurar: fare hareketi host'a, DAW otomasyonu arayüze gider.
 setLookAndFeel(&theme);
 addAndMakeVisible(logo);addChildComponent(about);
 logo.onActivate=[this]{about.setVisible(true);about.toFront(true);about.grabKeyboardFocus();};
 about.onClose=[this]{logo.grabKeyboardFocus();};
 for(auto* s:{&technique,&intensity,&character}) {
   addAndMakeVisible(s);s->setSliderStyle(juce::Slider::RotaryVerticalDrag);
   s->setRotaryParameters(juce::MathConstants<float>::pi*1.23f,juce::MathConstants<float>::pi*2.77f,true);
   s->setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);s->setMouseDragSensitivity(280);
   s->setWantsKeyboardFocus(true);
 }
 technique.setComponentID("technique");intensity.setComponentID("intensity");character.setComponentID("character");
 technique.setName("Technique selector"); intensity.setName("Intensity");character.setName("Soft to hard character");
 techniqueLink=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.state,"technique",technique);
 intensityLink=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.state,"intensity",intensity);
 characterLink=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.state,"character",character);
 technique.setDoubleClickReturnValue(true,0);intensity.setDoubleClickReturnValue(true,0);character.setDoubleClickReturnValue(true,2);
 technique.setTooltip("7 multi-effect transforms. Drag, scroll or click a name.");
 intensity.setTooltip("Automate this control to perform the transition. Double-click: dry (0%).");
 character.setTooltip("5 fixed positions: Silk, Soft, Balanced, Hard, Extreme.");
 addAndMakeVisible(bypass);bypass.setClickingTogglesState(true);
 bypassLink=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.state,"bypass",bypass);
 setSize(1120,650);startTimerHz(25);
}
// LookAndFeel üyesi silinmeden bileşenin referansı ayrılır; kapanışta sarkan işaretçi kalmaz.
Editor::~Editor(){stopTimer();setLookAndFeel(nullptr);}
void Editor::resized() {
 logo.setBounds(28,30,300,43);about.setBounds(getLocalBounds());
 technique.setBounds(133,260,166,166);intensity.setBounds(418,210,280,280);character.setBounds(849,260,166,166);
 bypass.setBounds(980,42,105,29);
}
// Etikete tıklama da knob sürükleme gibi bir host gesture üretir; otomasyon bunu kaydedebilir.
void Editor::select(int index){auto* p=processor.state.getParameter("technique");p->beginChangeGesture();p->setValueNotifyingHost(p->convertTo0to1((float)index));p->endChangeGesture();}
void Editor::mouseDown(const juce::MouseEvent& e){
 for(int i=0;i<techniqueCount;++i){bool left=i<leftTechniqueCount;
   if(juce::Rectangle<int>(left?35:288,(int)techniqueLabelY(i),left?100:109,24).contains(e.getPosition()))select(i);
 }
}
void Editor::paint(juce::Graphics& g) {
 const auto s=processor.settings();int index=juce::jlimit(0,techniqueCount-1,s.technique);
 auto& t=techniques[(size_t)index];g.fillAll(bg);
 text(g,"FAZIL IRAK  /  TRANSITION ENGINE",30,78,320,20,10,muted,juce::Justification::centredLeft);
 text(g,"01",28,130,32,20,11,accent);text(g,"TECHNIQUE",66,130,250,20,11,ivory,juce::Justification::centredLeft);
 text(g,"02   INTENSITY",425,130,265,20,11);text(g,"03   CHARACTER",800,130,270,20,11);
 g.setColour(accent);g.fillRect(28,116,1064,2);g.setColour(ivory.withAlpha(.4f));g.fillRect(28,121,1064,1);
 g.setColour(panel);g.fillRoundedRectangle(22,167,1076,352,8);
 g.setColour(juce::Colour(0xff343c3e));g.drawVerticalLine(407,188,495);g.drawVerticalLine(740,188,495);
 // Üstteki çizgi Intensity'ye bağlı dekoratif bir dönüşüm izidir; osiloskop ölçümü değildir.
 juce::Path trace;
 for(int i=0;i<430;++i){float x=450.f+i;float envelope=(.12f+.88f*i/430.f)*(.15f+.85f*s.intensity);
   float y=64+std::sin(i*.092f)*std::sin(i*.031f)*25*envelope;
   if(i==0)trace.startNewSubPath(x,y);else trace.lineTo(x,y);}
 g.setColour(accent.withAlpha(.75f));g.strokePath(trace,juce::PathStrokeType(1.4f));
 text(g,"SELECT A TRANSFORM",32,181,360,20,10,muted);
 for(int i=0;i<techniqueCount;++i) {
   bool left=i<leftTechniqueCount;float yy=techniqueLabelY(i);
   bool selected=i==index;g.setColour(selected?accent:juce::Colour(0xff536062));
   // Teknik adları çizgisiz gösterilir; yalnız knobun Theme içinde çizilen çevre çentikleri kalır.
   // Sağ sütun 12 px sola çekilir; metnin sağ kenarı ile seçim noktası arasında 9 px boşluk kalır.
   text(g,techniques[(size_t)i].name,left?35.f:288.f,yy,96,24,11,selected?accent:ivory,
     left?juce::Justification::centredLeft:juce::Justification::centredRight);
   if(selected) {
     // Noktayı satır kutusuna değil, çizilen harflerin gerçek dikey merkezine hizala.
     // Fontun üst/alt boşlukları veya kelimedeki aşağı uzanan harfler hizayı bozmasın.
     juce::GlyphArrangement letters;
     letters.addCurtailedLineOfText(g.getCurrentFont(),techniques[(size_t)i].name,0,0,96,true);
     letters.justifyGlyphs(0,letters.getNumGlyphs(),left?35.f:288.f,yy,96,24,
       left?juce::Justification::centredLeft:juce::Justification::centredRight);
     juce::Path outlines;letters.createPath(outlines);
     const float centreY=outlines.getBounds().getCentreY();
     g.fillEllipse(left?28.f:393.f,centreY-2.f,4.f,4.f);
   }
 }
 text(g,juce::String(index+1).paddedLeft('0',2),160,427,112,24,14,accent);
 text(g,t.group,147,457,140,18,9,muted);
 text(g,juce::String(juce::roundToInt(s.intensity*100))+"%",448,461,220,42,32,ivory);
 text(g,"DRY",428,435,55,20,9,muted);text(g,"FULL",643,435,55,20,9,muted);
 const float labelX[]{784,781,879,991,1000};const float labelY[]{419,239,204,239,419};
 for(int i=0;i<5;++i)text(g,juce::String(characters[i]).toUpperCase(),labelX[i],labelY[i],90,20,10,i==s.character?accent:ivory);
 text(g,"SOFT  /  HARD",816,470,235,20,9,muted);
 // Alt açıklama aktif tekniği anlatır; yeni bir kontrol eklemeden tetik eşiklerini görünür kılar.
 g.setColour(juce::Colour(0xff101516));g.fillRoundedRectangle(22,536,802,80,5);
 text(g,juce::String(index+1).paddedLeft('0',2)+"  /  "+juce::String(t.name).toUpperCase(),40,547,760,24,14,accent,juce::Justification::centredLeft);
 juce::String desc=t.description;
 text(g,desc,40,577,760,24,11,ivory,juce::Justification::centredLeft);
 text(g,juce::String(processor.tempo.load(),1)+" BPM",851,552,94,20,12,ivory);
 text(g,"HOST SYNC",851,577,94,20,9,muted);
 for(int c=0;c<2;++c){float peak=c?processor.outPeak.load():processor.inPeak.load();
   float norm=juce::jlimit(0.f,1.f,(juce::Decibels::gainToDecibels(peak,-60.f)+60)/60);
   float xx=970.f+c*65; text(g,c?"OUT":"IN",xx,540,48,17,9,muted);
   for(int j=0;j<12;++j){g.setColour(j<norm*12?(j>10?accent:ivory.withAlpha(.7f)):juce::Colour(0xff30383b));g.fillRect(xx+4,598-j*3.f,39.f,2.f);}
 }
 text(g,"7 MULTI-FX TRANSFORMS  /  ONE GESTURE",28,625,400,16,9,muted,juce::Justification::centredLeft);
 text(g,"F-TransForm " FT_VERSION,880,625,210,16,9,muted,juce::Justification::centredRight);
}
}
