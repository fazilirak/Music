// Fazıl Irak · 03.11.2026 · Classic eşitliği, bağımsız dönüşler, ritmik bass ve UI.
// SPDX-License-Identifier: AGPL-3.0-or-later
#include "PluginEditor.h"
#include "RumbleRecipes.h"
#include "Legacy/TechnoFX.h"
#include <iostream>
#include <stdexcept>
namespace {
void require(bool b,const char* text){if(!b)throw std::runtime_error(text);}
juce::Component* find(juce::Component& c,const juce::String& id){if(c.getComponentID()==id)return &c;for(auto* x:c.getChildren())if(auto* p=find(*x,id))return p;return nullptr;}
void shot(juce::Component& c,const juce::File& f){auto out=f.createOutputStream();require(out!=nullptr,"Rumble PNG stream");out->setPosition(0);out->truncate();require(juce::PNGImageFormat{}.writeImageToStream(c.createComponentSnapshot(c.getLocalBounds()),*out),"Rumble PNG");}
void wave(const juce::File& f,const juce::AudioBuffer<float>& a){auto out=f.createOutputStream();require(out!=nullptr,"Rumble WAV stream");out->setPosition(0);out->truncate();juce::WavAudioFormat fmt;std::unique_ptr<juce::AudioFormatWriter> w(fmt.createWriterFor(out.get(),48000,2,24,{},0));require(w!=nullptr,"Rumble WAV writer");out.release();require(w->writeFromAudioSampleBuffer(a,0,a.getNumSamples()),"Rumble WAV");}
struct Clock : juce::AudioPlayHead {PositionInfo pos;juce::Optional<PositionInfo> getPosition() const override{return pos;}};
juce::AudioBuffer<float> phrase(FKickProcessor& p,double rate=48000,int block=257,bool host=false,int samples=192000){
    p.prepareToPlay(rate,block);juce::AudioBuffer<float> out(2,samples);out.clear();Clock clock;
    if(host){clock.pos.setBpm(90);clock.pos.setIsPlaying(true);p.setPlayHead(&clock);}p.setParameter("tempo",120);
    for(int off=0;off<samples;){const int turn=int(rate),n=juce::jmin(juce::jmin(block,samples-off),off<turn?turn-off:block);
        const double first=juce::jmin(off,turn)/rate,second=juce::jmax(0,off-turn)/rate;
        clock.pos.setBpm(off<turn?90:170);clock.pos.setPpqPosition(first*90/60+second*170/60);
        juce::AudioBuffer<float> a(2,n);juce::MidiBuffer midi;
        for(int k=0;k<4;++k){int at=int(k*rate*.5);if(at>=off&&at<off+n)midi.addEvent(juce::MidiMessage::noteOn(1,60,(juce::uint8)110),at-off);}
        p.processBlock(a,midi);for(int c=0;c<2;++c)out.copyFrom(c,off,a,c,0,n);off+=n;
    }p.setPlayHead(nullptr);return out;
}
double diff(const juce::AudioBuffer<float>& a,const juce::AudioBuffer<float>& b){double d=0;for(int c=0;c<2;++c)for(int i=0;i<a.getNumSamples();++i)d=juce::jmax(d,std::abs(double(a.getSample(c,i)-b.getSample(c,i))));return d;}
double energy(const juce::AudioBuffer<float>& a){double e=0;for(int c=0;c<2;++c)for(int i=0;i<a.getNumSamples();++i)e+=double(a.getSample(c,i))*a.getSample(c,i);return e;}
fkick_legacy::FXSettings legacy(const fkick::FXSettings& a){fkick_legacy::FXSettings b;
#define COPY(x) b.x=a.x
    COPY(hp);COPY(rumbleMix);COPY(rumbleDecay);COPY(rumbleHP);COPY(rumbleLP);COPY(rumbleDrive);COPY(duck);COPY(recover);COPY(rumbleGain);
    COPY(echoMix);COPY(echoTime);COPY(feedback);COPY(echoLP);COPY(echoSync);COPY(echoDivision);COPY(echoSource);COPY(driveType);
    COPY(scShape);COPY(scShift);COPY(scMix);COPY(fxHP);COPY(fxLP);COPY(bassLift);COPY(bassQ);COPY(room);
#undef COPY
    return b;
}
}
void runRumbleTests(const juce::File& folder){
    // Aynı derleyicide tutulan 0.8.3 gerçek ses yolu ile örnek örnek karşılaştırma.
    double worst=0;
    for(double rate:{44100.,48000.,96000.})for(int variant=0;variant<6;++variant){
        fkick::TechnoFX modern;fkick_legacy::TechnoFX old;modern.prepare(rate);old.prepare(rate);
        fkick::FXSettings s;s.rumbleMix=.6f;s.rumbleGain=variant;s.rumbleDecay=.5f+variant*.4f;s.echoMix=.3f;s.echoSource=variant%2;s.room=.2f;
        s.scMix=variant%2?.8f:0;s.scShape=variant%4;s.scShift=.13f;s.fxHP=variant>2?80.f:0;s.fxLP=variant>3?1800.f:20000;
        s.bassLift=variant==5?3.f:0;s.driveType=variant%3;auto t=legacy(s);
        fkick::MotionSettings m;fkick_legacy::MotionSettings om;m.bpm=om.bpm=123;
        fkick::MotionFrame f;fkick_legacy::MotionFrame of;
        modern.begin(s,m);old.begin(t,om);
        for(int i=0;i<int(rate*.7);++i){if(i%int(rate*.22)==0){modern.trigger();old.trigger();}
            const double input=.3*std::sin(i*.067)*std::exp(-(i%int(rate*.22))/(rate*.03));
            const auto a=modern.process(input,input*.8,s,f),b=old.process(input,input*.8,t,of);
            for(int c=0;c<2;++c)worst=juce::jmax(worst,std::abs(a[size_t(c)]-b[size_t(c)]));
        }
    }
    require(worst<1e-12,"Classic changed compared to preserved 0.8.3 DSP");
    std::cout<<"PASS 18 Classic legacy comparisons, max difference="<<worst<<"\n";
    // Returnlar kapalıyken Split/Groove kuru kick ve sample yolunu değiştiremez.
    for(int mode:{0,1}){fkick::TechnoFX fx;fx.prepare(48000);fkick::FXSettings s;s.rumbleMode=mode;s.separateReturns=true;s.scMix=1;s.duck=1;fkick::MotionSettings m;fkick::MotionFrame f;fx.begin(s,m);fx.trigger();
        for(int n=0;n<12000;++n){double x=.1*std::sin(n*.13);auto y=fx.process(x,-x,s,f);require(y[0]==x&&y[1]==-x,"Split/Groove touched dry source");}}
    // Bağımsız SC katılımı: Room SC değişimi tek rumble dönüşünü etkilemez.
    FKickProcessor r1,r2,r3;for(auto* p:{&r1,&r2,&r3}){p->setCurrentProgram(64);p->setParameter("returnRouting",1);p->setParameter("rumbleMix",.6f);p->setParameter("echoMix",0);p->setParameter("room",0);p->setParameter("scMix",1);}
    r2.setParameter("roomSC",0);r3.setParameter("rumbleSC",0);
    auto a=phrase(r1),b=phrase(r2),c=phrase(r3);require(diff(a,b)==0,"Unrelated return SC leaked");require(diff(a,c)>.001,"Rumble SC participation inaudible");
    // Rumble peak limiter kuru yolu değiştirmeden kendi stereo dönüşünü sınırlar.
    fkick::TechnoFX limited;limited.prepare(48000);fkick::FXSettings s;s.rumbleMix=1;s.rumbleGain=24;s.rumbleDrive=30;s.duck=0;s.rumbleLimit=true;s.rumbleCeiling=-18;fkick::MotionSettings m;fkick::MotionFrame f;limited.begin(s,m);limited.trigger();double wetPeak=0;
    for(int n=0;n<48000;++n){double x=std::sin(n*.13);auto y=limited.process(x,x,s,f);wetPeak=juce::jmax(wetPeak,std::abs(y[0]-x));}
    require(wetPeak<juce::Decibels::decibelsToGain(-18.0)+1e-6,"Rumble limiter ceiling exceeded");
    // Bass, kuru giriş ve rumble olmadan da ses üretir; root frekansı gerçekten değişir.
    auto bassFrequency=[](int root){fkick::HybridBass bass;bass.prepare(48000);bass.trigger(0,60,1);int previous=-1;double periods=0;int count=0;double last=0;
        for(int n=0;n<12000;++n){double x=bass.tick(n/24000.,0,float(root),0,false,.25f,0,1500);
            if(n>6500&&n<11700&&last<=0&&x>0){if(previous>0){periods+=n-previous;++count;}previous=n;}last=x;}
        require(count>=2,"Bass root measurement silent");return 48000.*count/periods;};
    const double f1=bassFrequency(29),f2=bassFrequency(41);require(std::abs(f1-43.6535)<1&&std::abs(f2/f1-2)<.03,"Hybrid root tuning wrong");
    FKickProcessor hybrid;fkick::applyRumbleRecipe(hybrid,8);hybrid.setParameter("bodyGain",-60);hybrid.setParameter("sampleGain",-60);hybrid.setParameter("rumbleMix",0);hybrid.setParameter("echoMix",0);hybrid.setParameter("room",0);
    auto bassOnly=phrase(hybrid);
    for(int n=0;n<bassOnly.getNumSamples();++n)require(bassOnly.getSample(0,n)==bassOnly.getSample(1,n),"Independent bass is not mono compatible");
    require(energy(bassOnly)>.1,"Hybrid is just kick tail, not independent bass");wave(folder.getChildFile("F-Kick-Hybrid-Bass-Only.wav"),bassOnly);
    // Tempo, MIDI offset ve blok uzunluğundan bağımsız örüntü/oturum geri çağırma.
    for(int mode:{1,2}){FKickProcessor p,q;fkick::applyRumbleRecipe(p,mode*4);fkick::applyRumbleRecipe(q,mode*4);
        auto x=phrase(p,48000,127,true),y=phrase(q,48000,512,true);require(diff(x,y)<2e-4,"New rumble block-size dependent");
        juce::MemoryBlock state;p.getStateInformation(state);q.setStateInformation(state.getData(),int(state.getSize()));auto z=phrase(q,48000,257,true);require(diff(x,z)<2e-4,"New rumble state changed sound");
        FKickProcessor other;fkick::applyRumbleRecipe(other,mode*4);other.setParameter("groovePattern",2);require(diff(x,phrase(other,48000,127,true))>.001,"Groove patterns identical");
    }
    // Panic bağımsız bass'ı da susturur; gelecekteki pattern notaları geri dönmemeli.
    hybrid.prepareToPlay(48000,256);juce::AudioBuffer<float> buf(2,256);juce::MidiBuffer midi;midi.addEvent(juce::MidiMessage::noteOn(1,60,(juce::uint8)100),0);hybrid.processBlock(buf,midi);hybrid.requestPanic();midi.clear();
    for(int i=0;i<100;++i)hybrid.processBlock(buf,midi);require(buf.getMagnitude(0,0,256)<1e-6,"Hybrid panic did not stay silent");
    // Özgün tarifler: tüm hızlar, yeni parametrelerin kaydı ve gerçek görüntü.
    auto recipeDir=folder.getChildFile("Rumble-Recipes");recipeDir.createDirectory();juce::String report="recipe,peak,rms,dc,render_ms\n";
    for(int i=0;i<12;++i){FKickProcessor p;fkick::applyRumbleRecipe(p,i);
        for(double rate:{44100.,96000.}){auto x=phrase(p,rate,255,false,int(rate*.8));for(int ch=0;ch<2;++ch)for(int n=0;n<x.getNumSamples();++n)require(std::isfinite(x.getSample(ch,n)),"New mode nonfinite");}
        auto start=juce::Time::getHighResolutionTicks();auto x=phrase(p);const double ms=juce::Time::highResolutionTicksToSeconds(juce::Time::getHighResolutionTicks()-start)*1000;
        const float peak=x.getMagnitude(0,0,x.getNumSamples());require(peak>.01f&&peak<.98f,"Rumble recipe silent or clipped");double dc=0;for(int n=0;n<x.getNumSamples();++n)dc+=x.getSample(0,n);dc/=x.getNumSamples();require(std::abs(dc)<.015,"Rumble recipe DC");
        report+=juce::String(fkick::rumbleRecipeNames[i])+","+juce::String(peak,6)+","+juce::String(x.getRMSLevel(0,0,x.getNumSamples()),6)+","+juce::String(dc,8)+","+juce::String(ms,3)+"\n";
        const auto name=juce::String(fkick::rumbleRecipeNames[i]);require(p.savePreset(recipeDir.getChildFile(name+".fkick")),"Rumble recipe save");wave(recipeDir.getChildFile(name+".wav"),x);
        if(i==0||i==4||i==8){FKickEditor ui(p);auto* fx=dynamic_cast<juce::TextButton*>(find(ui,"page-fx"));require(fx!=nullptr,"FX page missing");fx->onClick();shot(ui,folder.getChildFile("rumble-mode-"+juce::String(i/4)+".png"));
            for(auto id:{"page-returns","page-bass"}){auto* button=dynamic_cast<juce::TextButton*>(find(ui,id));require(button!=nullptr,"Rumble subpage missing");button->onClick();shot(ui,folder.getChildFile(juce::String(id)+"-"+juce::String(i/4)+".png"));}
            require(ui.getWidth()==912&&ui.getHeight()==511,"Rumble UI expanded");
            for(auto id:{"rumbleLimit","bassFollow"}){auto* toggle=find(ui,id);require(toggle!=nullptr&&toggle->getParentComponent()!=nullptr&&toggle->isVisible(),"New rumble toggle missing from UI");}
        }
    }
    // Ayrı 1 saniyelik DSP ölçümü; host CPU yüzdesi/gerçek zaman garantisi değildir.
    juce::String performance="case,audio_seconds,render_ms\n";
    for(int kind=-1;kind<=2;++kind){
        double elapsed[3]{};
        for(int repeat=0;repeat<3;++repeat){
            fkick::TechnoFX current;fkick_legacy::TechnoFX reference;current.prepare(48000);reference.prepare(48000);
            fkick::FXSettings cfg;cfg.rumbleMix=.5f;cfg.echoMix=.2f;cfg.room=.1f;cfg.rumbleMode=juce::jmax(0,kind);
            auto oldCfg=legacy(cfg);fkick::MotionSettings timing;fkick_legacy::MotionSettings oldTiming;fkick::MotionFrame mf;fkick_legacy::MotionFrame oldMf;
            current.begin(cfg,timing);reference.begin(oldCfg,oldTiming);current.trigger();reference.trigger();
            volatile double sink=0;auto start=juce::Time::getHighResolutionTicks();
            for(int n=0;n<48000;++n){const double input=n%24000<2400?.1:0;if(n==24000){current.trigger();reference.trigger();}
                const auto y=kind<0?reference.process(input,input,oldCfg,oldMf):current.process(input,input,cfg,mf);sink+=y[0];}
            elapsed[repeat]=juce::Time::highResolutionTicksToSeconds(juce::Time::getHighResolutionTicks()-start)*1000;
        }
        std::sort(std::begin(elapsed),std::end(elapsed));performance+=juce::String(kind)+",1,"+juce::String(elapsed[1],3)+"\n";
    }
    folder.getChildFile("RUMBLE-084-PERFORMANCE.csv").replaceWithText(performance);
    folder.getChildFile("RUMBLE-084-METRICS.csv").replaceWithText(report);
    std::cout<<"PASS split dry isolation / per-return SC / linked rumble limiter / bass root "<<f1<<" Hz / octave / independent bass / patterns / state / panic / 12 recipes\n";
}
