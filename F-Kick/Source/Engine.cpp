// Geliştirici: Fazıl Irak - Son düzenleme: 27.10.2026
// Dosyanın görevi: Sample oynatma, kick gövdesi, ses yönetimi ve MIDI olaylarının gerçek zamanlı işlenmesi.
// Copyright (c) 2026 Fazıl Irak - Özgün F-Kick uygulama kodu.
// SPDX-License-Identifier: AGPL-3.0-or-later
// Fazıl Irak · 27.10.2026 · Nota örnek doğruluğu işlemcidedir; burada sabit kapasiteli ses üretimi yapılır.
#include "Engine.h"
#include <cmath>

namespace fkick {
// Fazıl Irak: Normalize zamanı ilgili segmentte değerlendirir; eğrilik yalnız segment içinde uygulanır.
float Curve::at(float t) const noexcept {
    if (t <= x[0]) return y[0];
    for (int i=1; i<count; ++i) {
        if (t <= x[(size_t)i]) {
            float a = juce::jlimit(0.f, 1.f, (t-x[(size_t)i-1]) / juce::jmax(.00001f, x[(size_t)i]-x[(size_t)i-1]));
            const float exponent = std::exp2(bend[(size_t)i-1]*3.f);
            a = std::pow(a, exponent);
            return juce::jmap(a, y[(size_t)i-1], y[(size_t)i]);
        }
    }
    return y[(size_t)count-1];
}
void Curve::sanitise() noexcept {
    count=juce::jlimit(2,maxPoints,count); x[0]=0; x[(size_t)count-1]=1;
    for (int i=0; i<count; ++i) {
        y[(size_t)i]=std::isfinite(y[(size_t)i])?juce::jlimit(0.f,1.f,y[(size_t)i]):0;
        bend[(size_t)i]=std::isfinite(bend[(size_t)i])?juce::jlimit(-1.f,1.f,bend[(size_t)i]):0;
        if (i>0 && i<count-1) x[(size_t)i]=juce::jlimit(x[(size_t)i-1]+.001f, 1.f-.001f*(count-1-i), std::isfinite(x[(size_t)i])?x[(size_t)i]:float(i)/(count-1));
    }
}
void Engine::prepare(double sr) noexcept {
    rate=sr>0?sr:48000;
    dcPole=std::exp(-juce::MathConstants<double>::twoPi*5.0/rate);
    effects.prepare(rate);motion.prepare(rate);
    for (auto* s:{&sampleLevel,&bodyLevel,&cutoff,&drive,&output,&punchAmount}) s->reset(rate,.008);
    reset();
}
void Engine::reset() noexcept {
    voices={}; bend={}; pedal={}; ic1={}; ic2={}; dcX={}; dcY={}; serial=0;primed=false;
    effects.reset();motion.reset();lastMotion={};
    sampleLevel.setCurrentAndTargetValue(.6f); bodyLevel.setCurrentAndTargetValue(.65f);
    cutoff.setCurrentAndTargetValue(18000); drive.setCurrentAndTargetValue(0); output.setCurrentAndTargetValue(.7f);punchAmount.setCurrentAndTargetValue(0);
}
void Engine::setSample(const Sample* next) noexcept { reset(); sample=next; }
void Engine::beginRelease(Voice& v) noexcept { if (!v.releasing) { v.releasing=true; v.releaseGain=v.gain; } }
void Engine::panic() noexcept { for(auto& v:voices) { v.held=false; v.sustained=false;v.panic=true; beginRelease(v); } pedal={};effects.panic(); }
// Fazıl Irak: Yeni MIDI vuruşunda ses ayırır; note/velocity ve retrigger bilgisi aynı olayda yakalanır.
void Engine::noteOn(int note, int channel, float velocity, const Settings& s) noexcept {
    motion.trigger(s.motion);effects.trigger();
    Voice* chosen=nullptr;
    if (!s.poly) {
        chosen=&voices[0];
        for(auto& v:voices) if (&v!=chosen) { v.held=false; beginRelease(v); }
    } else {
        for (auto& v:voices) if(!v.active) {chosen=&v;break;}
        if (!chosen) chosen=&*std::min_element(voices.begin(),voices.end(),[](auto& a,auto& b){return a.serial<b.serial;});
    }
    const float oldL=chosen->lastL, oldR=chosen->lastR;
    *chosen={}; chosen->active=true; chosen->held=true; chosen->note=note; chosen->channel=channel;
    chosen->velocity=velocity; chosen->serial=++serial;
    chosen->phase=s.phase/360.0;
    chosen->stealL=oldL; chosen->stealR=oldR; chosen->stealFade=64;
    if(sample) {
        const auto n=sample->audio.getNumSamples();
        const float end=juce::jmax(s.start+.0001f,s.end);
        chosen->position=s.reverse?juce::jlimit(0.0,double(n-1),double(end*n)-1):double(s.start*n);
    }
}
void Engine::noteOff(int note,int channel,const Settings& s) noexcept {
    if (!(s.mode==1||s.gate)) return;
    for(auto& v:voices) if(v.active&&v.note==note&&v.channel==channel&&v.held) {
        v.held=false; v.sustained=pedal[(size_t)channel-1];
        if(!v.sustained) beginRelease(v);
    }
}
void Engine::sustainPedal(int ch,bool down,const Settings&) noexcept {
    pedal[(size_t)ch-1]=down;
    if(!down) for(auto& v:voices) if(v.channel==ch&&v.sustained&&!v.held) {v.sustained=false;beginRelease(v);}
}
int Engine::activeVoices() const noexcept { int n=0;for(auto& v:voices)if(v.active)++n;return n; }
// Fazıl Irak: Önceden ayrılmış ses ve sample hafızalarından üretir; ses callback içinde disk veya ağ erişimi yoktur.
void Engine::render(juce::AudioBuffer<float>& buffer,int offset,int n,const Settings& s) noexcept {
    if(!primed){sampleLevel.setCurrentAndTargetValue(s.sampleGain);bodyLevel.setCurrentAndTargetValue(s.bodyGain);cutoff.setCurrentAndTargetValue(s.cutoff);drive.setCurrentAndTargetValue(s.drive);output.setCurrentAndTargetValue(s.output);punchAmount.setCurrentAndTargetValue(s.punch);primed=true;}
    sampleLevel.setTargetValue(s.sampleGain); bodyLevel.setTargetValue(s.bodyGain);
    cutoff.setTargetValue(s.cutoff); drive.setTargetValue(s.drive); output.setTargetValue(s.output);punchAmount.setTargetValue(s.punch);
    effects.begin(s.fx,s.motion);
    const auto channels=buffer.getNumChannels();
    const float length=juce::jmax(.04f,s.length);
    const float panL=std::sqrt((1-s.samplePan)*.5f),panR=std::sqrt((1+s.samplePan)*.5f);
    const double low=sample?juce::jlimit(0.0,double(sample->audio.getNumSamples()-2),double(s.start*sample->audio.getNumSamples())):0;
    const double high=sample?juce::jlimit(low+2,double(sample->audio.getNumSamples()),double(s.end*sample->audio.getNumSamples())):2;
    for(int i=0;i<n;++i) {
        lastMotion=motion.tick(s.motion);
        double mix[2]{};const float impact=punchAmount.getNextValue();
        const float sg=sampleLevel.getNextValue(),bg=bodyLevel.getNextValue();
        for(auto& v:voices) if(v.active) {
            const auto t=float(v.age/length);
            float shape=0;
            if(s.mode==1) {
                const auto a=juce::jmax(.0005f,s.attack);
                const auto decay=juce::jmax(.01f,length*.3f);
                shape=v.age<a?float(v.age/a):juce::jmap(juce::jlimit(0.f,1.f,float((v.age-a)/decay)),1.f,s.sustain);
            } else shape=s.amp.at(t)*juce::jmin(1.f,float(v.age/juce::jmax(.0005f,s.attack)))
                       *juce::jlimit(0.f,1.f,float((length-v.age)/.003));
            if(v.releasing) { v.releaseGain=juce::jmax(0.f,v.releaseGain-float(1.0/(rate*(v.panic?.005f:juce::jmax(.002f,s.release)))));shape=juce::jmin(shape,v.releaseGain); }
            v.gain=shape;
            const float wheel=bend[(size_t)v.channel-1];
            const float tracking=(s.mode==1||s.trackTail)?float(v.note-60):0;
            const float env=s.pitch.at(juce::jmin(t,1.f));
            // Selective tracking: attack perdesi sabit kalırken son gövde notayı izler.
            const float semis=(tracking*(1-env)+s.sweep*env+wheel+lastMotion.amount[PitchMod]*12)/12.f;
            const double freq=juce::jmin(rate*.42,double(s.root)*std::exp2(semis));
            const double sine=std::sin(v.phase*juce::MathConstants<double>::twoPi);
            const double h2=freq*2<rate*.45?s.harmonics*.3*std::sin(v.phase*juce::MathConstants<double>::twoPi*2):0;
            const double h3=freq*3<rate*.45?s.harmonics*.15*std::sin(v.phase*juce::MathConstants<double>::twoPi*3):0;
            double body=(sine+h2+h3)/(1+s.harmonics*.45);
            if(s.mode==0&&impact>.0001f){
                // Faz kilitli kısa ikinci harmonik ve yoğunluk: peak yükseltmek yerine gövdeyi doldur.
                const double knock=freq*2<rate*.45?.32*impact*std::exp(-v.age/.045)*std::sin(v.phase*juce::MathConstants<double>::twoPi*2):0;
                const double density=1+impact*2.2;
                body=std::tanh((body+knock)*density)/std::tanh(density);
            }
            v.phase+=freq/rate;v.phase-=std::floor(v.phase);
            float sampleValues[2]{}; bool sampleAlive=false;
            if(sample) {
                const double span=high-low;
                if(s.loop&&s.mode==1&&!v.releasing) {
                    while(v.position>=high) v.position-=span;
                    while(v.position<low) v.position+=span;
                }
                if(v.position>=low&&v.position<high) {
                    sampleAlive=true;
                    const int pos=int(v.position), pos2=juce::jmin(pos+1,sample->audio.getNumSamples()-1);
                    const float frac=float(v.position-pos);
                    // Bölge kenarlarında en fazla 2 ms fade; sample tıklamalarını azaltır.
                    const float fade=juce::jmin(1.f,float(juce::jmin(v.position-low,high-v.position)/juce::jmax(1.0,juce::jmin(span*.1,sample->rate*.002))));
                    for(int c=0;c<2;++c) {
                        const auto* data=sample->audio.getReadPointer(juce::jmin(c,sample->audio.getNumChannels()-1));
                        sampleValues[c]=juce::jmap(frac,data[pos],data[pos2])*fade;
                    }
                    const double step=sample->rate/rate*std::exp2((s.sampleTune+(s.mode==1?v.note-60:0)+wheel)/12.f);
                    v.position+=(s.reverse?-step:step);
                }
            }
            const float level=shape*v.velocity;
            float l=(float(body)*bg+sampleValues[0]*sg*panL*1.41421356f)*level;
            float r=(float(body)*bg+sampleValues[1]*sg*panR*1.41421356f)*level;
            if(v.stealFade>0) {const float fade=v.stealFade/64.f;l=l*(1-fade)+v.stealL*fade;r=r*(1-fade)+v.stealR*fade;--v.stealFade;}
            v.lastL=l;v.lastR=r;mix[0]+=l;mix[1]+=r;v.age+=1/rate;
            if((v.releasing&&shape<=0)||(s.mode==0&&v.age>=length)||(s.mode==1&&!s.loop&&!sampleAlive&&bg<.00001f)) v.active=false;
        }
        const double f=juce::jlimit(20.0,rate*.45,double(cutoff.getNextValue())*std::exp2(lastMotion.amount[FilterMod]*4));
        const double g=std::tan(juce::MathConstants<double>::pi*f/rate);
        const double k=1.0/juce::jlimit(.5f,2.f,s.resonance), a1=1/(1+g*(g+k));
        const float dr=juce::jlimit(0.f,36.f,drive.getNextValue()+lastMotion.amount[DriveMod]*24),out=output.getNextValue();
        std::array<double,2> dry{};
        for(int c=0;c<2;++c) {
            const double v3=mix[c]-ic2[(size_t)c],v1=a1*(ic1[(size_t)c]+g*v3),v2=ic2[(size_t)c]+g*v1;
            ic1[(size_t)c]=2*v1-ic1[(size_t)c];ic2[(size_t)c]=2*v2-ic2[(size_t)c];
            dry[(size_t)c]=TechnoFX::distort(v2,dr,s.fx.driveType);
        }
        const auto processed=effects.process(dry[0],dry[1],s.fx,lastMotion);
        for(int c=0;c<2;++c) {
            const double value=processed[(size_t)c];
            const double dc=value-dcX[(size_t)c]+dcPole*dcY[(size_t)c];dcX[(size_t)c]=value;dcY[(size_t)c]=dc;
            const float result=float(juce::jlimit(-1.0,1.0,dc*out*juce::jlimit(0.f,2.f,1+lastMotion.amount[LevelMod])));
            if(channels==1) buffer.addSample(0,offset+i,result*.5f);
            else if(c<channels) buffer.setSample(c,offset+i,result);
        }
    }
}
} // namespace fkick
