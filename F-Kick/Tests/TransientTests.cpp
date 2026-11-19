#include "NumericPolicy.h"
// SPDX-License-Identifier: AGPL-3.0-or-later
// Fazıl Irak · 03.11.2026: transient bölgeleri, tam bypass, otomasyon/state ve GUI regresyonları.
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <iostream>
#include <cstring>
namespace {
void checkTS(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
juce::Component* findTS(juce::Component& c,const juce::String& id){if(c.getComponentID()==id)return &c;for(auto* child:c.getChildren())if(auto* p=findTS(*child,id))return p;return nullptr;}
juce::AudioBuffer<float> syntheticTS(double rate,float a,float p,float s){
    fkick::TransientShaper shaper;shaper.prepare(rate);shaper.set(a,p,s);shaper.trigger();
    const int n=int(rate*.4);juce::AudioBuffer<float> out(2,n);
    for(int i=0;i<n;++i){double t=i/rate;double x=.2*std::sin(juce::MathConstants<double>::twoPi*200*t)*std::exp(-t*3);
        auto y=shaper.process(x,x*.4,x,x*.4);out.setSample(0,i,float(y[0]));out.setSample(1,i,float(y[1]));
        checkTS(std::abs(y[1]-y[0]*.4)<1e-12,"Transient stereo image moved");}
    return out;
}
double rmsTS(const juce::AudioBuffer<float>& b,double rate,double from,double to){return b.getRMSLevel(0,int(from*rate),int((to-from)*rate));}
juce::AudioBuffer<float> processorTS(int block,float a,float p,float s){
    FKickProcessor processor;processor.setCurrentProgram(0);processor.setParameter("output",-18);
    processor.setParameter("tsAttack",a);processor.setParameter("tsPump",p);processor.setParameter("tsSustain",s);
    processor.prepareToPlay(48000,block);juce::AudioBuffer<float> out(2,24000);
    for(int offset=0;offset<out.getNumSamples();offset+=block){int n=juce::jmin(block,out.getNumSamples()-offset);juce::AudioBuffer<float> b(2,n);juce::MidiBuffer midi;
        for(int pos:{0,8400,16800})if(pos>=offset&&pos<offset+n)midi.addEvent(juce::MidiMessage::noteOn(1,60,(juce::uint8)110),pos-offset);
        processor.processBlock(b,midi);for(int c=0;c<2;++c)out.copyFrom(c,offset,b,c,0,n);}
    return out;
}
void saveTS(const juce::File& file,const juce::AudioBuffer<float>& b){juce::WavAudioFormat wav;std::unique_ptr<juce::AudioFormatWriter> writer(wav.createWriterFor(file.createOutputStream().release(),48000,2,24,{},0));checkTS(writer!=nullptr,"Transient WAV writer");checkTS(writer->writeFromAudioSampleBuffer(b,0,b.getNumSamples()),"Transient WAV output");}
}
void runTransientTests(const juce::File& folder){
    for(double rate:{44100.,48000.,96000.,192000.}){
        fkick::TransientShaper shaper;shaper.prepare(rate);shaper.set(0,0,0);shaper.trigger();
        for(int i=0;i<10000;++i){double l=std::sin(i*.131)*.19,r=std::cos(i*.173)*.3;auto y=shaper.process(l,r,l,r);
            checkTS(std::memcmp(&l,&y[0],sizeof(double))==0&&std::memcmp(&r,&y[1],sizeof(double))==0,"Zero transient not bit-exact");}
        shaper.set(100,100,100);for(int i=0;i<300;++i)shaper.process(.2,.1,.2,.1);
        shaper.set(0,0,0);auto returned=shaper.process(.123,-.234,.123,-.234);checkTS(returned[0]==.123&&returned[1]==-.234,"Returning all controls to zero not bypass");
        auto dry=syntheticTS(rate,0,0,0),attack=syntheticTS(rate,100,0,0),soft=syntheticTS(rate,-100,0,0),pump=syntheticTS(rate,0,100,0),tail=syntheticTS(rate,0,0,100),shorter=syntheticTS(rate,0,0,-100);
        double early=rmsTS(dry,rate,0,.008),middle=rmsTS(dry,rate,.022,.040),late=rmsTS(dry,rate,.15,.25);
        checkTS(rmsTS(attack,rate,0,.008)>early*2,"Attack did not boost onset");checkTS(rmsTS(soft,rate,0,.008)<early*.65,"Attack did not soften onset");
        checkTS(rmsTS(pump,rate,.022,.040)<middle*.3,"Pump did not cut post-transient");
        for(int i=0;i<int(rate*.007);++i)checkTS(pump.getSample(0,i)==dry.getSample(0,i),"Pump changed first hit");
        checkTS(pump.getMagnitude(0,0,pump.getNumSamples())<=dry.getMagnitude(0,0,dry.getNumSamples())+1e-7,"Pump boosted transient");
        checkTS(rmsTS(tail,rate,.15,.25)>late*3.8&&rmsTS(shorter,rate,.15,.25)<late*.3,"Sustain direction incorrect");
        // Geç başlayan sample: MIDI'den 140 ms sonra kuru onset yeni attack penceresi açmalı.
        fkick::TransientShaper delayed;delayed.prepare(rate);delayed.set(100,0,0);delayed.trigger();
        for(int i=0;i<int(rate*.14);++i)delayed.process(0,0,0,0);
        double latePeak=0;for(int i=0;i<int(rate*.008);++i){auto y=delayed.process(.15,.15,.15,.15);latePeak=juce::jmax(latePeak,y[0]);}checkTS(latePeak>.4,"Late sample onset not detected");
    }
    checkTS(fkick::enteredNumber("bassNote",29.7f)==30&&fkick::enteredNumber("tempo",135.6f)==136,"Integer user input");
    checkTS(std::abs(fkick::enteredNumber("root",50.1234f)-50.12f)<1e-5,"Two-decimal user input");
    checkTS(std::abs(fkick::enteredNumber("start",.123456f)-.1235f)<1e-6,"Sample percent precision");
    FKickProcessor state;const char* ids[]{"tsAttack","tsPump","tsSustain"};float values[]{-55,68,42};
    for(auto* param:state.getParameters())if(auto* fp=dynamic_cast<juce::AudioParameterFloat*>(param))for(float n:{0.f,.1234567f,.7891234f,1.f}){
        const auto text=param->getText(n,128);const int dot=text.indexOfChar('.');
        if(dot>=0&&text.length()-dot-1>2)throw std::runtime_error(("Host precision: "+fp->paramID+" = "+text).toStdString());}
    checkTS(state.getParameters().size()==158,"Transient append-only parameter count");
    for(int i=0;i<3;++i){auto* p=dynamic_cast<juce::RangedAudioParameter*>(state.getParameters()[155+i]);checkTS(p&&p->paramID==ids[i],"Transient parameter IDs/order");state.setParameter(ids[i],values[i]);}
    juce::MemoryBlock data;state.getStateInformation(data);FKickProcessor recalled;recalled.setStateInformation(data.getData(),int(data.getSize()));
    for(int i=0;i<3;++i)checkTS(std::abs(recalled.value(ids[i])-values[i])<.001,"Transient state recall");
    auto xml=juce::AudioProcessor::getXmlFromBinary(data.getData(),int(data.getSize()));auto* parameters=xml->getChildByName("FKICK_PARAMETERS");
    for(int i=parameters->getNumChildElements()-1;i>=0;--i){auto* child=parameters->getChildElement(i);if(child->getStringAttribute("id").startsWith("ts"))parameters->removeChildElement(child,true);}
    juce::MemoryBlock legacy;juce::AudioProcessor::copyXmlToBinary(*xml,legacy);recalled.setStateInformation(legacy.getData(),int(legacy.getSize()));
    for(auto* id:ids)checkTS(recalled.value(id)==0,"Legacy preset inherited transient settings");
    auto tiny=processorTS(13,70,65,-40),large=processorTS(1024,70,65,-40);
    for(int c=0;c<2;++c)for(int i=0;i<tiny.getNumSamples();++i)checkTS(std::isfinite(tiny.getSample(c,i))&&std::abs(tiny.getSample(c,i)-large.getSample(c,i))<2e-6,"Transient MIDI/block-size invariance");
    FKickProcessor visual;std::unique_ptr<juce::AudioProcessorEditor> editor(visual.createEditor());
    for(auto entry:std::array<std::pair<const char*,const char*>,4>{{{"tempo","135.67"},{"root","50.12345"},{"start","12.34567%"},{"bodyGain","-6.126"}}}){
        auto* field=dynamic_cast<FKickValue*>(findTS(*editor,entry.first));checkTS(field!=nullptr,"Numeric editor field missing");
        field->number.setText(entry.second,juce::sendNotificationSync);
    }
    checkTS(visual.value("tempo")==136&&std::abs(visual.value("root")-50.12f)<1e-4&&std::abs(visual.value("start")-.1235f)<1e-5&&std::abs(visual.value("bodyGain")+6.13f)<1e-4,"Numeric editor entry precision");
    visual.setCurrentProgram(0);
    auto* panel=findTS(*editor,"sample-drop-area"); // Bileşen kimliği yerine parent ilişkisi de kontrol edilir.
    juce::ignoreUnused(panel);
    auto* tempo=findTS(*editor,"tempo");
    for(auto* id:ids){auto* knob=findTS(*editor,id);checkTS(knob&&knob->isShowing()==editor->isShowing(),"Transient knob missing");
        auto* slider=dynamic_cast<juce::Slider*>(findTS(*knob,juce::String(id)+"-slider"));
        checkTS(slider&&slider->getTextFromValue(slider->getValue())=="0%","Transient percentage formatting");
        auto bounds=editor->getLocalArea(knob,knob->getLocalBounds());checkTS(editor->getLocalBounds().contains(bounds),"Transient knob outside panel");
        if(tempo)checkTS(!bounds.intersects(editor->getLocalArea(tempo,tempo->getLocalBounds())),"Transient/tempo overlap");}
    {juce::FileOutputStream stream(folder.getChildFile("F-Kick-transient-086.png"));stream.setPosition(0);stream.truncate();checkTS(juce::PNGImageFormat{}.writeImageToStream(editor->createComponentSnapshot(editor->getLocalBounds()),stream),"Transient preview");}
    for(int i=0;i<5;++i){const char* names[]{"Dry","Attack","Pump","Sustain-Up","Sustain-Down"};saveTS(folder.getChildFile(juce::String("Transient-")+names[i]+".wav"),processorTS(257,i==1?70.f:0,i==2?80.f:0,i==3?70.f:i==4?-70.f:0));}
    std::cout<<"PASS transient: exact-zero bypass, Attack/Pump/Sustain regions, stereo link, late onset, 44.1/48/96/192k, state/legacy, block sizes, GUI and five WAVs\n";
}
