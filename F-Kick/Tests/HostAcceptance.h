// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (c) 2026 Fazıl Irak
// Fazıl Irak · 19.11.2026: gerçek VST3 ikilisiyle çoklu instance ve otomasyon host matrisi.
#pragma once
#include "RumbleRecipes.h"
#include <algorithm>
namespace kick_acceptance {
struct Clock final : juce::AudioPlayHead {PositionInfo p;juce::Optional<PositionInfo> getPosition() const override{return p;}};
inline void apply(juce::AudioPluginInstance& target,int recipe)
{
 FKickProcessor source;fkick::applyRumbleRecipe(source,recipe);juce::MemoryBlock state;source.getStateInformation(state);
 juce::XmlElement vst("VST3PluginState");vst.createNewChildElement("IComponent")->addTextElement(state.toBase64Encoding());
 juce::MemoryBlock wrapped;juce::AudioProcessor::copyXmlToBinary(vst,wrapped);target.setStateInformation(wrapped.getData(),int(wrapped.getSize()));
}
inline void run(const juce::File& folder,const juce::String& bundle)
{
 FKickProcessor switches;juce::MemoryBlock saved;switches.getStateInformation(saved);
 for(auto* parameter:switches.getParameters())if(parameter->isBoolean()) {
  const float expected=parameter->getValue();
  for(float raw:{.22f,.41f,.5f,.83f}) {
   parameter->setValueNotifyingHost(raw);require(parameter->getValue()==(raw>=.5f?1.f:0.f),"Boolean host value must be binary");
   switches.setStateInformation(saved.getData(),int(saved.getSize()));require(parameter->getValue()==expected,"Boolean session recall mismatch");
  }
 }
 juce::VST3PluginFormat format;juce::OwnedArray<juce::PluginDescription> descriptions;format.findAllTypesForFile(descriptions,bundle);require(descriptions.size()==1,"Acceptance: VST3 not found");
 juce::String report="rate,block,instances,audio_seconds,median_callback_ms,p95_callback_ms,max_callback_ms,deadline_ms,overruns,reported_latency_samples\n";
 for(double sr:{44100.,48000.,96000.})for(int block:{64,256})for(int count:{1,4})
 {
  Clock clock;clock.p.setIsPlaying(true);clock.p.setBpm(135);
  std::vector<std::unique_ptr<juce::AudioPluginInstance>> instances;
  for(int k=0;k<count;++k){juce::String error;auto p=format.createInstanceFromDescription(*descriptions[0],sr,block,error);require(p!=nullptr,"Acceptance: instance creation failed");apply(*p,8+k%4);p->setPlayHead(&clock);p->setRateAndBufferSizeDetails(sr,block);p->prepareToPlay(sr,block);instances.push_back(std::move(p));}
  // Warm-up is outside measured callback intervals.
  juce::AudioBuffer<float> audio(2,block);juce::MidiBuffer midi;
  for(int k=0;k<24;++k){audio.clear();for(auto& p:instances)p->processBlock(audio,midi);}
  std::vector<double> times;int overruns=0;const double deadline=1000*block/sr;int lastBeat=-1;
  const int blocks=int(sr*3)/block;
  for(int n=0;n<blocks;++n)
  {
   double elapsed=n*block/sr;const double bpm=elapsed<1.5?135:155;
   const double beat=elapsed<1.5?elapsed*135/60:1.5*135/60+(elapsed-1.5)*155/60;
   clock.p.setTimeInSamples(juce::int64(n)*block);clock.p.setPpqPosition(beat);clock.p.setBpm(bpm);
   midi.clear();if(int(beat)!=lastBeat){midi.addEvent(juce::MidiMessage::noteOn(1,60,juce::uint8(110)),0);lastBeat=int(beat);}
   if(n%31==0)for(auto& p:instances)for(auto* parameter:p->getParameters())if(auto* value=dynamic_cast<juce::RangedAudioParameter*>(parameter))
      if(value->paramID=="scShift"||value->paramID=="duck"||value->paramID=="rumbleMix")value->setValueNotifyingHost(float(.15+.7*(n%127)/126.));
   const auto start=juce::Time::getHighResolutionTicks();
   for(auto& p:instances){audio.clear();p->processBlock(audio,midi);}
   const double ms=1000*juce::Time::highResolutionTicksToSeconds(juce::Time::getHighResolutionTicks()-start);times.push_back(ms);if(ms>deadline)++overruns;
   for(int c=0;c<2;++c)for(int j=0;j<block;++j)require(std::isfinite(audio.getSample(c,j)),"Acceptance: non-finite audio");
   if(n==blocks/2)for(auto& p:instances){juce::MemoryBlock state;p->getStateInformation(state);p->setStateInformation(state.getData(),int(state.getSize()));}
  }
  std::sort(times.begin(),times.end());report+=juce::String(sr,0)+","+juce::String(block)+","+juce::String(count)+",3,"+juce::String(times[times.size()/2],4)+","+juce::String(times[size_t(times.size()*.95)],4)+","+juce::String(times.back(),4)+","+juce::String(deadline,4)+","+juce::String(overruns)+","+juce::String(instances[0]->getLatencySamples())+"\n";
  for(auto& p:instances){p->setPlayHead(nullptr);p->releaseResources();}
 }
 // Every additional rumble recipe survives genuine VST3 session restoration.
 for(int recipe=0;recipe<12;++recipe)
 {
  juce::String error;auto p=format.createInstanceFromDescription(*descriptions[0],48000,256,error);require(p!=nullptr,"Recipe VST3 instantiate");apply(*p,recipe);juce::MemoryBlock state;p->getStateInformation(state);
  auto restored=format.createInstanceFromDescription(*descriptions[0],48000,256,error);require(restored!=nullptr,"Recipe reopen");restored->setStateInformation(state.getData(),int(state.getSize()));restored->prepareToPlay(48000,256);
  juce::AudioBuffer<float> audio(2,256);juce::MidiBuffer midi;midi.addEvent(juce::MidiMessage::noteOn(1,60,juce::uint8(110)),0);restored->processBlock(audio,midi);require(audio.getMagnitude(0,256)>1e-5,"Restored recipe silent");restored->releaseResources();
 }
 folder.getChildFile("HOST-ACCEPTANCE-2026-11-19.csv").replaceWithText(report);
 std::cout<<"PASS: real VST3 host matrix, 1/4 instances, 44.1/48/96 kHz, 64/256 blocks, MIDI, automation, tempo change, state recall, twelve recipe reopens. Not an Ableton/FL Studio acceptance test.\n"<<report;
}
}
