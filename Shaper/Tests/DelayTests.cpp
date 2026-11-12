// SPDX-License-Identifier: GPL-3.0-only
// Fazıl Irak · 12.11.2026: Delay ses sözleşmesi; kuru uç, kuyruk, feedback, stereo ve state.
#include "DelayEffect.h"
#include "Editor.h"
#include <iostream>
#include <stdexcept>
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static void set(DelayEffect& p,const char* id,float v){auto* a=p.state.getParameter(id);check(a!=nullptr,id);a->setValueNotifyingHost(a->convertTo0to1(v));}
static void fixed(DelayEffect& p,const char* id,float v){set(p,(juce::String(id)+"_curve").toRawUTF8(),0);set(p,(juce::String(id)+"_min").toRawUTF8(),v);set(p,(juce::String(id)+"_max").toRawUTF8(),v);}
int main(){try{juce::ScopedJuceInitialiser_GUI gui;
 for(double sr:{44100.,48000.,96000.})for(int block:{32,257,1024}){
    DelayEffect p;fixed(p,"send",1);fixed(p,"blend",1);fixed(p,"feedback",.7f);fixed(p,"filter",1);set(p,"division",0);set(p,"time_ms",50);p.prepareToPlay(sr,block);
    juce::AudioBuffer<float> b(4,block);juce::MidiBuffer midi;double tail=0;float peak=0;int first=-1;
    // İlk boş bloklarla süre yumuşatıcısı hedefe yerleşir.
    for(int k=0;k<int(sr*.7)/block+1;++k){b.clear();p.processBlock(b,midi);}
    for(int k=0;k<int(sr*.6)/block+1;++k){b.clear();if(k==0)b.setSample(0,0,.5f);p.processBlock(b,midi);
        for(int i=0;i<block;++i){float l=b.getSample(0,i);check(std::isfinite(l)&&std::isfinite(b.getSample(1,i)),"finite stereo");peak=juce::jmax(peak,std::abs(l));if(first<0&&std::abs(l)>1e-5f)first=k*block+i;if(k*block+i>sr*.09)tail+=l*l;}
        if(k==0)fixed(p,"send",0); // Yeni giriş kapalı; mevcut tekrarlar bitmemeli.
    }
    check(first>int(sr*.045)&&first<int(sr*.055),"Free delay timing");check(tail>1e-5,"Send zero preserves tail");check(peak<1,"Bounded impulse feedback");
    fixed(p,"blend",0);fixed(p,"send",0);p.prepareToPlay(sr,block);
    for(int k=0;k<50;++k){for(int i=0;i<block;++i){b.setSample(0,i,.1f*std::sin(i*.1f));b.setSample(1,i,-.2f);}p.processBlock(b,midi);}
    for(int i=0;i<block;++i)check(std::abs(b.getSample(0,i)-.1f*std::sin(i*.1f))<1e-6,"Mix zero exact dry");
 }
 // Fazıl Irak: nota bölünmesi / tempo değişimi ve sıfır feedback uçları.
 for(float bpm:{120.f,240.f}){
    DelayEffect p;fixed(p,"send",1);fixed(p,"blend",1);fixed(p,"feedback",0);fixed(p,"filter",1);set(p,"division",4);set(p,"tempo",bpm);p.prepareToPlay(48000,128);
    juce::AudioBuffer<float> b(4,128);juce::MidiBuffer m;
    for(int k=0;k<400;++k){b.clear();p.processBlock(b,m);}
    int first=-1;double late=0;
    for(int k=0;k<500;++k){b.clear();if(k==0)b.setSample(0,0,.5f);p.processBlock(b,m);for(int i=0;i<128;++i){float x=b.getSample(0,i);if(first<0&&std::abs(x)>1e-5)first=k*128+i;if(k*128+i>24000)late+=x*x;}}
    check(std::abs(first-int(48000.*30./bpm))<8,"Tempo synced eighth note");check(late<1e-10,"Zero feedback has no repeats");
 }
 {double energies[2]{};for(int f=0;f<2;++f){DelayEffect p;fixed(p,"send",1);fixed(p,"blend",1);fixed(p,"feedback",0);fixed(p,"filter",float(f));set(p,"division",0);set(p,"time_ms",20);p.prepareToPlay(48000,128);
    juce::AudioBuffer<float> b(4,128);juce::MidiBuffer m;for(int k=0;k<200;++k){b.clear();for(int i=0;i<128;++i)b.setSample(0,i,.1f*std::sin((k*128+i)*juce::MathConstants<double>::twoPi*6000./48000.));p.processBlock(b,m);if(k>100)for(int i=0;i<128;++i)energies[f]+=std::pow(b.getSample(0,i),2);}}
    std::cerr<<"Filter energies "<<energies[0]<<" -> "<<energies[1]<<"\n";check(energies[1]>energies[0]*100,"Filter low endpoint darkens repeats");
 }
 {DelayEffect p;fixed(p,"send",1);fixed(p,"blend",1);fixed(p,"feedback",1);set(p,"mode",1);set(p,"division",0);set(p,"time_ms",20);p.prepareToPlay(48000,128);
  juce::AudioBuffer<float> b(4,128);juce::MidiBuffer m;double right=0;
  for(int k=0;k<2400;++k){b.clear();if(k==300)b.setSample(0,0,1);p.processBlock(b,m);for(int i=0;i<128;++i){check(std::isfinite(b.getSample(0,i))&&std::abs(b.getSample(0,i))<4.01f,"Max feedback stability");if(k>300)right+=b.getSample(1,i)*b.getSample(1,i);}}
  check(right>1e-4,"Ping-pong stereo output");
  for(int t=0;t<4;++t){set(p,(juce::String(t==0?"send":t==1?"blend":t==2?"feedback":"filter")+"_curve").toRawUTF8(),1);}
  set(p,"filter_min",.1f);set(p,"filter_max",.9f);set(p,"feedback_depth",.6f);p.setCurrentProgram(4);
  juce::MemoryBlock bytes;p.getStateInformation(bytes);DelayEffect restored;restored.setStateInformation(bytes.getData(),int(bytes.getSize()));std::cerr<<"Recall depth="<<restored.state.getRawParameterValue("feedback_depth")->load()<<" program="<<restored.getCurrentProgram()<<"\n";check(std::abs(restored.state.getRawParameterValue("feedback_depth")->load()-.6f)<1e-6f&&restored.getCurrentProgram()==4,"Delay state all targets and pattern");
 }
 {ShaperProcessor p;p.addEffect(10);p.prepareToPlay(48000,256);p.selectedEffect=10;ShaperEditor ui(p);ui.setZoom(.75f);
  auto folder=juce::File::getCurrentWorkingDirectory().getChildFile("../dist");folder.createDirectory();juce::FileOutputStream out(folder.getChildFile("preview-delay-090.png"));out.setPosition(0);out.truncate();check(juce::PNGImageFormat{}.writeImageToStream(ui.createComponentSnapshot(ui.getLocalBounds()),out),"Delay snapshot");
  juce::MemoryBlock bytes;p.getStateInformation(bytes);check(folder.getChildFile("Delay-090.shaper").replaceWithData(bytes.getData(),bytes.getSize()),"Delay test preset");ShaperProcessor r;r.setStateInformation(bytes.getData(),int(bytes.getSize()));check(r.palette()==std::vector<int>{10},"Rack Delay recall");
 }
 // Görsel kontrol: tüm efektler eklendiğinde paletin sağında boş + alanı kalmamalı.
 {ShaperProcessor p;for(int e=0;e<ShaperProcessor::effectCount;++e)p.addEffect(e);p.selectedEffect=10;
  ShaperEditor ui(p);ui.setZoom(1.f);auto file=juce::File::getCurrentWorkingDirectory().getChildFile("../dist/preview-full-rack-090.png");
  juce::FileOutputStream out(file);out.setPosition(0);out.truncate();check(juce::PNGImageFormat{}.writeImageToStream(ui.createComponentSnapshot(ui.getLocalBounds()),out),"Full rack snapshot");
 }
 juce::MessageManager::callAsync([]{juce::MessageManager::getInstance()->stopDispatchLoop();});juce::MessageManager::getInstance()->runDispatchLoop();
 std::cout<<"PASS Delay: 9 rate/block combinations, exact dry, Send tail, bounded feedback, ping-pong, state and UI\n";return 0;
}catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<"\n";return 1;}}
