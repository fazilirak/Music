// SPDX-License-Identifier: GPL-3.0-only
// Fazıl Irak · 19.11.2026: gerçek VST3 ile uzun render/tempo/otomasyon kabul kanıtı.
#pragma once
#include <algorithm>
namespace delay_acceptance {
struct Clock:juce::AudioPlayHead {
 PositionInfo position;
 juce::Optional<PositionInfo> getPosition() const override{return position;}
};
inline void parameter(juce::AudioProcessor& p,const juce::String& id,float value){
 const juce::String expected=id=="e10_feedback_max"?"Delay Feedback max":"Delay Filter max";
 for(auto* base:p.getParameters())if(base->getName(128)==expected){base->beginChangeGesture();base->setValueNotifyingHost(value);base->endChangeGesture();return;}
 throw std::runtime_error(("Missing host parameter "+id).toStdString());
}
inline juce::MemoryBlock fixture(){
 ShaperProcessor rack;rack.addEffect(10);rack.addEffect(8);
 auto& bands=dynamic_cast<MultibandProcessor&>(rack.module(10));
 for(int band=0;band<3;++band){auto& d=dynamic_cast<DelayEffect&>(bands.band(band));
  fixed(d,"send",1);fixed(d,"blend",.5f);fixed(d,"feedback",.6f);fixed(d,"filter",.8f);set(d,"division",4);
 }
 rack.switchAB(1);
 for(int band=0;band<3;++band){auto& d=dynamic_cast<DelayEffect&>(bands.band(band));set(d,"mode",1);fixed(d,"feedback",.75f);}
 rack.switchAB(0);juce::MemoryBlock raw;rack.getStateInformation(raw);
 juce::XmlElement xml("VST3PluginState");xml.createNewChildElement("IComponent")->addTextElement(raw.toBase64Encoding());
 juce::MemoryBlock wrapped;juce::AudioProcessor::copyXmlToBinary(xml,wrapped);return wrapped;
}
inline void run(const juce::String& bundle){
 juce::VST3PluginFormat format;juce::OwnedArray<juce::PluginDescription> descriptions;format.findAllTypesForFile(descriptions,bundle);
 check(descriptions.size()==1,"Built Jellybone VST3 scan");const auto saved=fixture();
 auto create=[&](double rate,int block){juce::String error;auto p=format.createInstanceFromDescription(*descriptions[0],rate,block,error);
  check(p!=nullptr,"Built VST3 instantiate");p->setStateInformation(saved.getData(),int(saved.getSize()));p->setRateAndBufferSizeDetails(rate,block);return p;};
 juce::String csv="rate,block,instances,audio_seconds,median_ms,p95_ms,max_ms,deadline_ms,overruns,latency_samples,tail_seconds,peak\n";
 auto out=juce::File::getCurrentWorkingDirectory().getChildFile("../docs/acceptance-2026-11-19");out.createDirectory();
 for(double rate:{44100.,48000.,96000.})for(int block:{64,257,1024}){
  auto p=create(rate,block);Clock clock;clock.position.setIsPlaying(true);clock.position.setBpm(120);p->setPlayHead(&clock);p->prepareToPlay(rate,block);
  const double duration=rate==48000&&block==257?300.:12.;const int total=int(rate*duration);
  juce::AudioBuffer<float> audio(4,block);juce::MidiBuffer midi;std::vector<double> times;times.reserve(total/block+1);
  float peak=0;int overruns=0;double ppq=0;const double deadline=1000.*block/rate;
  juce::AudioBuffer<float> audition(2,rate==48000&&block==257?48000*12:1);audition.clear();
  for(int offset=0;offset<total;offset+=block){
   const double seconds=offset/rate;const double bpm=seconds<4?90:seconds<8?135:180;
   clock.position.setTimeInSamples(offset);clock.position.setTimeInSeconds(seconds);clock.position.setPpqPosition(ppq);clock.position.setBpm(bpm);
   clock.position.setIsPlaying(seconds<duration-2);ppq+=block/rate*bpm/60.;midi.clear();
   if(offset%(block*17)==0)midi.addEvent(juce::MidiMessage::noteOn(1,60,juce::uint8(100)),0);
   if(offset%(block*19)==0){float v=float(.15+.7*(.5+.5*std::sin(seconds)));parameter(*p,"e10_feedback_max",v);parameter(*p,"e10_filter_max",.8f-.3f*v);}
   if(offset==block*(total/block/2)){juce::MemoryBlock state;p->getStateInformation(state);p->setStateInformation(state.getData(),int(state.getSize()));}
   audio.clear();
   for(int i=0;i<block;++i){const double t=(offset+i)/rate;const double beat=t*2;const double envelope=std::exp(-5*std::fmod(beat,1.));
    const float left=float(.12*envelope*(std::sin(t*juce::MathConstants<double>::twoPi*130.8128)+.5*std::sin(t*juce::MathConstants<double>::twoPi*196.)));
    const float right=float(.12*envelope*std::sin(t*juce::MathConstants<double>::twoPi*164.8138));
    if(seconds<duration-2){audio.setSample(0,i,left);audio.setSample(1,i,right);}
   }
   const auto start=juce::Time::getHighResolutionTicks();p->processBlock(audio,midi);
   const double ms=1000*juce::Time::highResolutionTicksToSeconds(juce::Time::getHighResolutionTicks()-start);times.push_back(ms);if(ms>deadline)++overruns;
   for(int c=0;c<2;++c)for(int i=0;i<block;++i){const float v=audio.getSample(c,i);check(std::isfinite(v)&&std::abs(v)<8,"Long VST3 render finite and bounded");peak=juce::jmax(peak,std::abs(v));}
   if(audition.getNumSamples()>1&&offset<audition.getNumSamples())for(int c=0;c<2;++c)audition.copyFrom(c,offset,audio,c,0,juce::jmin(block,audition.getNumSamples()-offset));
  }
  check(peak>1e-4,"Long render not silent");std::sort(times.begin(),times.end());
  csv+=juce::String(rate,0)+","+juce::String(block)+",1,"+juce::String(duration,0)+","+juce::String(times[times.size()/2],4)+","+juce::String(times[size_t(times.size()*.95)],4)+","+juce::String(times.back(),4)+","+juce::String(deadline,4)+","+juce::String(overruns)+","+juce::String(p->getLatencySamples())+","+juce::String(p->getTailLengthSeconds(),2)+","+juce::String(peak,6)+"\n";
  p->setPlayHead(nullptr);p->releaseResources();
  if(audition.getNumSamples()>1){juce::WavAudioFormat wav;std::unique_ptr<juce::FileOutputStream> stream(out.getChildFile("Delay-original-phrase.wav").createOutputStream());stream->setPosition(0);stream->truncate();
   std::unique_ptr<juce::AudioFormatWriter> writer(wav.createWriterFor(stream.get(),48000,2,24,{},0));check(writer!=nullptr,"Audition writer");stream.release();check(writer->writeFromAudioSampleBuffer(audition,0,audition.getNumSamples()),"Audition write");}
 }
 check(out.getChildFile("VST3-DELAY-MATRIX.csv").replaceWithText(csv),"Acceptance CSV");
 std::cout<<"PASS built VST3: nine rate/buffer cases, 300-second render, host PPQ/tempo/stop, MIDI, automation, state recall and finite stereo output. Offline host, not a live DAW test.\n"<<csv;
}
}
