// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (c) 2026 Fazıl Irak
// Geliştirici: Fazıl Irak | Oluşturma / son düzenleme: 27.10.2026
// Dosyanın görevi: Çoklu efekt makrolarının ses, otomasyon, host durumu ve editör doğrulaması.
// Üretilen techno loop tamamen sentetiktir; başka üründen ses/preset alınmaz.
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <iostream>
#include <stdexcept>
using namespace ft;
constexpr double tau=juce::MathConstants<double>::twoPi;
void require(bool ok,const char* what){if(!ok)throw std::runtime_error(what);}
juce::Component* findComponent(juce::Component& parent,const juce::String& id){
 if(parent.getComponentID()==id)return &parent;
 for(auto* child:parent.getChildren())if(auto* found=findComponent(*child,id))return found;
 return nullptr;
}
void set(Processor& p,const char* id,float value){auto* v=p.state.getParameter(id);v->setValueNotifyingHost(v->convertTo0to1(value));}
float input(int i,double sr){return (float)(.15*std::sin(i*tau*220/sr)+.06*std::sin(i*tau*3233/sr));}
std::vector<float> render(int mode,int character,double rate,int block,float amount,bool bypass=false,int channels=2,double duration=.6) {
 TransformDSP dsp;dsp.prepare(rate);Settings s{mode,character,amount,bypass};
 std::vector<float> result;int count=(int)(rate*duration);
 for(int pos=0;pos<count;pos+=block){int n=juce::jmin(block,count-pos);juce::AudioBuffer<float> b(channels,n);
   for(int c=0;c<channels;++c)for(int i=0;i<n;++i)b.setSample(c,i,input(pos+i,rate));
   dsp.process(b,nullptr,s,128);
   for(int i=0;i<n;++i){float x=b.getSample(0,i);require(std::isfinite(x)&&std::abs(x)<2,"Finite bounded output");result.push_back(x);}
 }return result;
}
void wav(const juce::File& file,const juce::AudioBuffer<float>& b) {
 file.deleteFile();juce::WavAudioFormat format;auto stream=file.createOutputStream();
 std::unique_ptr<juce::AudioFormatWriter> writer(format.createWriterFor(stream.release(),48000,b.getNumChannels(),24,{},0));
 require(writer&&writer->writeFromAudioSampleBuffer(b,0,b.getNumSamples()),"WAV export");
}
// Kick, offbeat bass, 8'lik hat, 2/4 snare ve kısa stereo stab: gerçek düzenleme üstünde makro karşılaştırması.
juce::AudioBuffer<float> technoLoop(int count) {
 juce::AudioBuffer<float> b(2,count);unsigned random=0x917a329u;
 for(int i=0;i<count;++i){double t=i/48000.,beat=t*128/60,age=(beat-std::floor(beat))*60/128;
   random^=random<<13;random^=random>>17;random^=random<<5;
   float noise=(float)((double)random/4294967295.0*2-1);
   float kick=(float)(.42*std::sin(tau*(48*age+140./32*(1-std::exp(-32*age))))*std::exp(-age*18));
   double hatAge=(beat*2-std::floor(beat*2))*60/128/2;
   float hat=noise*(float)(.06*std::exp(-hatAge*100));
   float snare=((int)beat%2)?noise*(float)(.15*std::exp(-age*32)):0.f;
   double bassAge=std::fmod(beat+.5,1.0)*60/128;
   float bass=(float)(.13*std::sin(tau*55*t)*std::exp(-bassAge*12));
   float stab=(float)(.055*(std::sin(tau*220*t)+.5*std::sin(tau*261.63*t)+.4*std::sin(tau*329.63*t))*std::exp(-age*10));
   b.setSample(0,i,kick+bass+snare+hat+stab);
   b.setSample(1,i,kick+bass+snare-hat*.8f+stab*.9f);
 }return b;
}
int main(int argc,char** argv){
 juce::ScopedJuceInitialiser_GUI gui;
 try {
 auto out=juce::File(argc>1?argv[1]:"dist");out.createDirectory();
 for(int m=0;m<techniqueCount;++m)for(int h=0;h<5;++h){
   auto dry=render(m,h,44100,257,0,false,2,.25);
   for(size_t i=0;i<dry.size();++i)require(dry[i]==input((int)i,44100),"Zero intensity must be exactly dry");
 }
 std::cout<<"PASS: 7 macros x 5 characters, exact dry at zero\n";
 for(double sr:{22050.,44100.,48000.,96000.})for(int m=0;m<techniqueCount;++m)for(int h:{0,4})
   render(m,h,sr,193,m==6?.93f:1,false,m%2?1:2);
 std::cout<<"PASS: all macros at four rates, character extremes and mono/stereo\n";
 std::vector<std::vector<float>> signatures;
 for(int m=0;m<techniqueCount;++m){
   auto a=render(m,3,48000,127,.8f,false,2,1.2),b=render(m,3,48000,512,.8f,false,2,1.2);
   for(size_t i=0;i<a.size();++i)require(std::abs(a[i]-b[i])<1.e-5,"Block-size independence");
   signatures.push_back(std::move(a));
 }
 for(int a=0;a<techniqueCount;++a)for(int b=a+1;b<techniqueCount;++b){double difference=0;
   for(size_t i=24000;i<signatures[a].size();++i)difference+=std::abs(signatures[a][i]-signatures[b][i]);
   require(difference/(signatures[a].size()-24000)>1.e-4,"Distinct macro audio");
 }
 std::cout<<"PASS: block-size independence and distinct audio for all seven macros\n";
 // Character yalnız etiket olmamalı: her modda bitişik kademelerin gerçek ses farkını ölç.
 // Bu bir null-fark ölçümüdür; algılanan ses yüksekliği veya dinleme değerlendirmesi değildir.
 juce::String characterReport="Macro,Silk-Soft,Soft-Balanced,Balanced-Hard,Hard-Extreme\n";
 for(int m=0;m<techniqueCount;++m){
   characterReport+=juce::String(techniques[(size_t)m].name);
   auto prior=render(m,0,48000,256,.6f,false,2,1);
   for(int h=1;h<5;++h){auto next=render(m,h,48000,256,.6f,false,2,1);double difference=0,reference=0;
     for(size_t i=24000;i<next.size();++i){double d=next[i]-prior[i];difference+=d*d;reference+=prior[i]*prior[i];}
     double ratio=std::sqrt(difference/juce::jmax(1.e-12,reference));
     require(ratio>.002,"Adjacent character settings must change audio");
     characterReport+=","+juce::String(ratio*100,2);prior=std::move(next);
   }
   characterReport+="\n";
 }
 require(out.getChildFile("CHARACTER-CHECK-0.2.1.csv").replaceWithText(characterReport),"Character report");
 std::cout<<"PASS: adjacent Character settings change audio in all seven macros (28 comparisons)\n";
 // Echo Lift: oturmuş %75 makroda 10 ms ses darbesinin ilk tekrarını doğrudan sesle karşılaştır.
 // 128 BPM noktalı sekizlik = 16875 örnek. Reverb'ün dağınık enerjisini azaltmak için dar pencere kullanılır.
 {
   TransformDSP e;e.prepare(48000);std::vector<float> samples(72000);
   for(int pos=0;pos<72000;pos+=240){juce::AudioBuffer<float>b(2,240);b.clear();
     for(int i=0;i<240;++i){int t=pos+i-12000;
       float hit=t>=0&&t<480?(float)(.15*std::sin(t*tau*2000/48000)*std::pow(std::sin(juce::MathConstants<double>::pi*t/480),2)):0;
       for(int c=0;c<2;++c)b.setSample(c,i,hit);}
     e.process(b,nullptr,{1,2,.75f,false},128);
     for(int i=0;i<240;++i)samples[(size_t)(pos+i)]=b.getSample(0,i);
   }
   double direct=0,repeat=0;for(int i=0;i<480;++i){direct+=std::pow(samples[(size_t)(12000+i)],2);repeat+=std::pow(samples[(size_t)(12000+16875+i)],2);}
   double ratio=std::sqrt(repeat/direct);require(ratio>.30,"Echo Lift first repeat must have clear weight");
   std::cout<<"PASS: Echo Lift first repeat / direct = "<<20*std::log10(ratio)<<" dB at 75% intensity\n";
 }
 // Nazik makro bası azaltmalı; tüm sinyali kesmemeli. İki frekansı ayrı ayrı ölç.
 auto toneRms=[](double hz){TransformDSP e;e.prepare(48000);double energy=0;
   for(int p=0;p<96000;p+=240){juce::AudioBuffer<float>b(1,240);
     for(int i=0;i<240;++i)b.setSample(0,i,(float)(.15*std::sin((p+i)*tau*hz/48000)));
     e.process(b,nullptr,{0,2,1,false},128);
     if(p>=48000)for(int i=0;i<240;++i)energy+=b.getSample(0,i)*b.getSample(0,i);}
   return std::sqrt(energy/48000);
 };
 require(toneRms(65)<toneRms(2000)*.35,"Air Bloom low-cut response");
 auto climax=render(6,4,48000,240,1,false,2,1);
 for(size_t i=36000;i<climax.size();++i)require(std::abs(climax[i])<1.e-5,"Critical Mass final dissolve");
 std::cout<<"PASS: soft low-cut response and full climax dissolve\n";
 // Birikmiş hall kuyruğunda da HP hareket etmeli: aynı 250 Hz besleme sonrası yalnız Intensity'yi yükselt.
 auto tailEnergy=[](bool rise){TransformDSP e;e.prepare(48000);double energy=0;
   for(int pos=0;pos<72000;pos+=240){juce::AudioBuffer<float>b(2,240);b.clear();
     if(pos<48000)for(int c=0;c<2;++c)for(int i=0;i<240;++i)
       b.setSample(c,i,(float)(.15*std::sin((pos+i)*tau*250/48000)));
     e.process(b,nullptr,{0,2,pos>=48000&&rise?1.f:.3f,false},128);
     if(pos>=52800)for(int i=0;i<240;++i)energy+=b.getSample(0,i)*b.getSample(0,i);
   }return energy;
 };
 double tailRatio=std::sqrt(tailEnergy(true)/tailEnergy(false));
 require(tailRatio<.4,"Rising intensity must high-pass the existing reverb tail");
 std::cout<<"PASS: existing 250 Hz hall tail reduced by "<<20*std::log10(tailRatio)<<" dB when intensity rises\n";
 // Sessiz girişte üreteçler kendiliğinden başlamaz; reverb/echo kuyruğu 12 saniyede söner.
 for(int m=0;m<techniqueCount;++m){TransformDSP e;e.prepare(48000);float last=0;
   for(int j=0;j<2500;++j){juce::AudioBuffer<float>b(2,240);b.clear();
     if(j<60)for(int c=0;c<2;++c)for(int i=0;i<240;++i)b.setSample(c,i,input(j*240+i,48000));
     e.process(b,nullptr,{m,4,.85f,false},128);
     if(j>2450)for(int c=0;c<2;++c)last=juce::jmax(last,b.getMagnitude(c,0,240));}
   require(last<1.e-4,"Silent tail and generator decay");
 }
 Processor p;p.prepareToPlay(48000,256);set(p,"technique",5);set(p,"intensity",.73f);set(p,"character",4);
 juce::MemoryBlock saved;p.getStateInformation(saved);Processor restored;restored.setStateInformation(saved.getData(),(int)saved.getSize());
 require(restored.settings().technique==5&&restored.settings().character==4&&std::abs(restored.settings().intensity-.73f)<1.e-6,"Session recall");
 require(p.getNumPrograms()==1&&p.getProgramName(0).isEmpty(),"No preset bank");
 // Şema etiketi olmayan 0.1 durumunda eski Crush(12) yeni Fracture(5)'e taşınır.
 auto old=p.state.copyState();old.removeProperty("engineVersion",nullptr);
 for(auto child:old)if(child["id"].toString()=="technique")child.setProperty("value",12,nullptr);
 juce::MemoryBlock oldBytes;juce::AudioProcessor::copyXmlToBinary(*old.createXml(),oldBytes);
 restored.setStateInformation(oldBytes.getData(),(int)oldBytes.getSize());require(restored.settings().technique==5,"Legacy state migration");
 juce::AudioBuffer<float> audio(2,256);juce::MidiBuffer midi;TransformDSP engine;engine.prepare(48000);
 for(int j=0;j<100;++j){for(int c=0;c<2;++c)for(int i=0;i<256;++i)audio.setSample(c,i,.12f);
   engine.process(audio,nullptr,{5,4,.9f,j>20},128);}
 require(std::abs(audio.getSample(0,255)-.12f)<1.e-6,"Bypass dry");
 for(int j=0;j<2000;++j){for(int c=0;c<2;++c)for(int i=0;i<256;++i)audio.setSample(c,i,input(j*256+i,48000));
   engine.process(audio,nullptr,{(j/31)%techniqueCount,(j/71)%5,(j%101)/100.f,false},40+(j%200));
   for(int c=0;c<2;++c)for(int i=0;i<256;++i)
     require(std::isfinite(audio.getSample(c,i))&&std::abs(audio.getSample(c,i))<2,"Automation stability");}
 std::cout<<"PASS: tails, state recall/migration, no presets, bypass and mode/tempo automation\n";
 set(p,"technique",4);set(p,"intensity",.67f);set(p,"character",2);
 std::unique_ptr<juce::AudioProcessorEditor> editor(p.createEditor());
 for(auto* child:editor->getChildren())if(auto* slider=dynamic_cast<juce::Slider*>(child)){
   bool stepped=slider->getComponentID()!="intensity";
   require(stepped?slider->getInterval()==1:slider->getInterval()==0,"Discrete selectors / continuous intensity");
   if(slider->getComponentID()=="technique")require(slider->getMaximum()==techniqueCount-1,"Seven macro positions");
 }
 auto image=editor->createComponentSnapshot(editor->getLocalBounds());
 {juce::FileOutputStream f(out.getChildFile("F-TransForm-preview.png"));f.setPosition(0);
   require(f.truncate().wasOk()&&juce::PNGImageFormat().writeImageToStream(image,f),"Preview PNG");}
 // Logo klavyeden de açılır; bütün lisanslar ikilinin içinden çevrimdışı gelmelidir.
 auto* logo=findComponent(*editor,"product-logo");auto* about=findComponent(*editor,"about-panel");
 require(logo&&about&&!about->isVisible(),"About starts hidden");
 require(logo->keyPressed(juce::KeyPress(juce::KeyPress::returnKey))&&about->isVisible(),"Logo opens About");
 auto* documents=dynamic_cast<juce::ComboBox*>(findComponent(*about,"about-documents"));
 auto* reader=dynamic_cast<juce::TextEditor*>(findComponent(*about,"about-text"));
 require(documents&&reader&&documents->getNumItems()==7,"Seven embedded licence documents");
 for(int i=1;i<=7;++i){documents->setSelectedId(i,juce::sendNotificationSync);
   require(reader->getText().length()>200&&!reader->getText().contains("unavailable"),"Embedded licence text available");}
 documents->setSelectedId(1,juce::sendNotificationSync);
 require(reader->getText().contains(juce::String::fromUTF8("Fazıl Irak")),"Original code owner in About");
 {auto png=editor->createComponentSnapshot(editor->getLocalBounds());
   juce::FileOutputStream f(out.getChildFile("F-TransForm-about.png"));f.setPosition(0);
   require(f.truncate().wasOk()&&juce::PNGImageFormat().writeImageToStream(png,f),"About preview PNG");}
 require(about->keyPressed(juce::KeyPress(juce::KeyPress::escapeKey))&&!about->isVisible(),"Escape closes About");
 std::cout<<"PASS: logo opens About, seven offline licence texts, owner attribution and close navigation\n";
 // 1 sn kuru giriş, 8 bar yükseliş, kısa zirve ve ardından Intensity=0 ile kuru drop.
 auto auditions=out.getChildFile("Auditions-0.2");auditions.createDirectory();
 const int count=18*48000;auto original=technoLoop(count);wav(auditions.getChildFile("00 Dry Techno.wav"),original);
 for(int m=0;m<techniqueCount;++m){juce::AudioBuffer<float> result(original);TransformDSP e;e.prepare(48000);
   for(int pos=0;pos<count;pos+=256){int n=juce::jmin(256,count-pos);auto* data=result.getArrayOfWritePointers();
     float* pointers[2]{data[0]+pos,data[1]+pos};juce::AudioBuffer<float> block(pointers,2,n);
     double time=pos/48000.;float a=time>=16?0.f:juce::jlimit(0.f,1.f,(float)((time-1)/15));
     e.process(block,nullptr,{m,2,a,false},128);}
   float peak=juce::jmax(result.getMagnitude(0,0,count),result.getMagnitude(1,0,count));
   require(peak<1.f,"Techno macro audition must not clip");
   std::cout<<"AUDITION "<<techniques[(size_t)m].name<<": peak "<<juce::Decibels::gainToDecibels(peak)<<" dBFS\n";
   wav(auditions.getChildFile(juce::String(m+1).paddedLeft('0',2)+" "+techniques[(size_t)m].name+".wav"),result);
 }
 std::cout<<"PASS: editor PNG, seven stereo techno macro auditions and dry reference exported\n";
 // Character için aynı giriş / aynı %60 Intensity ile iki uç kademe; ayrıca dinlenebilir kanıt üret.
 auto characterAudio=out.getChildFile("Character-0.2.1");characterAudio.createDirectory();
 for(int m:{1,4})for(int h:{0,4}){
   const int length=6*48000;juce::AudioBuffer<float> b(2,length);
   for(int c=0;c<2;++c)b.copyFrom(c,0,original,c,0,length);
   TransformDSP e;e.prepare(48000);
   for(int pos=0;pos<length;pos+=240){float* pointers[2]{b.getWritePointer(0)+pos,b.getWritePointer(1)+pos};
     juce::AudioBuffer<float> block(pointers,2,240);e.process(block,nullptr,{m,h,.6f,false},128);}
   wav(characterAudio.getChildFile(juce::String(techniques[(size_t)m].name)+" - "+characters[h]+".wav"),b);
 }
 std::cout<<"PASS: Silk/Extreme comparisons at fixed 60% intensity exported for Echo Lift and Spiral Rise\n";
 return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<std::endl;return 1;}
}

