// Geliştirici: Fazıl Irak - Son düzenleme: 27.10.2026
// Dosyanın görevi: Kompakt İngilizce ürün arayüzü, kontrol bağlantıları ve kullanıcı etkileşimi.
// Copyright (c) 2026 Fazıl Irak - Özgün F-Kick uygulama kodu.
// SPDX-License-Identifier: AGPL-3.0-or-later
// Fazıl Irak · 27.10.2026 · İngilizce ürün metni, gömülü font/lisanslar, Türkçe bakım yorumları.
#include "PluginEditor.h"
#include <BinaryData.h>

namespace {
const juce::Colour background(0xff181818),panel(0xff202124),line(0xff41454a),yellow(0xffffd42a),text(0xffe9eaec),muted(0xff979fa8);
void label(juce::Graphics& g,const juce::String& t,juce::Rectangle<int> r,float size=12,juce::Colour colour=muted,juce::Justification align=juce::Justification::centredLeft) {
    g.setFont(juce::FontOptions(size));g.setColour(colour);g.drawText(t,r,align,true);
}

}
FKickTheme::FKickTheme() {
    typeface=juce::Typeface::createSystemTypefaceFor(fkick_data::UbuntuMedium_ttf,fkick_data::UbuntuMedium_ttfSize);
    setColour(juce::TextButton::textColourOffId,text);setColour(juce::TextButton::textColourOnId,yellow);
    setColour(juce::ComboBox::backgroundColourId,juce::Colour(0xff262a30));setColour(juce::ComboBox::outlineColourId,line);
    setColour(juce::ComboBox::textColourId,text);setColour(juce::PopupMenu::backgroundColourId,panel);
    setColour(juce::PopupMenu::textColourId,text);setColour(juce::PopupMenu::highlightedBackgroundColourId,juce::Colour(0xff514923));
    setColour(juce::Slider::textBoxTextColourId,text);setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxBackgroundColourId,juce::Colours::transparentBlack);
    setColour(juce::TooltipWindow::backgroundColourId,juce::Colour(0xff303238));setColour(juce::TooltipWindow::textColourId,text);
}
void FKickTheme::drawButtonBackground(juce::Graphics& g,juce::Button& b,const juce::Colour&,bool hover,bool down) {
    // Küçük yazı seçimleri: kalıcı buton kasası yok; yalnız hover ve aktif alt çizgi.
    if(hover||down){g.setColour(text.withAlpha(.055f));g.fillRect(b.getLocalBounds());}
    if(b.getToggleState()&&b.isEnabled()){g.setColour(yellow);g.fillRect(3,b.getHeight()-2,b.getWidth()-6,1);}
}
void FKickTheme::drawComboBox(juce::Graphics& g,int w,int h,bool,int,int,int,int,juce::ComboBox&) {
    g.setColour(juce::Colour(0xff222428));g.fillRect(0,0,w,h);
    juce::Path arrow;arrow.startNewSubPath(float(w-13),h*.43f);arrow.lineTo(float(w-9),h*.61f);arrow.lineTo(float(w-5),h*.43f);
    g.setColour(muted);g.strokePath(arrow,juce::PathStrokeType(1));
}
FKickSidechainGraph::FKickSidechainGraph(FKickProcessor& p):processor(p),shift(*p.state.getParameter("scShift"),[this](float){repaint();},&p.undo){setComponentID("sidechain-graph");setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);shift.sendInitialUpdate();}
FKickSidechainGraph::~FKickSidechainGraph(){if(dragging)shift.endGesture();}
void FKickSidechainGraph::paint(juce::Graphics& g){
    g.fillAll(juce::Colour(0xff141516));auto r=getLocalBounds().toFloat().reduced(18,20);r.removeFromTop(5);r.removeFromBottom(3);
    label(g,"FX ONLY  /  1/4 NOTE",{18,1,280,18},10,yellow);label(g,"Drag left / right   |   Double-click: reset shift",{315,1,getWidth()-330,18},10,muted,juce::Justification::centredRight);
    g.setColour(line.withAlpha(.5f));g.drawRect(r,1);for(int i=1;i<4;++i)g.drawVerticalLine(juce::roundToInt(r.getX()+r.getWidth()*i/4),r.getY(),r.getBottom());
    const auto s=processor.settings().fx;juce::Path curve,area;area.startNewSubPath(r.getX(),r.getBottom());
    for(int i=0;i<=400;++i){const float gain=fkick::FXShaper::curve(s.scShape,i/400.,s.scShift);auto point=juce::Point<float>{r.getX()+r.getWidth()*i/400,r.getBottom()-gain*r.getHeight()};if(i==0)curve.startNewSubPath(point);else curve.lineTo(point);area.lineTo(point);}
    area.lineTo(r.getRight(),r.getBottom());area.closeSubPath();g.setColour(yellow.withAlpha(.08f));g.fillPath(area);g.setColour(yellow);g.strokePath(curve,juce::PathStrokeType(1.7f));
    g.setColour(text.withAlpha(.5f));const float x=r.getX()+r.getWidth()*processor.scPhase.load();g.drawLine(x,r.getY(),x,r.getBottom(),1);
    label(g,s.scMix==0?"MIX 0% / BYPASS":"SIDECHAIN MIX "+juce::String(s.scMix*100,0)+"%",{18,getHeight()-18,250,17},9);
}
void FKickSidechainGraph::mouseDown(const juce::MouseEvent& e){if(e.mods.isPopupMenu())return;if(e.mods.isAltDown()){shift.setValueAsCompleteGesture(0);return;}processor.undo.beginNewTransaction("Sidechain shift");startShift=processor.value("scShift");startX=e.position.x;shift.beginGesture();dragging=true;}
void FKickSidechainGraph::mouseDrag(const juce::MouseEvent& e){if(dragging)shift.setValueAsPartOfGesture(juce::jlimit(-.5f,.5f,startShift+(e.position.x-startX)/juce::jmax(1,getWidth()-36)));}
void FKickSidechainGraph::mouseUp(const juce::MouseEvent&){if(dragging){shift.endGesture();dragging=false;}}
void FKickSidechainGraph::mouseDoubleClick(const juce::MouseEvent&){if(dragging){shift.endGesture();dragging=false;}processor.undo.beginNewTransaction("Reset sidechain shift");shift.setValueAsCompleteGesture(0);}
FKickLogo::FKickLogo(){setComponentID("brand-logo");setName("F-Kick - About");setWantsKeyboardFocus(true);setMouseCursor(juce::MouseCursor::PointingHandCursor);setTooltip("F-Kick / About and offline licences");}
void FKickLogo::paint(juce::Graphics& g){
    // Fazıl Irak: Kompakt başlıkta ürün adı ve geliştirici / ürün türü aynı sade logo alanındadır.
    label(g,"F-Kick",{0,0,getWidth(),22},21,yellow);
    g.setColour(muted);g.setFont(juce::FontOptions(7.5f));
    g.drawFittedText(juce::String::fromUTF8("FAZIL IRAK / KICK & SAMPLER"),0,22,getWidth(),10,juce::Justification::centredLeft,1,.8f);
    if(hasKeyboardFocus(false)){g.setColour(yellow.withAlpha(.4f));g.drawHorizontalLine(getHeight()-1,0,float(getWidth()-1));}
}
void FKickLogo::mouseUp(const juce::MouseEvent& e){if(!e.mouseWasDraggedSinceMouseDown()&&getLocalBounds().contains(e.getPosition())&&onClick)onClick();}
bool FKickLogo::keyPressed(const juce::KeyPress& k){if(k==juce::KeyPress::returnKey||k==juce::KeyPress::spaceKey){if(onClick)onClick();return true;}return false;}

