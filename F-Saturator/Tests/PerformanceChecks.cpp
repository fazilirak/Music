// Fazıl Irak · 19.11.2026: repeatable CPU benchmark and pre/post audio comparison.
#include "PluginProcessor.h"
#include <iostream>
#include <algorithm>
#include <cmath>
int main(int argc, char** argv)
{
 juce::ScopedJuceInitialiser_GUI gui;
 juce::ScopedNoDenormals guard;
 const bool baseline=argc>1 && juce::String(argv[1])=="--baseline";
 const auto folder=juce::File::getCurrentWorkingDirectory().getChildFile("../backups/2026-11-19-0.5.5/measurements");
 folder.createDirectory();
 juce::String report="rate,block,model,ms_per_audio_second\n";
 double maxDifference=0, sumError=0;size_t compared=0;
 for(const double sr : {44100.,48000.,96000.}) for(const int block : {64,256}) for(int model=0;model<5;++model)
 {
  irak::Settings s;s.model=model;s.drive=18;s.bias=.12f;s.character=.2f;s.harmonics=.3f;
  juce::AudioBuffer<float> input(2,block), audio(2,block);
  for(int c=0;c<2;++c)for(int i=0;i<block;++i) input.setSample(c,i,float(.18*std::sin((i+c*19)*.071)+.035*std::cos(i*.31)));
  std::vector<double> times;
  for(int run=0;run<3;++run)
  {
   irak::SaturationDSP dsp;dsp.prepare(sr,block);
   for(int n=0;n<40;++n){audio.makeCopyOf(input,true);dsp.process(audio,s);}
   const int blocks=int(sr*.18)/block;
   const auto start=juce::Time::getHighResolutionTicks();
   for(int n=0;n<blocks;++n){audio.makeCopyOf(input,true);dsp.process(audio,s);}
   times.push_back(1000*juce::Time::highResolutionTicksToSeconds(juce::Time::getHighResolutionTicks()-start)/(blocks*block/sr));
  }
  std::sort(times.begin(),times.end());
  report+=juce::String(sr,0)+","+juce::String(block)+","+irak::modelNames[model]+","+juce::String(times[1],4)+"\n";
 }
 for(const double sr : {44100.,48000.,96000.,192000.})for(int scenario=0;scenario<10;++scenario)
 {
  irak::SaturationDSP dsp;dsp.prepare(sr,256);
  juce::AudioBuffer<float> audio(2,256);std::vector<float> result;
  irak::Settings s;s.model=scenario%5;s.drive=20;s.bias=.16f;s.character=.35f;
  if(scenario>=5){s.harmonics=.7f;s.drift=.6f;s.comp=.5f;s.transient=.3f;s.bass=4;s.treble=-3;s.bassTight=s.fastAttack=true;}
  for(int n=0;n<128;++n)
  {
   if(n==32)s.model=(s.model+1)%5;
   if(n==56){s.bias=-.25f;s.harmonics=-.6f;s.character=-.4f;}
   if(n==80){s.drive=8;s.drift=0;s.lowCut=85;s.highCut=12500;s.tone=-.3f;}
   if(n==110){s.bypass=true;s.mix=.3f;}
   for(int c=0;c<2;++c)for(int i=0;i<256;++i){const double t=(n*256+i)/sr;audio.setSample(c,i,float(.15*std::sin(6.283185307179586*(c?443:173)*t)+.04*std::cos(6.283185307179586*4091*t)));}
   dsp.process(audio,s);
   for(int i=0;i<256;++i)for(int c=0;c<2;++c)result.push_back(audio.getSample(c,i));
  }
  const auto file=folder.getChildFile("audio-"+juce::String(sr,0)+"-"+juce::String(scenario)+".f32");
  if(baseline){if(!file.replaceWithData(result.data(),result.size()*sizeof(float)))return 2;}
  else
  {
   juce::MemoryBlock data;if(!file.loadFileAsData(data)||data.getSize()!=result.size()*sizeof(float))return 3;
   const auto* original=static_cast<const float*>(data.getData());
   for(size_t i=0;i<result.size();++i){if(!std::isfinite(result[i]))return 4;const double error=std::abs(result[i]-original[i]);maxDifference=std::max(maxDifference,error);sumError+=error*error;++compared;}
  }
 }
 const auto out=folder.getChildFile(baseline?"before.csv":"after.csv");out.replaceWithText(report);
 std::cout<<report<<"Maximum audio difference: "<<maxDifference<<"; RMS error: "<<(compared?std::sqrt(sumError/compared):0)<<"; compared samples: "<<compared<<std::endl;
 if(!baseline){folder.getChildFile("audio-comparison.txt").replaceWithText("Compared samples: "+juce::String((juce::int64)compared)+"\nMaximum error: "+juce::String(maxDifference,12)+"\nRMS error: "+juce::String(std::sqrt(sumError/compared),12)+"\n");if(maxDifference>2e-6)return 5;}
 return 0;
}
