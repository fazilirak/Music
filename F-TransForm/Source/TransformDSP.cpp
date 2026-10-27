// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (c) 2026 Fazıl Irak
// Geliştirici: Fazıl Irak | Oluşturma / son düzenleme: 27.10.2026
// Dosyanın görevi: Filtre, hall, echo, phaser, frekans kaydırma, noise ve riser'ı tek makroyla sürmek.
// 0.2 motoru eski tek-efekt listesinin yerine geçer. Başka bir ürünün kodu veya ölçülmüş modeli değildir.
#include "TransformDSP.h"
#include <cmath>
namespace ft {
namespace {
constexpr double tau=juce::MathConstants<double>::twoPi;
// Her katman farklı Intensity aralığında başlar. Smoothstep eşiklerde sıçramayı önler.
float stage(float x,float lo,float hi) {
 float t=juce::jlimit(0.f,1.f,(x-lo)/(hi-lo));return t*t*(3-2*t);
}
float logFrequency(float bottom,float top,float p){return bottom*std::pow(top/bottom,p);}
}
std::array<float,TransformDSP::recipeFields> TransformDSP::values(const Recipe& r) {
 return {r.highpass,r.space,r.echo,r.feedback,r.drive,r.phaser,r.shift,r.noise,r.riser,r.dissolve,r.finish};
}
void TransformDSP::prepare(double rate) {
 sr=rate;echo.setSize(2,(int)(sr*4)+8);reverb.setSampleRate(sr);
 juce::Reverb::Parameters hall;hall.roomSize=.84f;hall.damping=.48f;hall.wetLevel=1;
 hall.dryLevel=0;hall.width=1;reverb.setParameters(hall);
 juce::dsp::ProcessSpec spec{sr,1,2};highpass.prepare(spec);reverbHighpass.prepare(spec);outputLowpass.prepare(spec);
 highpass.setType(juce::dsp::StateVariableTPTFilterType::highpass);
 reverbHighpass.setType(juce::dsp::StateVariableTPTFilterType::highpass);
 outputLowpass.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
 highpass.setResonance(.7071f);reverbHighpass.setResonance(.7071f);outputLowpass.setResonance(.7071f);
 amount.reset(sr,.018);hardness.reset(sr,.05);enabled.reset(sr,.02);
 for(auto& v:recipeValues)v.reset(sr,.045);
 // Pencerelenmiş Hilbert FIR: reel yol 31 örnek gecikir; kuadratur sin/cos tek yan bant üretir.
 // Bu frekans kaydırmasıdır, tüm frekansları aynı oranda çarpan pitch shift değildir.
 const int centre=(hilbertLength-1)/2;
 for(int k=0;k<hilbertLength;++k){int n=k-centre;
   double window=.42-.5*std::cos(tau*k/(hilbertLength-1))+.08*std::cos(2*tau*k/(hilbertLength-1));
   hilbertCoefficients[k]=(n!=0&&std::abs(n)%2==1)?(float)(2*window/(juce::MathConstants<double>::pi*n)):0;
 }
 reset();
}
void TransformDSP::reset() {
 echo.clear();reverb.reset();highpass.reset();reverbHighpass.reset();outputLowpass.reset();
 echoWrite=hilbertWrite=controlCounter=0;beat=shiftPhase=sweepPhase=0;rng=0x735ad12u;
 for(auto& channel:phaseMemory)for(auto& branch:channel)std::fill_n(branch,6,0.f);
 for(auto& channel:analyticHistory)std::fill_n(channel,64,0.f);
 for(auto& branch:phaserCoefficient)std::fill_n(branch,6,0.f);
 for(int c=0;c<2;++c)shiftSmooth[c]=feedbackLow[c]=noiseLow[c]=noiseBass[c]=dcIn[c]=dcOut[c]=0;
 for(auto& p:tonePhases)p=0;
 inputEnvelope=delayTime=delayFrom=delayTarget=0;delayFade=1;
 amount.setCurrentAndTargetValue(0);hardness.setCurrentAndTargetValue(.5f);enabled.setCurrentAndTargetValue(1);
 const auto defaults=values(recipes[0]);
 for(int k=0;k<recipeFields;++k)recipeValues[(size_t)k].setCurrentAndTargetValue(defaults[(size_t)k]);
}
float TransformDSP::readEcho(int c,float delay) const {
 // Tempo değişiminde iki okuma kafası çaprazlanır; sesin perdesi birden bükülmez.
 float pos=echoWrite-delay;const int size=echo.getNumSamples();
 if(pos<0)pos+=size;int index=(int)pos;float f=pos-index;
 return echo.getSample(c,index)*(1-f)+echo.getSample(c,(index+1)%size)*f;
}
float TransformDSP::lowpass(float x,float hz,float& state) const {
 float coefficient=(float)(1-std::exp(-tau*juce::jlimit(10.f,(float)sr*.45f,hz)/sr));
 state+=coefficient*(x-state);return state;
}
float TransformDSP::phaser(float x,int c) noexcept {
 float branches[2]{};
 for(int branch=0;branch<2;++branch){float y=x;
   for(int st=0;st<6;++st){float a=phaserCoefficient[branch][st];
     float next=-a*y+phaseMemory[c][branch][st];
     phaseMemory[c][branch][st]=y+a*next;y=next;}
   branches[branch]=y;
 }
 // Hann ağırlık: bir kolun sarma anında öteki kol duyulur; hareket daima yukarı sürer.
 float weight=(float)(.5-.5*std::cos(tau*sweepPhase));
 return branches[0]*weight+branches[1]*(1-weight);
}
float TransformDSP::shift(float x,int c,double phase) noexcept {
 analyticHistory[c][hilbertWrite]=x;float quadrature=0;
 // Tek merkezli FIR'ın sıfır olmayan katsayıları çift indekslerde bulunur.
 for(int k=0;k<hilbertLength;k+=2)
   quadrature+=hilbertCoefficients[k]*analyticHistory[c][(hilbertWrite-k+64)&63];
 float real=analyticHistory[c][(hilbertWrite-31+64)&63];
 return real*(float)std::cos(phase)-quadrature*(float)std::sin(phase);
}
void TransformDSP::process(juce::AudioBuffer<float>& b,const juce::AudioBuffer<float>*,Settings s,double bpm,double ppq) {
 juce::ScopedNoDenormals guard;if(echo.getNumSamples()==0||b.getNumChannels()==0)return;
 s.technique=juce::jlimit(0,techniqueCount-1,s.technique);s.character=juce::jlimit(0,4,s.character);
 bpm=std::isfinite(bpm)?juce::jlimit(20.0,400.0,bpm):120;
 if(std::isfinite(ppq)&&ppq>=0)beat=ppq;
 amount.setTargetValue(juce::jlimit(0.f,1.f,s.intensity));hardness.setTargetValue(s.character*.25f);
 enabled.setTargetValue(s.bypass?0.f:1.f);
 // Tarif hedefleri yumuşatılır: hızlı mod değişimi de mevcut ara değerden devam eder.
 const auto target=values(recipes[(size_t)s.technique]);
 for(int k=0;k<recipeFields;++k)recipeValues[(size_t)k].setTargetValue(target[(size_t)k]);
 // Noktalı sekizlik echo, BPM değişiminde 50 ms iki-kafa çapraz geçiş.
 float requested=juce::jlimit(2.f,(float)echo.getNumSamples()-2,(float)(sr*60/bpm*.75));
 if(delayTime==0){delayTime=delayFrom=delayTarget=requested;delayFade=1;}
 if(std::abs(requested-delayTarget)>.5f){delayFrom=delayTime;delayTarget=requested;delayFade=0;}
 const int channels=juce::jmin(2,b.getNumChannels());
 for(int i=0;i<b.getNumSamples();++i){
   std::array<float,recipeFields> r{};
   for(int k=0;k<recipeFields;++k)r[(size_t)k]=recipeValues[(size_t)k].getNextValue();
   float intensity=amount.getNextValue(),h=hardness.getNextValue(),active=enabled.getNextValue();
   // Karakter tüm zincirin başlama eğrisini ve son HP/drive hedeflerini değiştirir.
   float a=std::pow(intensity,juce::jmap(h,1.35f,.72f));
   float filtering=stage(a,0,1),space=stage(a,.02f,.82f),delay=stage(a,.12f,.90f);
   float motion=stage(a,.18f,.94f),grit=stage(a,.30f,1),noiseAmount=stage(a,.40f,1);
   float hpEnd=juce::jmin((float)sr*.43f,r[0]*juce::jmap(h,.72f,1.16f));
   // Filtre güncelleme sayacı callback boyundan bağımsızdır; blok farklılığı sesi değiştirmez.
   if(controlCounter==0){
     highpass.setCutoffFrequency(logFrequency(20,hpEnd,filtering));
     // Yalnız reverb beslemesini değil, içeride önceden birikmiş kuyruğu da süpür.
     // Nazik modlarda bile hall son bölgede havalanır; yüksek modlarda ana HP'yi izler.
     float hallEnd=juce::jmin((float)sr*.43f,juce::jmax(1200.f,hpEnd));
     reverbHighpass.setCutoffFrequency(logFrequency(35,hallEnd,stage(a,.08f,1)));
     outputLowpass.setCutoffFrequency(juce::jmin((float)sr*.44f,18500.f-3500.f*grit*r[4]));
     for(int branch=0;branch<2;++branch){double phase=std::fmod(sweepPhase+branch*.5,1.0);
       for(int st=0;st<6;++st){
         float hz=juce::jlimit(35.f,(float)sr*.40f,70.f*std::pow(90.f,(float)phase)*std::pow(1.27f,(float)st));
         float t=std::tan((float)(juce::MathConstants<double>::pi*hz/sr));phaserCoefficient[branch][st]=(1-t)/(1+t);
       }
     }
   }
   controlCounter=(controlCounter+1)&15; // Uzun oturumlarda işaretli sayaç taşması oluşmaz.
   float in[2]{b.getSample(0,i),b.getSample(channels>1?1:0,i)};
   float level=juce::jmax(std::abs(in[0]),std::abs(in[1]));
   inputEnvelope=juce::jmax(level,inputEnvelope*(float)std::exp(-1/(sr*.18)));
   // Noise ve oktav bulutu, giriş enerjisi sönünce kapanır; duruşta süresiz üreteç sesi kalmaz.
   float generatorGate=stage(inputEnvelope,.0001f,.015f),riser=0;
   for(int octave=0;octave<8;++octave){
     double p=std::fmod(sweepPhase+octave/8.0,1.0),hz=40*std::pow(256.0,p);
     tonePhases[octave]+=hz/sr;tonePhases[octave]-=std::floor(tonePhases[octave]);
     riser+=(float)(std::sin(tau*tonePhases[octave])*(.5-.5*std::cos(tau*p))*.16);
   }
   delayFade=juce::jmin(1.f,delayFade+1.f/(float)(sr*.05));delayTime=juce::jmap(delayFade,delayFrom,delayTarget);
   float echoes[2]{};
   for(int c=0;c<2;++c)echoes[c]=juce::jmap(delayFade,readEcho(c,delayFrom),readEcho(c,delayTarget));
   float source[2]{},hall[2]{};
   for(int c=0;c<2;++c){
     float filtered=highpass.processSample(c,in[c]),drive=1+grit*r[4]*(2+4*h);
     float driven=std::tanh(filtered*drive)/std::sqrt(drive);
     float coloured=filtered+grit*r[4]*(driven-filtered);
     float phased=phaser(coloured,c);coloured+=r[5]*motion*.65f*(phased-coloured);source[c]=coloured;
     float shifted=shift(coloured+echoes[1-c]*.5f,c,shiftPhase);shiftSmooth[c]=shifted;
     // Kaydırılmış ses feedback'e girer; art arda gelen yankılar yukarıya doğru katmanlanır.
     float feedback=lowpass(echoes[1-c],6500,feedbackLow[c]),feedbackAmount=r[3]*stage(a,.05f,1);
     echo.setSample(c,echoWrite,std::tanh(coloured*(.65f+.35f*(1-a))+feedback*feedbackAmount+shifted*r[6]*motion*.22f));
     hall[c]=coloured*(.65f+.35f*(1-a))+echoes[c]*r[2]*delay*.35f;
   }
   reverb.processStereo(&hall[0],&hall[1],1);
   for(int c=0;c<2;++c)hall[c]=reverbHighpass.processSample(c,hall[c]);
   for(int c=0;c<channels;++c){
     rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;float n=(float)((double)rng/4294967295.0*2-1);
     float noise=lowpass(n,logFrequency(650,14500,noiseAmount),noiseLow[c]);
     noise-=lowpass(noise,300+3000*noiseAmount,noiseBass[c]);
     float generator=(noise*r[7]*(.6f+.8f*h)+riser*r[8])*noiseAmount*generatorGate;
     // Filtrelenmiş gövde çekilirken reverb ve echo büyür; bütün katmanlar aynı anda çalışır.
     float processed=source[c]*(1-r[9]*stage(a,.25f,1))
       +hall[c]*r[1]*space*.9f+echoes[c]*r[2]*delay*.65f
       +shiftSmooth[c]*r[6]*motion*.24f+generator;
     processed=outputLowpass.processSample(c,processed);
     // Fracture son bölgede kısmen, Critical Mass tamamen çözülür.
     processed*=1-r[10]*stage(a,.91f,1);
     float dc=processed-dcIn[c]+.995f*dcOut[c];dcIn[c]=processed;dcOut[c]=dc;
     // Yalnız efekt yolunda yüksek tepe koruması; 0.9 altındaki ses değişmez, kuru yol aynen kalır.
     if(std::abs(dc)>.9f)dc=std::copysign(.9f+.09f*std::tanh((std::abs(dc)-.9f)/.09f),dc);
     // 0% tam kuru. Drop'ta makro sıfıra dönünce kuyruk da karışımdan çıkar.
     float engage=stage(intensity,0,.075f)*active;
     float out=in[c]+engage*(dc-in[c]);b.setSample(c,i,std::isfinite(out)?out:0.f);
   }
   hilbertWrite=(hilbertWrite+1)&63;echoWrite=(echoWrite+1)%echo.getNumSamples();
   shiftPhase+=tau*(2+motion*(8+42*h))/sr;shiftPhase=std::fmod(shiftPhase,tau);
   sweepPhase+=(bpm/(60*sr))*(.04+.16*motion);sweepPhase-=std::floor(sweepPhase);
   beat+=bpm/(60*sr);
 }
}
}

