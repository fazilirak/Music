// Fazıl Irak · 19.11.2026: interleaved, same-process 0.5.5 / 0.5.6 comparison.
#include "PluginProcessor.h"
#include "Reference055/SaturationDSP.h"
#include <algorithm>
#include <iostream>
#include <cmath>
#include <vector>
#if JUCE_WINDOWS
#define NOMINMAX
#include <windows.h>
#undef max
#undef min
#endif
template<class DSP,class Settings> double measure(DSP& dsp,const Settings& s,double sr,int block)
{
 juce::AudioBuffer<float> input(2,block),audio(2,block);
 for(int c=0;c<2;++c)for(int i=0;i<block;++i)input.setSample(c,i,float(.18*std::sin((i+c*19)*.071)+.035*std::cos(i*.31)));
 for(int n=0;n<20;++n){audio.makeCopyOf(input,true);dsp.process(audio,s);}
 const int blocks=int(sr*.12)/block;
 const auto start=juce::Time::getHighResolutionTicks();
 for(int n=0;n<blocks;++n){audio.makeCopyOf(input,true);dsp.process(audio,s);}
 return 1000*juce::Time::highResolutionTicksToSeconds(juce::Time::getHighResolutionTicks()-start)/(blocks*block/sr);
}
int main()
{
 juce::ScopedJuceInitialiser_GUI gui;juce::ScopedNoDenormals guard;
#if JUCE_WINDOWS
 // Pin only this benchmark thread so P/E core migration does not skew comparisons.
 DWORD_PTR allowed=0,system=0,previousAffinity=0;
 if(GetProcessAffinityMask(GetCurrentProcess(),&allowed,&system) && allowed!=0)
     previousAffinity=SetThreadAffinityMask(GetCurrentThread(),allowed & (~allowed+1));
#endif
 const auto folder=juce::File::getCurrentWorkingDirectory().getChildFile("../docs");
 juce::String report="rate,block,model,before_ms_per_audio_second,after_ms_per_audio_second,reduction_percent\n";
 for(double sr:{44100.,48000.,96000.})for(int block:{64,256})for(int model=0;model<5;++model)
 {
  irak::SaturationDSP now;irak_reference::SaturationDSP old;now.prepare(sr,block);old.prepare(sr,block);
  irak::Settings s;irak_reference::Settings os;s.model=os.model=model;s.drive=os.drive=18;s.bias=os.bias=.12f;s.character=os.character=.2f;s.harmonics=os.harmonics=.3f;
  std::vector<double> a,b;
  for(int n=0;n<3;++n){if(n%2){b.push_back(measure(now,s,sr,block));a.push_back(measure(old,os,sr,block));}else{a.push_back(measure(old,os,sr,block));b.push_back(measure(now,s,sr,block));}}
  std::sort(a.begin(),a.end());std::sort(b.begin(),b.end());
  report+=juce::String(sr,0)+","+juce::String(block)+","+irak::modelNames[model]+","+juce::String(a[1],4)+","+juce::String(b[1],4)+","+juce::String(100*(1-b[1]/a[1]),2)+"\n";
 }
 folder.getChildFile("CPU-0.5.6.csv").replaceWithText(report);std::cout<<report;
 double maxError=0;size_t samples=0;
 for(double sr:{44100.,48000.,96000.,192000.})for(int scenario=0;scenario<10;++scenario)
 {
  irak::SaturationDSP now;irak_reference::SaturationDSP old;now.prepare(sr,256);old.prepare(sr,256);
  juce::AudioBuffer<float> a(2,256),b(2,256);
  irak::Settings s;irak_reference::Settings os;
  s.model=os.model=scenario%5;s.drive=os.drive=20;s.bias=os.bias=.16f;s.character=os.character=.35f;
  if(scenario>=5){s.harmonics=os.harmonics=.7f;s.drift=os.drift=.6f;s.comp=os.comp=.5f;s.transient=os.transient=.3f;s.bass=os.bass=4;s.treble=os.treble=-3;s.bassTight=os.bassTight=s.fastAttack=os.fastAttack=true;}
  for(int n=0;n<128;++n)
  {
   if(n==32)s.model=os.model=(s.model+1)%5;
   if(n==56){s.bias=os.bias=-.25f;s.harmonics=os.harmonics=-.6f;s.character=os.character=-.4f;}
   if(n==80){s.drive=os.drive=8;s.drift=os.drift=0;s.lowCut=os.lowCut=85;s.highCut=os.highCut=12500;s.tone=os.tone=-.3f;}
   if(n==110){s.bypass=os.bypass=true;s.mix=os.mix=.3f;}
   for(int c=0;c<2;++c)for(int i=0;i<256;++i){double t=(n*256+i)/sr;a.setSample(c,i,float(.15*std::sin(6.283185307179586*(c?443:173)*t)+.04*std::cos(6.283185307179586*4091*t)));}
   b.makeCopyOf(a,true);now.process(a,s);old.process(b,os);
   for(int c=0;c<2;++c)for(int i=0;i<256;++i){const float x=a.getSample(c,i),y=b.getSample(c,i);if(!std::isfinite(x))return 1;maxError=std::max(maxError,double(std::abs(x-y)));++samples;}
  }
 }
 const auto comparison="Compared samples: "+juce::String((juce::int64)samples)+"\nMaximum sample error: "+juce::String(maxError,12)+"\n";
 folder.getChildFile("AUDIO-COMPARISON-0.5.6.txt").replaceWithText(comparison);std::cout<<comparison;
#if JUCE_WINDOWS
 if(previousAffinity!=0)SetThreadAffinityMask(GetCurrentThread(),previousAffinity);
#endif
 return maxError>2e-6?2:0;
}
