// SPDX-License-Identifier: AGPL-3.0-or-later
// Fazıl Irak · 03.11.2026: özgün, MIDI/onset tetiklemeli stereo transient şekillendirici.
#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
#include <cmath>
namespace fkick {
class TransientShaper {
public:
    void prepare(double sampleRate) noexcept {
        rate=sampleRate>0?sampleRate:48000.;step=1./rate;
        fastAttack=std::exp(-1./(.0003*rate));fastRelease=std::exp(-1./(.004*rate));
        slowAttack=std::exp(-1./(.012*rate));slowRelease=std::exp(-1./(.070*rate));
        gainPole=std::exp(-1./(.0004*rate));
        for(auto* g:{&attackGain,&pumpGain,&sustainGain})g->reset(rate,.006);
        reset();
    }
    void reset() noexcept {
        fast=slow=0;age=1;gain=1;armed=true;primed=false;bypass=true;
        for(auto* g:{&attackGain,&pumpGain,&sustainGain})g->setCurrentAndTargetValue(1.f);
    }
    // MIDI vuruşu hassas başlangıcı verir. Gecikmeli sample başlangıçları kuru sinyalde de algılanır.
    void trigger() noexcept {age=0;armed=false;}
    void set(float attack,float pump,float sustain) noexcept {
        auto sane=[](float x,float lo,float hi){return std::isfinite(x)?juce::jlimit(lo,hi,x):0.f;};
        attack=sane(attack,-100,100);pump=sane(pump,0,100);sustain=sane(sustain,-100,100);
        bypass=attack==0&&pump==0&&sustain==0;
        const float values[]{juce::Decibels::decibelsToGain(attack*.12f),
            juce::Decibels::decibelsToGain(-pump*.18f),juce::Decibels::decibelsToGain(sustain*.12f)};
        juce::SmoothedValue<float>* controls[]{&attackGain,&pumpGain,&sustainGain};
        for(int i=0;i<3;++i) {
            if(!primed||bypass)controls[i]->setCurrentAndTargetValue(values[i]);
            else controls[i]->setTargetValue(values[i]);
        }
        if(bypass)gain=1;primed=true;
    }
    std::array<double,2> process(double left,double right,double dryLeft,double dryRight) noexcept {
        // Algılama FX kuyruğundan değil kuru kick/sample'dan: reverb kendi kendini tetiklemez.
        const double level=juce::jmax(std::abs(dryLeft),std::abs(dryRight));
        const double fp=level>fast?fastAttack:fastRelease,sp=level>slow?slowAttack:slowRelease;
        fast=level+fp*(fast-level);slow=level+sp*(slow-level);
        if(fast<.00001||fast<slow*1.05)armed=true;
        if(armed&&age>.060&&fast>.0001&&fast>slow*1.6){age=0;armed=false;}
        const double t=age;age=juce::jmin(1.,age+step);
        // Sıfır üçlü, katsayı/çarpma kullanmadan örnekleri aynen döndürür (bit-exact bypass).
        if(bypass)return {left,right};
        const double a=1-ramp(t,.002,.024);
        const double p=ramp(t,.008,.020)*(1-ramp(t,.045,.100));
        const double s=ramp(t,.035,.090);
        // Attack ve Sustain bölgeleri çakışmaz; Pump yalnız 0..-18 dB zayıflatır.
        const double envelope=(1+(attackGain.getNextValue()-1)*a+(sustainGain.getNextValue()-1)*s)
            *(1-(1-pumpGain.getNextValue())*p);
        gain=envelope+gainPole*(gain-envelope);
        return {left*gain,right*gain}; // İki kanalda tek gain: stereo dengesi değişmez.
    }
private:
    static double ramp(double t,double from,double to) noexcept {
        const double x=juce::jlimit(0.,1.,(t-from)/(to-from));return x*x*(3-2*x);
    }
    double rate=48000,step=1./48000.,age=1,fast=0,slow=0,gain=1;
    double fastAttack=0,fastRelease=0,slowAttack=0,slowRelease=0,gainPole=0;
    bool armed=true,primed=false,bypass=true;
    juce::SmoothedValue<float> attackGain,pumpGain,sustainGain;
};
}
