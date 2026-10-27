// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (c) 2026 Fazıl Irak
// Geliştirici: Fazıl Irak | Oluşturma / son düzenleme: 27.10.2026
// Dosyanın görevi: Parametreleri ses motoruna aktarmak, host temposunu almak ve DAW durumunu saklamak.
#include "PluginProcessor.h"
#include "PluginEditor.h"
namespace ft {
juce::AudioProcessorValueTreeState::ParameterLayout Processor::layout() {
 // Choice parametreleri ayrık adımlıdır: sol/sağ knob ara değerde durmaz.
 // Float olan Intensity kesintisizdir; DAW otomasyon eğrisiyle hareket ettirilebilir.
 juce::AudioProcessorValueTreeState::ParameterLayout p;
 juce::StringArray names; for(auto& t:techniques)names.add(t.name);
 p.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"technique",1},"Technique",names,0));
 p.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"intensity",1},"Intensity",juce::NormalisableRange<float>(0.f,1.f),0.f));
 p.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"character",1},"Character",juce::StringArray{"Silk","Soft","Balanced","Hard","Extreme"},2));
 p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"bypass",1},"Bypass",false));
 return p;
}
// Eski ikinci bus host düzeni için korunur; yeni makrolar yalnız ana girişi işler.
Processor::Processor():AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true)
 .withInput("Blend Source",juce::AudioChannelSet::stereo(),false).withOutput("Output",juce::AudioChannelSet::stereo(),true)),
 state(*this,nullptr,"FTRANSFORM_STATE",layout()) {state.state.setProperty("engineVersion",2,nullptr);}
Settings Processor::settings() const {
 return {(int)state.getRawParameterValue("technique")->load(),(int)state.getRawParameterValue("character")->load(),
 state.getRawParameterValue("intensity")->load(),state.getRawParameterValue("bypass")->load()>.5f};
}
bool Processor::isBusesLayoutSupported(const BusesLayout& l) const {
 // Ana giriş/çıkış eşleşmelidir. İkinci kaynak mono veya stereo olabilir.
 auto out=l.getMainOutputChannelSet();
 return (out==juce::AudioChannelSet::mono()||out==juce::AudioChannelSet::stereo())&&out==l.getMainInputChannelSet()
 && (l.getChannelSet(true,1).isDisabled()||l.getChannelSet(true,1)==juce::AudioChannelSet::mono()||l.getChannelSet(true,1)==juce::AudioChannelSet::stereo());
}
void Processor::processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer&) {
 // Kopyasız bus görünümü: sidechain kanalları yanlışlıkla çıkışa işlenmez.
 juce::ScopedNoDenormals dn;
 auto main=getBusBuffer(buffer,false,0); auto side=getBusBuffer(buffer,true,1);
 bool connected=getBus(true,1)->isEnabled(); sideConnected.store(connected);
 float peak=0; for(int c=0;c<main.getNumChannels();++c)peak=juce::jmax(peak,main.getMagnitude(c,0,main.getNumSamples()));
 inPeak.store(juce::jmax(peak,inPeak.load()*.86f));
 double bpm=120,ppq=-1;
 // Transport çalışıyorsa PPQ ritmi hizalar; durmuşsa DSP serbest zamanla devam eder.
 if(auto* play=getPlayHead())if(auto pos=play->getPosition()) {
   if(auto value=pos->getBpm())bpm=*value;
   if(pos->getIsPlaying())if(auto value=pos->getPpqPosition())ppq=*value;
 }
 tempo.store((float)bpm);
 auto s=settings();
 dsp.process(main,connected?&side:nullptr,s,bpm,ppq);
 peak=0;for(int c=0;c<main.getNumChannels();++c)peak=juce::jmax(peak,main.getMagnitude(c,0,main.getNumSamples()));
 outPeak.store(juce::jmax(peak,outPeak.load()*.86f));
}
juce::AudioProcessorEditor* Processor::createEditor(){return new Editor(*this);}
// Bu işlem preset üretmez; DAW projesi yeniden açıldığında kontrolleri geri yükler.
void Processor::getStateInformation(juce::MemoryBlock& b){
 auto tree=state.copyState();tree.setProperty("engineVersion",2,nullptr);
 auto xml=tree.createXml();copyXmlToBinary(*xml,b);
}
void Processor::setStateInformation(const void* data,int size){
 // Farklı eklentinin XML'i bu kök etiketiyle elenir.
 if(auto xml=getXmlFromBinary(data,size))if(xml->hasTagName("FTRANSFORM_STATE")) {
   auto tree=juce::ValueTree::fromXml(*xml);
   // 0.1'in 17 indeksini güvenli bir yeni makroya taşı. Eski ses motoru geri yüklenmez;
   // bu sürüm bilinçli bir ses tasarımı değişimidir ve eski otomasyon aynı ses sonucunu vermez.
   if((int)tree.getProperty("engineVersion",0)<2){
     constexpr int oldToNew[]{0,3,4,6,1,6,2,5,0,3,3,4,5,6,6,3,1};
     for(auto child:tree)if(child["id"].toString()=="technique"){
       int old=juce::jlimit(0,16,(int)(float)child["value"]);child.setProperty("value",oldToNew[old],nullptr);
     }
   }
   tree.setProperty("engineVersion",2,nullptr);state.replaceState(tree);
 }
}
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new ft::Processor();}