FKickValue::FKickValue(FKickProcessor& p,const char* parameterID,const char* name,const char* help)
    :processor(p),parameter(*p.state.getParameter(parameterID)),id(parameterID),caption(name),
     attachment(parameter,[this](float v){plain=v;refresh();},&p.undo) {
    setComponentID(id);setName(name);setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
    setTooltip(juce::String(help)+" Drag / scroll to adjust. Shift: fine. Double-click: type. Alt-click: reset.");
    number.setComponentID(id+"-value");number.setEditable(false,true,false);number.setJustificationType(juce::Justification::centredLeft);
    number.setFont(juce::FontOptions(12.f));number.setBorderSize(juce::BorderSize<int>(0));
    number.setColour(juce::Label::backgroundColourId,juce::Colours::transparentBlack);number.setColour(juce::Label::outlineColourId,juce::Colours::transparentBlack);
    number.setColour(juce::Label::textColourId,text);number.setColour(juce::Label::textWhenEditingColourId,text);number.setColour(juce::Label::backgroundWhenEditingColourId,panel);
    number.setTooltip(getTooltip());number.setMouseCursor(juce::MouseCursor::UpDownResizeCursor);number.addMouseListener(this,false);
    number.onTextChange=[this]{float v=parsed(number.getText());if(std::isfinite(v)){if(id=="start")v=juce::jmin(v,processor.value("end")-.001f);if(id=="end")v=juce::jmax(v,processor.value("start")+.001f);processor.undo.beginNewTransaction("Type "+caption);attachment.setValueAsCompleteGesture(parameter.convertFrom0to1(parameter.convertTo0to1(v)));}refresh();};
    number.onEditorShow=[this]{if(dragging){attachment.endGesture();dragging=false;}};
    addAndMakeVisible(number);attachment.sendInitialUpdate();
}
FKickValue::~FKickValue(){number.removeMouseListener(this);if(dragging)attachment.endGesture();}
juce::String FKickValue::formatted(float v) const {
    if(id.startsWith("lfo_")||id.startsWith("ad_")||id=="rumbleMix"||id=="echoMix"||id=="duck"||id=="echoFeedback")return juce::String(v*100,0)+"%";
    if(id=="rumbleDecay")return juce::String(v,1)+" s";
    if(id=="adCurve")return juce::String(v,2);
    if(id=="tempo")return juce::String(processor.hostTempo.load()>0?processor.hostTempo.load():v,0)+" BPM";
    if(id=="lfoHz")return juce::String(v,2)+" Hz";
    if(id=="rumbleHP"||id=="rumbleLP"||id=="echoLP")return v>=1000?juce::String(v/1000,1)+"k":juce::String(v,0);
    if(id=="rumbleDrive"||id=="rumbleGain")return juce::String(v,0)+" dB";
    if(id=="lfoPhase")return juce::String(v,0)+juce::String::fromUTF8("°");
    if(id=="sampleGain"||id=="bodyGain"||id=="output")return v<=-59.9f?juce::String("Off"):juce::String(v,1)+" dB";
    if(id=="sampleTune"||id=="sweep")return juce::String(v,0)+" st";
    if(id=="samplePan")return std::abs(v)<.005f?juce::String("C"):juce::String(v<0?"L ":"R ")+juce::String(juce::roundToInt(std::abs(v)*100));
    if(id=="room"||id=="scMix"||id=="punch"||id=="harmonics"||id=="sustain"||id=="start"||id=="end")return juce::String(v*100,id=="start"||id=="end"?1:0)+"%";
    if(id=="root")return juce::String(v,1)+" Hz";
    if((id=="hp"||id=="fxHP")&&v<1)return "Off";
    if(id=="fxLP"&&v>=19999)return "Off";
    if(id=="scShift")return (v>0?"+":"")+juce::String(v*100,0)+"%";
    if(id=="cutoff"||id=="hp"||id=="fxHP"||id=="fxLP")return v>=1000?juce::String(v/1000,1)+"k":juce::String(v,0);
    if(id=="phase")return juce::String(v,0)+juce::String::fromUTF8("°");
    if(id=="resonance"||id=="bassQ")return juce::String(v,2);
    if(id=="bassLift")return (v>0?"+":"")+juce::String(v,1)+" dB";
    if(id=="drive")return juce::String(v,0)+" dB";
    return juce::String(v,0)+" ms";
}
float FKickValue::parsed(const juce::String& input) const {
    auto t=input.trim().replaceCharacter(',', '.');
    if(t.equalsIgnoreCase("Off"))return id=="fxLP"?20000:id=="hp"||id=="fxHP"?0:-60;
    if(id=="samplePan"){if(t.equalsIgnoreCase("C"))return 0;const float v=t.retainCharacters("0123456789.-").getFloatValue();return t.startsWithIgnoreCase("L")?-v/100:t.startsWithIgnoreCase("R")?v/100:v;}
    const float value=t.getFloatValue();if(id=="scShift")return value/100;
    if(id=="room"||id=="scMix"||id=="punch"||id=="harmonics"||id=="sustain"||id=="start"||id=="end"||id.startsWith("lfo_")||id.startsWith("ad_")||id=="rumbleMix"||id=="echoMix"||id=="duck"||id=="echoFeedback")return value/100;
    return (id=="cutoff"||id=="hp"||id=="fxHP"||id=="fxLP"||id=="rumbleHP"||id=="rumbleLP"||id=="echoLP")&&t.containsIgnoreCase("k")?value*1000:value;
}
void FKickValue::refresh(){if(!number.isBeingEdited())number.setText(formatted(plain),juce::dontSendNotification);repaint();}
void FKickValue::paint(juce::Graphics& g){label(g,caption,{0,0,inlineWidth>0?inlineWidth:getWidth(),inlineWidth>0?getHeight():13},10,isEnabled()?muted:muted.withAlpha(.35f));}
void FKickValue::resized(){if(inlineWidth>0)number.setBounds(inlineWidth,0,getWidth()-inlineWidth,getHeight());else number.setBounds(0,14,getWidth(),21);}
void FKickValue::mouseDown(const juce::MouseEvent& e){
    if(e.mods.isPopupMenu())return;
    if(!isEnabled()||e.getNumberOfClicks()>1||number.isBeingEdited())return;
    processor.undo.beginNewTransaction("Adjust "+caption);
    if(e.mods.isAltDown()){attachment.setValueAsCompleteGesture(parameter.convertFrom0to1(parameter.getDefaultValue()));return;}
    dragging=true;dragValue=parameter.getValue();dragY=e.getScreenY();attachment.beginGesture();
}
void FKickValue::mouseDrag(const juce::MouseEvent& e){
    if(!dragging)return;const int y=e.getScreenY();dragValue=juce::jlimit(0.f,1.f,dragValue+(dragY-y)/(e.mods.isShiftDown()?1800.f:240.f));dragY=y;
    float v=parameter.convertFrom0to1(dragValue);
    if(id=="start")v=juce::jmin(v,processor.value("end")-.001f);
    if(id=="end")v=juce::jmax(v,processor.value("start")+.001f);
    attachment.setValueAsPartOfGesture(v);
}
void FKickValue::mouseUp(const juce::MouseEvent&){if(dragging){attachment.endGesture();dragging=false;}}
void FKickValue::mouseWheelMove(const juce::MouseEvent& e,const juce::MouseWheelDetails& w){
    if(!isEnabled()||number.isBeingEdited()||dragging||w.deltaY==0)return;processor.undo.beginNewTransaction("Adjust "+caption);
    float v=parameter.convertFrom0to1(juce::jlimit(0.f,1.f,parameter.getValue()+(w.deltaY>0?1:-1)*(e.mods.isShiftDown()?.001f:.01f)));
    if(id=="start")v=juce::jmin(v,processor.value("end")-.001f);if(id=="end")v=juce::jmax(v,processor.value("start")+.001f);attachment.setValueAsCompleteGesture(v);
}
juce::Rectangle<float> FKickGraph::plot() const {return {30.f,42.f,float(getWidth()-60),float(getHeight()-62)};}
juce::Point<float> FKickGraph::point(const fkick::Curve& c,int i) const {auto r=plot();return {r.getX()+c.x[(size_t)i]*r.getWidth(),r.getBottom()-c.y[(size_t)i]*r.getHeight()};}
int FKickGraph::hit(juce::Point<float> p) const {auto c=processor.curve(tab==1?1:0);for(int i=0;i<c.count;++i)if(point(c,i).getDistanceFrom(p)<11)return i;return -1;}
void FKickGraph::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff141516));auto r=plot();const auto s=processor.settings();const auto view=processor.sampleView();
    g.setColour(line.withAlpha(.45f));
    for(int i=0;i<=8;++i){const float x=r.getX()+r.getWidth()*i/8;g.drawVerticalLine(int(x),r.getY(),r.getBottom());}
    for(int i=0;i<=4;++i){const float y=r.getY()+r.getHeight()*i/4;g.drawHorizontalLine(int(y),r.getX(),r.getRight());}
    label(g,juce::String(int((tab==0&&view.loaded?view.duration:s.length)*1000))+" ms",{getWidth()-92,getHeight()-17,70,15},10);
    label(g,"0",{int(r.getX()),getHeight()-17,24,15},10);
    if(view.loaded) {
        g.setColour(tab==0?yellow.withAlpha(.65f):juce::Colour(0xff697079).withAlpha(.22f));
        for(int i=0;i<512;++i){const float x=r.getX()+i/511.f*r.getWidth(),h=juce::jmin(1.f,view.peaks[(size_t)i])*r.getHeight()*.45f;g.drawVerticalLine(int(x),r.getCentreY()-h,r.getCentreY()+h);}
    }
    if(tab==0) {
        if(!view.loaded) {label(g,"DRAG AUDIO HERE",r.toNearestInt().withHeight(38).withY(int(r.getCentreY())-25),20,text,juce::Justification::centred);label(g,"WAV / AIFF / FLAC  -  up to 30 seconds",{int(r.getX()),int(r.getCentreY())+8,int(r.getWidth()),20},12,muted,juce::Justification::centred);}
        else {
            // Çalınmayacak kısımları hafif karart; dalga şekli görünür, işaretçiler parlak kalır.
            const float startX=r.getX()+s.start*r.getWidth(),endX=r.getX()+s.end*r.getWidth();
            g.setColour(juce::Colours::black.withAlpha(.42f));
            g.fillRect(juce::Rectangle<float>(r.getX(),r.getY(),startX-r.getX(),r.getHeight()));
            g.fillRect(juce::Rectangle<float>(endX,r.getY(),r.getRight()-endX,r.getHeight()));
            for(int i=0;i<2;++i){float x=r.getX()+(i==0?s.start:s.end)*r.getWidth();g.setColour(yellow);g.drawVerticalLine(int(x),r.getY(),r.getBottom());g.fillRect(x-3,r.getY(),6.f,8.f);}
            label(g,"START "+juce::String(s.start*view.duration,3)+" s",{int(r.getX())+7,24,170,14},10,yellow);
            label(g,"END "+juce::String(s.end*view.duration,3)+" s",{getWidth()-220,24,180,14},10,yellow,juce::Justification::centredRight);
        }
        return;
    }
    const auto c=processor.curve(tab==1?1:0);
    juce::Path curve;for(int i=0;i<=640;++i){float t=i/640.f;float x=r.getX()+t*r.getWidth(),y=r.getBottom()-c.at(t)*r.getHeight();if(i==0)curve.startNewSubPath(x,y);else curve.lineTo(x,y);}
    auto fill=curve;fill.lineTo(r.getRight(),r.getBottom());fill.lineTo(r.getX(),r.getBottom());fill.closeSubPath();g.setColour(yellow.withAlpha(.06f));g.fillPath(fill);
    g.setColour(text);g.strokePath(curve,juce::PathStrokeType(1.8f));
    for(int i=0;i<c.count;++i){auto p=point(c,i);g.setColour(i==selected?yellow:text);g.fillEllipse(p.x-4,p.y-4,8,8);g.setColour(background);g.fillEllipse(p.x-1.5f,p.y-1.5f,3,3);}
    label(g,tab==1?"HIGH":"1.0",{1,int(r.getY())-5,34,15},9);
    label(g,tab==1?juce::String(int(s.root))+" Hz":"0.0",{1,int(r.getBottom())-9,34,15},9);
    if(selected>=0&&selected<c.count){const float y=c.y[(size_t)selected];juce::String info=juce::String(c.x[(size_t)selected]*s.length*1000,1)+" ms   /   ";info+=tab==1?juce::String(s.root*std::exp2(s.sweep*y/12),1)+" Hz":juce::String(y*100,1)+" %";label(g,info,{110,getHeight()-17,430,15},10,yellow);}
}
void FKickGraph::mouseDown(const juce::MouseEvent& e) {
    processor.undo.beginNewTransaction(tab==0?"Trim sample":"Edit envelope");dragOrigin=e.position;
    if(tab==0){const auto r=plot();const float t=juce::jlimit(0.f,1.f,(e.position.x-r.getX())/r.getWidth());trim=std::abs(t-processor.value("start"))<std::abs(t-processor.value("end"))?0:1;return;}
    selected=hit(e.position);auto c=processor.curve(tab==1?1:0);
    if(e.mods.isRightButtonDown()&&selected>0&&selected<c.count-1){for(int i=selected;i<c.count-1;++i){c.x[(size_t)i]=c.x[(size_t)i+1];c.y[(size_t)i]=c.y[(size_t)i+1];c.bend[(size_t)i]=c.bend[(size_t)i+1];}--c.count;processor.setCurve(tab==1?1:0,c);selected=-1;}
    if(selected>=0)bendStart=c.bend[(size_t)selected];repaint();
}
void FKickGraph::mouseDrag(const juce::MouseEvent& e) {
    auto r=plot();const float t=juce::jlimit(0.f,1.f,(e.position.x-r.getX())/r.getWidth());
    if(tab==0){if(trim==0)processor.setParameter("start",juce::jmin(processor.value("end")-.001f,t));else if(trim==1)processor.setParameter("end",juce::jmax(processor.value("start")+.001f,t));repaint();return;}
    if(selected<0||e.mods.isRightButtonDown())return;auto c=processor.curve(tab==1?1:0);
    if(e.mods.isAltDown()) c.bend[(size_t)selected]=juce::jlimit(-1.f,1.f,bendStart+(e.position.y-dragOrigin.y)/100.f);
    else {
        c.y[(size_t)selected]=juce::jlimit(0.f,1.f,(r.getBottom()-e.position.y)/r.getHeight());
        if(selected>0&&selected<c.count-1)c.x[(size_t)selected]=juce::jlimit(c.x[(size_t)selected-1]+.001f,c.x[(size_t)selected+1]-.001f,t);
    }
    processor.setCurve(tab==1?1:0,c);repaint();
}
void FKickGraph::mouseUp(const juce::MouseEvent&) {trim=-1;processor.undo.beginNewTransaction();}
void FKickGraph::mouseDoubleClick(const juce::MouseEvent& e) {
    if(tab==0)return;auto c=processor.curve(tab==1?1:0);if(c.count>=fkick::maxPoints||hit(e.position)>=0)return;
    const auto r=plot();const float t=juce::jlimit(.002f,.998f,(e.position.x-r.getX())/r.getWidth());int index=1;while(index<c.count-1&&c.x[(size_t)index]<t)++index;
    if(t-c.x[(size_t)index-1]<.001f||c.x[(size_t)index]-t<.001f)return;
    for(int i=c.count;i>index;--i){c.x[(size_t)i]=c.x[(size_t)i-1];c.y[(size_t)i]=c.y[(size_t)i-1];c.bend[(size_t)i]=c.bend[(size_t)i-1];}
    ++c.count;c.x[(size_t)index]=t;c.y[(size_t)index]=juce::jlimit(0.f,1.f,(r.getBottom()-e.position.y)/r.getHeight());c.bend[(size_t)index]=0;
    processor.undo.beginNewTransaction("Add envelope point");processor.setCurve(tab==1?1:0,c);selected=index;repaint();
}

