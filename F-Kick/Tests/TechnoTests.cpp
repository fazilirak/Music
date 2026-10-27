// Geliştirici: Fazıl Irak - Son düzenleme: 27.10.2026
// Dosyanın görevi: Ses ve durum davranışını doğrulayan testler.
// Copyright (c) 2026 Fazıl Irak - Özgün F-Kick uygulama kodu.
// SPDX-License-Identifier: AGPL-3.0-or-later
// Fazıl Irak · 27.10.2026 · FX, modülasyon, eski state ve örnek ses doğrulaması.
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <iostream>
#include <stdexcept>
namespace {
void check(bool x,const char* why){if(!x)throw std::runtime_error(why);}
void set(FKickProcessor& p,const char* id,float v){p.setParameter(id,v);}
juce::AudioBuffer<float> render(FKickProcessor& p,int samples,int block=256){
    p.prepareToPlay(48000,block);juce::AudioBuffer<float> out(2,samples);out.clear();
    for(int n=0;n<samples;n+=block){int count=juce::jmin(block,samples-n);juce::AudioBuffer<float> a(2,count);juce::MidiBuffer midi;if(n==0)midi.addEvent(juce::MidiMessage::noteOn(1,60,(juce::uint8)110),0);p.processBlock(a,midi);for(int c=0;c<2;++c)out.copyFrom(c,n,a,c,0,count);}return out;
}
double energy(const juce::AudioBuffer<float>& a,int start,int n){double sum=0;for(int i=start;i<start+n;++i)sum+=a.getSample(0,i)*a.getSample(0,i);return std::sqrt(sum/n);}
float difference(const juce::AudioBuffer<float>& a,const juce::AudioBuffer<float>& b){float max=0;for(int c=0;c<2;++c)for(int i=0;i<a.getNumSamples();++i)max=juce::jmax(max,std::abs(a.getSample(c,i)-b.getSample(c,i)));return max;}
void write(const juce::File& file,juce::AudioBuffer<float>& audio){auto stream=file.createOutputStream();check(stream!=nullptr,"Demo file stream");stream->setPosition(0);stream->truncate();juce::WavAudioFormat fmt;std::unique_ptr<juce::AudioFormatWriter> writer(fmt.createWriterFor(stream.get(),48000,2,24,{},0));check(writer!=nullptr,"Demo WAV writer");stream.release();check(writer->writeFromAudioSampleBuffer(audio,0,audio.getNumSamples()),"Demo WAV write");}
}
void runTechnoTests(const juce::File& folder){
    FKickProcessor dry,hp;for(auto* p:{&dry,&hp}){set(*p,"mode",1);set(*p,"sustain",1);set(*p,"sweep",0);}set(hp,"hp",400);
    const auto full=render(dry,48000),cut=render(hp,48000);check(energy(cut,24000,24000)<energy(full,24000,24000)*.08,"Independent HP did not remove sub energy");
    std::cout<<"PASS independent HP attenuation\n";
    FKickProcessor plain,rumble,duck;for(auto* p:{&plain,&rumble,&duck})set(*p,"length",120);
    for(auto* p:{&rumble,&duck}){set(*p,"rumbleMix",.8f);set(*p,"rumbleDecay",1.2f);set(*p,"recover",300);}
    set(rumble,"duck",0);set(duck,"duck",.95f);auto a=render(plain,96000),b=render(rumble,96000),c=render(duck,96000);
    check(energy(b,24000,24000)>energy(a,24000,24000)*10+1e-5,"Rumble tail missing");
    for(int i=0;i<a.getNumSamples();++i){b.getWritePointer(0)[i]-=a.getSample(0,i);c.getWritePointer(0)[i]-=a.getSample(0,i);}
    check(energy(c,1500,3500)<energy(b,1500,3500)*.65,"Duck did not reduce the early wet return");
    FKickProcessor echo;set(echo,"length",60);set(echo,"echoMix",.8f);set(echo,"echoSync",0);set(echo,"echoTime",180);set(echo,"echoFeedback",0);auto e=render(echo,24000);
    check(energy(e,8700,2000)>.001,"Echo repeat missing at selected time");check(fkick::TechnoFX::echoSeconds(echo.settings().fx,135)>.179,"Free echo time wrong");
    auto synced=echo.settings().fx;synced.echoSync=true;synced.echoDivision=1;check(std::abs(fkick::TechnoFX::echoSeconds(synced,120)-.25)<1e-9,"Tempo synced echo wrong");
    std::cout<<"PASS rumble decay / wet duck / echo repeat and tempo sync\n";
    fkick::Motion motion;motion.prepare(48000);fkick::MotionSettings ms;ms.sync=false;ms.hz=2;motion.trigger(ms);for(int i=0;i<6000;++i)motion.tick(ms);check(motion.tick(ms).lfo>.999,"LFO rate wrong");motion.trigger(ms);check(std::abs(motion.tick(ms).lfo)<1e-5,"LFO retrigger failed");
    ms.delay=.01f;ms.attack=.02f;ms.decay=.03f;check(fkick::Motion::envelope(ms,.005)==0&&std::abs(fkick::Motion::envelope(ms,.02)-.5)<.001&&fkick::Motion::envelope(ms,.031)>.9&&fkick::Motion::envelope(ms,.07)==0,"AD delay / attack / decay wrong");
    ms.sync=true;ms.bpm=120;ms.division=1;check(fkick::Motion::lfoHz(ms)==4,"Synced LFO rate wrong");
    FKickProcessor fx1,fx2;fx1.setCurrentProgram(66);fx2.setCurrentProgram(66);auto f1=render(fx1,96000,127),f2=render(fx2,96000,512);check(difference(f1,f2)<2e-6,"FX/modulation block-size dependence");
    FKickProcessor still;still.setCurrentProgram(66);set(still,"lfo_rumbleLP",0);set(still,"lfo_rumbleMix",0);set(still,"ad_rumbleDrive",0);check(difference(f1,render(still,96000))>.001,"Modulation did not affect audio");
    for(double sr:{44100.,96000.,192000.})for(int kind=0;kind<3;++kind){FKickProcessor stress;stress.setCurrentProgram(65);set(stress,"driveType",float(kind));set(stress,"drive",24);set(stress,"echoFeedback",.8f);set(stress,"rumbleDrive",30);set(stress,"lfo_echoFeedback",1);set(stress,"ad_drive",-1);stress.prepareToPlay(sr,513);juce::AudioBuffer<float> block(2,513);for(int k=0;k<70;++k){juce::MidiBuffer midi;if(k%12==0)midi.addEvent(juce::MidiMessage::noteOn(1,60,(juce::uint8)127),0);stress.processBlock(block,midi);for(int channel=0;channel<2;++channel)for(int i=0;i<513;++i)check(std::isfinite(block.getSample(channel,i))&&std::abs(block.getSample(channel,i))<=1,"FX extremes unstable");}}
    std::cout<<"PASS LFO / AD / audible modulation / FX block invariance / drive extremes\n";
    juce::MemoryBlock state;fx1.getStateInformation(state);FKickProcessor restored;restored.setStateInformation(state.getData(),(int)state.getSize());check(difference(f1,render(restored,96000,256))<2e-6,"FX + route state roundtrip changed sound");
    auto xml=juce::AudioProcessor::getXmlFromBinary(state.getData(),(int)state.getSize());auto* params=xml->getChildByName("FKICK_PARAMETERS");
    // Eski 74 parametreli kayıt: yeni efektlerin önceki instance ayarlarını taşımamasını sına.
    for(int i=74;i<fx1.getParameters().size();++i)if(auto* p=dynamic_cast<juce::RangedAudioParameter*>(fx1.getParameters()[i]))
        for(auto* child=params->getFirstChildElement();child;){auto* next=child->getNextElement();if(child->getStringAttribute("id")==p->paramID)params->removeChildElement(child,true);child=next;}
    juce::MemoryBlock legacy;juce::AudioProcessor::copyXmlToBinary(*xml,legacy);restored.setStateInformation(legacy.getData(),(int)legacy.getSize());check(restored.value("rumbleMix")==0&&restored.value("echoMix")==0&&restored.value("lfo_rumbleLP")==0&&restored.value("hp")==0,"Legacy preset inherited current FX routes");
    std::cout<<"PASS FX/route state and legacy neutral migration\n";
    FKickProcessor visual;visual.setCurrentProgram(64);visual.prepareToPlay(48000,256);std::unique_ptr<juce::AudioProcessorEditor> ui(visual.createEditor());
    std::function<juce::Component*(juce::Component&,const juce::String&)> find=[&](juce::Component& root,const juce::String& id)->juce::Component*{if(root.getComponentID()==id)return &root;for(auto* c:root.getChildren())if(auto* r=find(*c,id))return r;return nullptr;};
    auto shot=[&](const char* name){auto image=ui->createComponentSnapshot(ui->getLocalBounds());auto stream=folder.getChildFile(name).createOutputStream();check(stream!=nullptr,"Techno GUI image stream");stream->setPosition(0);stream->truncate();check(juce::PNGImageFormat{}.writeImageToStream(image,*stream),"Techno GUI image");};
    shot("F-Kick-techno-sound.png");auto* fxPage=dynamic_cast<juce::TextButton*>(find(*ui,"page-fx"));check(fxPage,"FX page missing");fxPage->onClick();check(find(*ui,"rumbleMix")->isVisible()&&!find(*ui,"hp")->isVisible(),"FX page did not replace Sound controls");shot("F-Kick-fx.png");
    std::cout<<"PASS Sound/FX snapshots / SC remains the only modulation UI\n";
    const char* names[]={"F-Kick-Deep-Rumble.wav","F-Kick-Hard-Echo.wav","F-Kick-Wobble.wav","F-Kick-Industrial.wav"};
    for(int index=8;index<12;++index){FKickProcessor p;p.setCurrentProgram(index+56);p.prepareToPlay(48000,256);const int beat=juce::roundToInt(48000*60./135),total=beat*6+int(fkick::TechnoFX::tailSeconds(p.settings().fx,p.settings().motion)*48000)+1024;juce::AudioBuffer<float> audio(2,total);audio.clear();
        for(int offset=0;offset<total;offset+=256){const int n=juce::jmin(256,total-offset);juce::AudioBuffer<float> block(2,n);juce::MidiBuffer midi;for(int k=0;k<6;++k)if(k*beat>=offset&&k*beat<offset+n)midi.addEvent(juce::MidiMessage::noteOn(1,60,(juce::uint8)110),k*beat-offset);p.processBlock(block,midi);for(int c2=0;c2<2;++c2)audio.copyFrom(c2,offset,block,c2,0,n);}
        for(int c2=0;c2<2;++c2)audio.applyGainRamp(c2,total-960,960,1,0);check(energy(audio,0,juce::jmin(total,48000))>.01,"Techno demo silent");write(folder.getChildFile(names[index-8]),audio);
    }
    std::cout<<"PASS four 135 BPM techno audio demos\n";
}
