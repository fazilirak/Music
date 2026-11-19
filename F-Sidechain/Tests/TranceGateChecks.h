// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 Fazıl Irak
// Fazıl Irak · 19.11.2026: gate ritmi, süreklilik, tempo ve bant/state geri çağırma.
#pragma once
#include <set>
namespace trance_checks {
struct Clock final : juce::AudioPlayHead { PositionInfo position;juce::Optional<PositionInfo> getPosition() const override{return position;} };
inline juce::AudioBuffer<float> render(int preset,double rate,int block,double bpm,bool pad=false,bool tempoChange=false)
{
 auto processor=make();processor->setCurrentProgram(preset);
 processor->setRateAndBufferSizeDetails(rate,block);processor->prepareToPlay(rate,block);
 Clock clock;clock.position.setIsPlaying(true);clock.position.setBpm(bpm);processor->setPlayHead(&clock);
 const int total=int(rate*480/bpm);juce::AudioBuffer<float> out(2,total);double beat=0;
 for(int off=0;off<total;)
 {
  const int n=juce::jmin(block,total-off);const double tempo=tempoChange&&off>=total/2?bpm*1.25:bpm;
  clock.position.setTimeInSamples(off);clock.position.setPpqPosition(beat);clock.position.setBpm(tempo);
  juce::AudioBuffer<float> a(2,n);juce::MidiBuffer midi;
  for(int c=0;c<2;++c)for(int j=0;j<n;++j){double v=.25;
   if(pad){const double t=(off+j)/rate;v=0;for(double f:{130.8128,164.8138,195.9977})for(int h=1;h<=5;++h)v+=.035/h*std::sin(juce::MathConstants<double>::twoPi*f*h*t+(c? .14*h:0));}
   a.setSample(c,j,float(v));}
  processor->processBlock(a,midi);for(int c=0;c<2;++c)out.copyFrom(c,off,a,c,0,n);
  beat+=n*tempo/(60*rate);off+=n;
 }
 processor->setPlayHead(nullptr);return out;
}
inline void run(const juce::File& folder)
{
 folder.createDirectory();std::set<juce::String> curves;
 for(int preset=8;preset<16;++preset)
 {
  const auto points=SidechainPresets::points(preset);
  require(points.front().x==0 && points.back().x==1 && points.front().y==1 && points.back().y==1,"Gate wrap endpoints must be continuous and silent");
  for(size_t i=1;i<points.size();++i)require(points[i].x>points[i-1].x,"Gate nodes must be strictly ordered");
  curves.insert(juce::String(SidechainPresets::state(preset)));
  for(double rate:{44100.,48000.,96000.})for(double bpm:{90.,135.,180.})
  {
   auto audio=render(preset,rate,257,bpm);float maxJump=0;
   for(int c=0;c<2;++c)for(int i=1;i<audio.getNumSamples();++i){const float v=audio.getSample(c,i);require(std::isfinite(v)&&std::abs(v)<=.251f,"Gate output invalid");maxJump=juce::jmax(maxJump,std::abs(v-audio.getSample(c,i-1)));}
   require(audio.getMagnitude(0,0,audio.getNumSamples())>.1f,"Gate unexpectedly silent");
   require(maxJump<.02f,"Gate gain has an abrupt edge");
  }
  auto a=render(preset,48000,64,120),b=render(preset,48000,257,120);
  double difference=0;for(int c=0;c<2;++c)for(int i=0;i<a.getNumSamples();++i)difference=juce::jmax(difference,std::abs(double(a.getSample(c,i)-b.getSample(c,i))));
  require(difference<3e-5,"Gate timing depends on block size");
  auto changing=render(preset,48000,127,120,false,true);require(std::isfinite(changing.getMagnitude(0,changing.getNumSamples())),"Tempo automation invalid");
  auto p=make();p->setCurrentProgram(preset);juce::MemoryBlock state;p->getStateInformation(state);auto restored=make();restored->setStateInformation(state.getData(),int(state.getSize()));
  require(restored->band(1).getCurrentProgram()==preset,"New preset index did not restore");
  const auto& saved=*static_cast<volumef::VOLUMEFAudioProcessor&>(restored->band(1)).viewPattern;
  require(saved.points.size()==points.size(),"New curve points did not restore");
  auto pad=render(preset,48000,256,120,true);auto stream=folder.getChildFile("TG-"+juce::String(preset-7)+".wav").createOutputStream();require(stream!=nullptr,"Gate audition stream");
  juce::WavAudioFormat format;std::unique_ptr<juce::AudioFormatWriter> writer(format.createWriterFor(stream.get(),48000,2,24,{},0));require(writer!=nullptr,"Gate audition writer");stream.release();require(writer->writeFromAudioSampleBuffer(pad,0,pad.getNumSamples()),"Gate audition write");
 }
 require(curves.size()==8,"Duplicate gate presets");
 std::cout<<"PASS: eight distinct gate curves; 72 rate/tempo runs; smooth edges; 64/257 block timing; tempo changes; state recall; eight original pad/chord WAVs.\n";
}
}