bool FKickSampleDropArea::isInterestedInFileDrag(const juce::StringArray& files) {
    return files.size()==1&&juce::File(files[0]).hasFileExtension("wav;aif;aiff;flac;ogg;mp3");
}
void FKickSampleDropArea::fileDragEnter(const juce::StringArray& files,int,int){hovering=isInterestedInFileDrag(files);repaint();}
void FKickSampleDropArea::fileDragMove(const juce::StringArray& files,int,int){const bool next=isInterestedInFileDrag(files);if(next!=hovering){hovering=next;repaint();}}
void FKickSampleDropArea::fileDragExit(const juce::StringArray&){hovering=false;repaint();}
void FKickSampleDropArea::filesDropped(const juce::StringArray& files,int,int){
    hovering=false;repaint();if(!isInterestedInFileDrag(files))return;
    const juce::File file(files[0]);if(file.existsAsFile()&&onSampleDropped)onSampleDropped(file);
}
void FKickSampleDropArea::paintOverChildren(juce::Graphics& g){
    if(!hovering)return;
    g.setColour(juce::Colours::black.withAlpha(.12f));g.fillRect(getLocalBounds());
    g.setColour(yellow.withAlpha(.85f));g.drawRect(getLocalBounds().reduced(1),1);
    auto r=juce::Rectangle<int>(getWidth()/2-95,getHeight()/2-12,190,24);
    g.setColour(juce::Colour(0xff202124));g.fillRect(r);label(g,"Drop to load sample",r,12,yellow,juce::Justification::centred);
}
FKickAbout::FKickAbout() {
    setComponentID("about-panel");setOpaque(true);addAndMakeVisible(close);close.onClick=[this]{setVisible(false);};addAndMakeVisible(documents);addAndMakeVisible(text);
    text.setMultiLine(true,true);text.setReadOnly(true);text.setCaretVisible(false);text.setScrollbarsShown(true);text.setFont(juce::FontOptions(12.f));
    text.setComponentID("license-text");
    text.setColour(juce::TextEditor::backgroundColourId,juce::Colour(0xff121314));text.setColour(juce::TextEditor::textColourId,::text);text.setColour(juce::TextEditor::outlineColourId,line);
    for(int i=0;i<fkick_data::namedResourceListSize;++i)if(!juce::String(fkick_data::originalFilenames[i]).endsWithIgnoreCase(".ttf"))documents.addItem(fkick_data::originalFilenames[i],i+1);
    documents.onChange=[this]{int size=0;auto* content=fkick_data::getNamedResource(fkick_data::namedResourceList[documents.getSelectedId()-1],size);text.setText(juce::String::fromUTF8(content,size),false);text.moveCaretToTop(false);};documents.setSelectedItemIndex(0,juce::sendNotificationSync);
}
void FKickAbout::paint(juce::Graphics& g) {
    g.fillAll(background);label(g,"F-Kick",{20,12,150,30},24,yellow);label(g,"SAMPLER / KICK SYNTH   -   v" FKICK_VERSION,{130,17,400,24},11);
    label(g,juce::String::fromUTF8("Design and development: Fazıl Irak  /  © 2026"),{20,49,getWidth()-40,22},14,::text);
    label(g,"Independent engine. Local processing. No account or network required.",{20,76,getWidth()-40,18},11);
    label(g,"AGPLv3+ original sources / JUCE AGPLv3 / Ubuntu Font Licence.",{20,97,getWidth()-40,18},11);
}
void FKickAbout::resized() {close.setBounds(getWidth()-78,14,58,24);documents.setBounds(20,125,getWidth()-40,22);text.setBounds(20,158,getWidth()-40,getHeight()-178);}
FKickEditor::FKickEditor(FKickProcessor& p):AudioProcessorEditor(p),processor(p),hpValue(p,"hp","HP","Independent high-pass cutoff. Off bypasses it."),bodyPunch(p,"punch","Punch","Body density and a short phase-locked knock. Off preserves the original oscillator."),tempoValue(p,"tempo","Tempo","Fallback tempo; DAW tempo takes precedence."),scMixValue(p,"scMix","Mix","0% bypass; 100% fully applies the curve to FX returns."),scShiftValue(p,"scShift","Shift","Percent of one beat; positive shifts the curve right."),roomMixValue(p,"room","Room Mix","Parallel room amount. 100% equals the previous Room On level."),fxHPValue(p,"fxHP","HP","High-pass on effect returns only. Off bypasses."),fxLPValue(p,"fxLP","LP","Low-pass on effect returns only. Off bypasses."),bassLiftValue(p,"bassLift","Bass Lift","300 Hz low shelf on the full output: kick, sample and FX. 0 dB bypasses."),bassQValue(p,"bassQ","Q","Bass Lift shelf transition, 0.1 to 2.0. Independent of the main filter Q."),graph(p),sidechainGraph(p) {
    setLookAndFeel(&theme);setResizable(false,false);
    addAndMakeVisible(logo);logo.onClick=[this]{about.setVisible(true);about.toFront(true);};
    for(auto* b:{&previousPreset,&nextPreset,&save,&load,&options,&openSample,&play,&exportButton,&panic,&kickMode,&samplerMode,&gate,&poly,&loop,&reverse,&tracking,&sampleTab,&pitchTab,&ampTab})addAndMakeVisible(*b);
    for(auto* b:{&soundPage,&fxPage,&modPage,&echoSync})addAndMakeVisible(*b);
    soundPage.setComponentID("page-sound");fxPage.setComponentID("page-fx");modPage.setButtonText("SC");modPage.setTooltip("Simple sidechain: four curves, shift and mix. FX returns only.");modPage.setComponentID("page-sidechain");
    soundPage.onClick=[this]{page=0;updateControls();};fxPage.onClick=[this]{page=1;updateControls();};modPage.onClick=[this]{page=2;updateControls();};
    addAndMakeVisible(presets);presets.setComponentID("preset-selector");for(int i=0;i<p.getNumPrograms();++i){if(i==0)presets.addSectionHeading("GENERAL / 64");if(i==64)presets.addSectionHeading("TECHNO / 48");if(i==112)presets.addSectionHeading("EXPERIMENTAL / 16");presets.addItem(juce::String(i+1).paddedLeft('0',3)+"  "+p.getProgramName(i),i+1);}
    presets.onChange=[this]{processor.setCurrentProgram(presets.getSelectedId()-1);updateControls();};
    shownProgram=p.getCurrentProgram();presets.setSelectedId(shownProgram+1,juce::dontSendNotification);
    previousPreset.setComponentID("preset-previous");nextPreset.setComponentID("preset-next");previousPreset.setTooltip("Previous factory preset");nextPreset.setTooltip("Next factory preset");
    previousPreset.onClick=[this]{stepPreset(-1);};nextPreset.onClick=[this]{stepPreset(1);};
    kickMode.setComponentID("mode-kick");samplerMode.setComponentID("mode-sampler");
    kickMode.onClick=[this]{processor.undo.beginNewTransaction("Kick mode");processor.setParameter("mode",0);updateControls();};
    samplerMode.onClick=[this]{processor.undo.beginNewTransaction("Sampler mode");processor.setParameter("mode",1);updateControls();};
    auto bind=[&](juce::TextButton& b,const char* id,const char* help){b.setComponentID(id);b.setClickingTogglesState(true);b.setTooltip(help);buttons.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.state,id,b));};
    bind(gate,"gate","Kick mode: release follows MIDI note-off. Sampler mode always follows note-off.");
    bind(poly,"poly","Up to 16 voices. Off: monophonic retrigger.");bind(loop,"loop","Sampler mode: repeat the trimmed sample while held.");
    bind(reverse,"reverse","Play the sample region backwards.");bind(tracking,"tracking","Track body tail pitch from MIDI notes; preserve the attack pitch. C3 / MIDI 60 is the reference.");
    bind(echoSync,"echoSync","Echo time follows tempo.");
    sampleTab.onClick=[this]{if(page>=2)page=0;graph.tab=0;updateControls();};pitchTab.onClick=[this]{if(page>=2)page=0;graph.tab=1;updateControls();};ampTab.onClick=[this]{if(page>=2)page=0;graph.tab=2;updateControls();};
    pitchTab.setToggleState(true,juce::dontSendNotification);
    sampleTab.setComponentID("tab-sample");pitchTab.setComponentID("tab-pitch");ampTab.setComponentID("tab-amp");
    openSample.setComponentID("sample-load");openSample.setTooltip("Click to load a sample, or drop an audio file onto this panel.");
    save.onClick=[this]{chooseFile(1);};load.onClick=[this]{chooseFile(2);};openSample.onClick=[this]{chooseFile(0);};exportButton.onClick=[this]{chooseFile(3);};options.onClick=[this]{showOptions();};
    play.setTriggeredOnMouseDown(true);play.onStateChange=[this]{if(play.isDown())processor.keyboard.noteOn(1,60,.87f);else processor.keyboard.noteOff(1,60,0);};
    play.setTooltip("Hold to audition at C3 / MIDI 60. Release stops gated sounds.");panic.onClick=[this]{processor.keyboard.allNotesOff(0);processor.requestPanic();};
    panic.setTooltip("Panic: stop all playing voices.");exportButton.setTooltip("Export a 48 kHz / 24-bit WAV.");save.setTooltip("Save preset with its sample.");load.setTooltip("Load F-Kick preset.");
    auto field=[&](const char* id,const char* name,const char* help){values.push_back(std::make_unique<FKickValue>(p,id,name,help));addAndMakeVisible(*values.back());};
    field("sampleGain","Gain","Sample level; Off mutes the sample.");field("sampleTune","Tune","Sample semitone tuning.");field("samplePan","Pan","Sample stereo balance.");
    field("start","Start","Sample region start, in percent.");field("end","End","Sample region end, in percent.");
    field("bodyGain","Level","Synthesised body level; Off mutes the body.");field("root","Root","Body tail frequency at MIDI 60.");field("sweep","Sweep","Pitch envelope depth in semitones.");field("phase","Phase","Body phase at note-on.");field("harmonics","Harmonics","Second and third harmonics.");
    field("length","Length","Kick duration; Sampler pitch-envelope duration and decay scale.");field("attack","Attack","Initial fade time.");field("release","Release","Fade after MIDI note-off.");field("sustain","Sustain","Held-note level in Sampler mode.");
    const int inlineWidths[]={27,27,24,30,25};for(int i=0;i<5;++i)values[(size_t)i]->setInlineCaption(inlineWidths[i]);
    field("cutoff","Cutoff","Low-pass cutoff in Hz. Enter 2.5k for 2500 Hz.");field("resonance","Q","Low-pass resonance.");field("drive","Drive","Saturation after the filter.");field("output","Output","Final output gain.");
    auto extra=[&](auto& list,const char* id,const char* name,const char* help){list.push_back(std::make_unique<FKickValue>(p,id,name,help));addAndMakeVisible(*list.back());};
    extra(fxValues,"rumbleMix","Amount","Rumble return level.");extra(fxValues,"rumbleDecay","Decay","Rumble comb decay in seconds.");extra(fxValues,"rumbleHP","HP Hz","Rumble high-pass.");extra(fxValues,"rumbleLP","LP Hz","Rumble low-pass.");extra(fxValues,"rumbleDrive","Drive","Rumble saturation.");extra(fxValues,"duck","Duck","Duck both effect returns at each kick.");extra(fxValues,"recover","Recover","Duck recovery time.");
    extra(fxValues,"echoMix","Amount","Echo return level.");extra(fxValues,"echoTime","Time","Echo free time in milliseconds.");extra(fxValues,"echoFeedback","Feedback","Echo feedback.");extra(fxValues,"echoLP","LP Hz","Echo feedback damping.");extra(fxValues,"rumbleGain","Gain","Rumble return gain, after tone and saturation; independent of dry kick.");
    auto choice=[&](juce::ComboBox& box,const char* id,const juce::StringArray& items){addAndMakeVisible(box);box.setComponentID(id);box.addItemList(items,1);choices.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.state,id,box));};
    choice(echoDivision,"echoDivision",{"1/16","1/8","1/8 D","1/4","1/4 D"});choice(echoSource,"echoSource",{"Kick","Rumble"});choice(driveType,"driveType",{"Soft","Clip","Fold"});
    const char* scNames[]={"Pump","Smooth","Late","Gate"};
    for(int i=0;i<4;++i){auto& b=scPresets[(size_t)i];addAndMakeVisible(b);b.setButtonText(scNames[i]);b.setComponentID("sc-preset-"+juce::String(i+1));b.onClick=[this,i]{processor.undo.beginNewTransaction("Sidechain curve");processor.setParameter("scShape",float(i));updateControls();};}
    addAndMakeVisible(scMixValue);addAndMakeVisible(scShiftValue);
    addAndMakeVisible(hpValue);addAndMakeVisible(bodyPunch);
    addAndMakeVisible(samplePanel);
    for(auto* b:{&openSample,&kickMode,&samplerMode,&gate,&poly,&loop,&reverse,&sampleTab,&pitchTab,&ampTab})samplePanel.addAndMakeVisible(*b);
    for(int i=0;i<5;++i)samplePanel.addAndMakeVisible(*values[(size_t)i]);
    samplePanel.addAndMakeVisible(graph);
    samplePanel.addAndMakeVisible(sidechainGraph);samplePanel.addAndMakeVisible(tempoValue);samplePanel.addAndMakeVisible(roomMixValue);samplePanel.addAndMakeVisible(fxHPValue);samplePanel.addAndMakeVisible(fxLPValue);samplePanel.addAndMakeVisible(bassLiftValue);samplePanel.addAndMakeVisible(bassQValue);
    // Sayısal sample satırı artık grafiğin içinde; dolgu katmanının üstünde kalsın.
    for(int i=0;i<5;++i)values[(size_t)i]->toFront(false);loop.toFront(false);reverse.toFront(false);
    samplePanel.onSampleDropped=[this](const juce::File& file){if(processor.value("sampleGain")<=-59.9f)processor.setParameter("sampleGain",-6);processor.loadSampleAsync(file);page=0;graph.tab=0;updateControls();};
    nativeDrop.accepts=[this](const juce::StringArray& files){return isInterestedInFileDrag(files);};
    nativeDrop.hover=[this](const juce::StringArray& files,juce::Point<int> p,bool yes){if(yes&&samplePanel.isInterestedInFileDrag(files)){const auto q=samplePanel.getLocalPoint(this,p);samplePanel.fileDragEnter(files,q.x,q.y);}else samplePanel.fileDragExit(files);};
    nativeDrop.drop=[this](const juce::StringArray& files,juce::Point<int> p){filesDropped(files,p.x,p.y);};
    addChildComponent(about);setSize(760,360);updateControls();startTimerHz(30);
}
FKickEditor::~FKickEditor() {stopTimer();processor.keyboard.allNotesOff(0);processor.requestPanic();setLookAndFeel(nullptr);}
void FKickEditor::resized() {
    samplePanel.setBounds(8,44,744,208);
    logo.setBounds(12,3,94,33);previousPreset.setBounds(209,8,23,23);presets.setBounds(236,8,242,23);nextPreset.setBounds(482,8,23,23);
    soundPage.setBounds(109,8,38,23);fxPage.setBounds(149,8,24,23);modPage.setBounds(175,8,31,23);
    save.setBounds(516,8,37,23);load.setBounds(559,8,37,23);exportButton.setBounds(603,8,40,23);play.setBounds(653,8,22,23);panic.setBounds(685,8,22,23);options.setBounds(722,8,30,23);
    kickMode.setBounds(12,77,57,24);samplerMode.setBounds(12,105,57,24);poly.setBounds(12,172,57,21);gate.setBounds(12,204,57,21);
    sampleTab.setBounds(82,47,50,21);pitchTab.setBounds(139,47,43,21);ampTab.setBounds(189,47,41,21);openSample.setBounds(286,47,356,21);
    graph.setBounds(76,72,570,178);
    sidechainGraph.setBounds(76,72,570,178);tempoValue.setBounds(660,209,81,38);
    values[0]->setBounds(86,78,87,20);values[1]->setBounds(176,78,73,20);values[2]->setBounds(251,78,55,20);values[3]->setBounds(311,78,91,20);values[4]->setBounds(407,78,91,20);
    loop.setBounds(514,78,45,20);reverse.setBounds(567,78,67,20);tracking.setBounds(232,263,70,17);
    for(int i=0;i<5;++i)values[(size_t)i+5]->setBounds(18+i*48,283,47,38);
    for(int i=0;i<4;++i)values[(size_t)i+10]->setBounds(326+i*51,283,50,38);
    bodyPunch.setBounds(258,283,47,38);
    hpValue.setBounds(548,283,43,38);values[14]->setBounds(596,283,37,38);values[15]->setBounds(635,283,29,38);values[16]->setBounds(669,283,37,38);values[17]->setBounds(710,283,39,38);
    for(int i=0;i<7;++i)fxValues[(size_t)i]->setBounds(18+i*41,283,40,38);
    fxValues[11]->setBounds(305,283,43,38);
    fxValues[7]->setBounds(370,283,48,38);fxValues[8]->setBounds(423,283,58,38);fxValues[9]->setBounds(486,283,54,38);fxValues[10]->setBounds(545,283,54,38);
    echoDivision.setBounds(423,299,58,21);echoSource.setBounds(605,299,62,21);driveType.setBounds(682,299,62,21);echoSync.setBounds(430,263,43,17);
    for(int i=0;i<4;++i)scPresets[(size_t)i].setBounds(18+i*88,290,83,26);
    scMixValue.setBounds(400,283,70,39);scShiftValue.setBounds(490,283,75,39);
    roomMixValue.setBounds(660,77,81,38);fxHPValue.setBounds(660,121,38,38);fxLPValue.setBounds(705,121,38,38);bassLiftValue.setBounds(660,165,51,38);bassQValue.setBounds(716,165,27,38);
    // Yerleşim tarifleri editör koordinatındadır; sample paneli çocuklarını yerel koordinata çevir.
    for(auto* child:samplePanel.getChildren())child->setTopLeftPosition(child->getPosition()-samplePanel.getPosition());
    about.setBounds(getLocalBounds());
}
void FKickEditor::paint(juce::Graphics& g) {
    g.fillAll(background);g.setColour(line.withAlpha(.65f));g.drawHorizontalLine(38,8,752);
    g.setColour(juce::Colour(0xff141516));g.fillRect(8,44,744,208);g.setColour(line.withAlpha(.5f));g.drawVerticalLine(74,44,252);g.drawVerticalLine(650,72,248);
    label(g,"MODE",{20,52,50,15},9);label(g,"VOICE",{20,155,50,14},9);label(g,"PLAY",{20,190,50,14},9);
    g.setColour(juce::Colour(0xff1c1d1f));g.fillRect(8,260,744,71);g.setColour(line.withAlpha(.6f));
    if(page==0){g.drawVerticalLine(309,269,324);g.drawVerticalLine(535,269,324);label(g,"BODY",{18,263,150,15},9,yellow);label(g,"ENVELOPE",{326,263,194,15},9,yellow);label(g,"FILTER / OUTPUT",{548,260,196,13},9,yellow);}
    else if(page==1){g.drawVerticalLine(355,269,324);label(g,"RUMBLE",{18,263,150,15},9,yellow);label(g,"ECHO",{370,263,58,15},9,yellow);label(g,"Source",{605,283,62,13},10);label(g,"Drive type",{682,283,62,13},10);label(g,"Time",{423,283,58,13},10);}
    else if(page==2){label(g,"SIDECHAIN CURVE",{18,263,340,15},9,yellow);g.drawVerticalLine(385,269,324);label(g,"FX AMOUNT / TIMING",{400,263,185,15},9,yellow);label(g,"1/4 note",{620,284,110,17},12,text);label(g,"FX returns only",{620,304,110,15},10);}

    const auto view=processor.sampleView();label(g,view.status.isNotEmpty()?view.status:page==2?"Drag curve left/right  |  Double-click: reset shift  |  Mix 0%: bypass":"Drag: adjust  |  Double-click: type  |  Room / FX HP / FX LP: effect returns only",{12,339,594,15},10);
    const float db=juce::Decibels::gainToDecibels(peak,-60.f);g.setColour(line);g.fillRect(615,345,64,2);g.setColour(yellow);g.fillRect(615,345,int(64*juce::jlimit(0.f,1.f,(db+60)/60)),2);
    label(g,juce::String(db,1)+" dB",{686,338,64,17},10,peak>.98f?yellow:muted,juce::Justification::centredRight);
}
void FKickEditor::stepPreset(int direction){processor.setCurrentProgram((processor.getCurrentProgram()+direction+processor.getNumPrograms())%processor.getNumPrograms());updateControls();}
void FKickEditor::updateControls() {
    const bool sampler=processor.value("mode")>.5f;
    if(sampler&&graph.tab==2)graph.tab=1;
    soundPage.setToggleState(page==0,juce::dontSendNotification);fxPage.setToggleState(page==1,juce::dontSendNotification);modPage.setToggleState(page==2,juce::dontSendNotification);
    for(size_t i=5;i<values.size();++i)values[i]->setVisible(page==0);tracking.setVisible(page==0);hpValue.setVisible(page==0);bodyPunch.setVisible(page==0);
    for(auto& v:fxValues)v->setVisible(page==1);for(auto* c:{&echoSource,&driveType})c->setVisible(page==1);echoSync.setVisible(page==1);echoDivision.setVisible(page==1&&processor.value("echoSync")>.5f);fxValues[8]->setVisible(page==1&&processor.value("echoSync")<.5f);
    graph.setVisible(page<2);sidechainGraph.setVisible(page==2);
    for(auto& b:scPresets)b.setVisible(page==2);for(int i=0;i<4;++i)scPresets[(size_t)i].setToggleState(juce::roundToInt(processor.value("scShape"))==i,juce::dontSendNotification);
    scMixValue.setVisible(page==2);scShiftValue.setVisible(page==2);roomMixValue.setVisible(true);fxHPValue.setVisible(true);fxLPValue.setVisible(true);
    for(int i=0;i<5;++i)values[(size_t)i]->setVisible(page<2);loop.setVisible(page<2);reverse.setVisible(page<2);
    tempoValue.setEnabled(processor.hostTempo.load()==0);tempoValue.refreshDisplay();
    kickMode.setToggleState(!sampler,juce::dontSendNotification);samplerMode.setToggleState(sampler,juce::dontSendNotification);
    sampleTab.setToggleState(page<2&&graph.tab==0,juce::dontSendNotification);pitchTab.setToggleState(page<2&&graph.tab==1,juce::dontSendNotification);ampTab.setToggleState(page<2&&graph.tab==2,juce::dontSendNotification);
    if(shownProgram!=processor.getCurrentProgram()){shownProgram=processor.getCurrentProgram();presets.setSelectedId(shownProgram+1,juce::dontSendNotification);}
    loop.setEnabled(sampler);values[13]->setEnabled(sampler);gate.setEnabled(!sampler);ampTab.setEnabled(!sampler);
    gate.setButtonText(sampler||gate.getToggleState()?"Gate":"Trigger");poly.setButtonText(poly.getToggleState()?"Poly":"Mono");
    const auto view=processor.sampleView();const juce::String name=view.loaded?view.name+"  /  "+juce::String(view.duration,2)+" s":"Drop / load sample";
    if(name!=shownSample){shownSample=name;openSample.setButtonText(name);}
    for(auto& v:values)if(v->isVisible())v->repaint();for(auto& v:fxValues)if(v->isVisible())v->repaint();sidechainGraph.repaint();graph.repaint();repaint();
}
void FKickEditor::timerCallback(){nativeDrop.attach(*this);peak=juce::jmax(processor.meter.load(),peak*.87f);updateControls();}
bool FKickEditor::isInterestedInFileDrag(const juce::StringArray& files) {return samplePanel.isInterestedInFileDrag(files)||(files.size()==1&&juce::File(files[0]).hasFileExtension("fkick"));}
void FKickEditor::filesDropped(const juce::StringArray& files,int,int) {if(samplePanel.isInterestedInFileDrag(files)){samplePanel.filesDropped(files,0,0);return;}if(!isInterestedInFileDrag(files))return;if(!processor.loadPreset(juce::File(files[0])))juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,"F-Kick","Invalid F-Kick preset.");}
void FKickEditor::chooseFile(int operation) {
    const bool saving=operation==1||operation==3;
    const auto name=operation==0?"Load sample":operation==1?"Save F-Kick preset":operation==2?"Load F-Kick preset":"Export C3 / velocity 110 / 48 kHz / 24-bit WAV";
    const auto ext=operation==0?"*.wav;*.aif;*.aiff;*.flac;*.ogg;*.mp3":operation==3?"*.wav":"*.fkick";
    chooser=std::make_unique<juce::FileChooser>(name,juce::File{},ext);
    const auto flags=saving?juce::FileBrowserComponent::saveMode|juce::FileBrowserComponent::canSelectFiles|juce::FileBrowserComponent::warnAboutOverwriting:juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles;
    chooser->launchAsync(flags,[safe=juce::Component::SafePointer<FKickEditor>(this),operation](const juce::FileChooser& fc){
        if(!safe)return;auto file=fc.getResult();if(file==juce::File{})return;
        if(operation==0){if(safe->processor.value("sampleGain")<=-59.9f)safe->processor.setParameter("sampleGain",-6);safe->processor.loadSampleAsync(file);safe->page=0;safe->graph.tab=0;safe->updateControls();}
        else if(operation==1){if(!safe->processor.savePreset(file.withFileExtension("fkick")))juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,"F-Kick","Could not save preset.");}
        else if(operation==2){if(!safe->processor.loadPreset(file))juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,"F-Kick","Invalid F-Kick preset.");}
        else safe->processor.exportWavAsync(file.withFileExtension("wav"));
    });
}
void FKickEditor::showOptions() {
    juce::PopupMenu menu;menu.addItem(1,"Clear sample");menu.addItem(2,"Reset current factory patch");menu.addSeparator();menu.addItem(3,"Undo",processor.undo.canUndo());menu.addItem(4,"Redo",processor.undo.canRedo());menu.addSeparator();menu.addItem(5,"About / licences");menu.addItem(6,"Sampler init");
    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(options),[safe=juce::Component::SafePointer<FKickEditor>(this)](int result){if(!safe)return;switch(result){case 1:safe->processor.clearSample();break;case 2:safe->processor.setCurrentProgram(safe->processor.getCurrentProgram());break;case 3:safe->processor.undo.undo();break;case 4:safe->processor.undo.redo();break;case 6:safe->processor.setCurrentProgram(0);safe->processor.setParameter("mode",1);safe->processor.setParameter("bodyGain",-60);safe->processor.setParameter("sampleGain",-6);safe->processor.setParameter("poly",1);safe->processor.setParameter("gate",1);safe->updateControls();break;case 5:safe->about.setVisible(true);safe->about.toFront(true);break;default:break;}});
}


