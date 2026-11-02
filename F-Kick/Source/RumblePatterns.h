// Fazıl Irak · 03.11.2026 · Özgün Groove eğrileri ve bağımsız Hybrid bass.
// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <juce_core/juce_core.h>
#include <array>
#include <cmath>
namespace fkick {
struct RumblePatterns {
    // Bir bar = dört dörtlük; 16 hücre. Sabit tablolar ses iş parçacığında tahsis yapmaz.
    static float groove(double beat,int pattern) noexcept {
        constexpr float p[4][16]={
            {0,.3f,1,.55f,0,.35f,1,.6f,0,.3f,1,.5f,0,.5f,1,.7f},
            {0,.85f,.5f,.9f,0,.7f,1,.45f,0,1,.45f,.8f,0,.65f,.9f,.55f},
            {0,.2f,.9f,0,.3f,.8f,0,.7f,0,.4f,1,.25f,.6f,0,.8f,.45f},
            {0,.9f,0,.45f,.8f,0,.55f,.25f,0,.8f,.4f,0,.7f,.35f,1,.5f}};
        const double at=beat*4.0;const auto cell=(long long)std::floor(at);
        const float f=float(at-std::floor(at));
        const float attack=juce::jlimit(0.f,1.f,f/.08f),release=juce::jlimit(0.f,1.f,(1-f)/.25f);
        return p[juce::jlimit(0,3,pattern)][size_t((cell%16+16)%16)]*attack*release;
    }
};
class HybridBass {
public:
    void prepare(double sr) noexcept {rate=sr;reset();}
    void reset() noexcept {phase=low=age=0;lastStep=-999999999;until=-1;velocity=0;note=60;active=false;lastPitch=lastCutoff=-999;}
    void trigger(double beat,int key,float vel) noexcept {
        until=beat+4.0;note=key;velocity=juce::jlimit(0.f,1.f,vel);
    }
    // Dört beat'lik nota kalıbı son tetikten sonra kendi saatinde çalar; kick kuyruğu değildir.
    double tick(double beat,int pattern,float root,float tune,bool follow,float decay,float tone,float cutoff) noexcept {
        constexpr int rest=99;
        constexpr int notes[4][16]={
            {rest,0,0,0,rest,0,0,7,rest,0,0,0,rest,0,12,7},
            {rest,0,rest,7,rest,0,rest,0,rest,0,rest,10,rest,7,rest,0},
            {rest,0,7,rest,0,rest,12,7,rest,0,rest,7,0,rest,10,7},
            {rest,0,rest,-12,rest,7,0,rest,rest,0,12,rest,7,rest,0,7}};
        const auto step=(long long)std::floor(beat*4.0+1e-8);
        if(step!=lastStep) {
            lastStep=step;const int n=notes[juce::jlimit(0,3,pattern)][size_t((step%16+16)%16)];
            if(beat<until && n!=rest) {age=0;pitch=root+tune+n+(follow?note-60:0);active=true;}
        }
        if(!active)return 0;
        // Fazıl Irak: yalnız perde/kesim değiştiğinde katsayı hesapla.
        if(pitch!=lastPitch){lastPitch=pitch;cachedHz=juce::jlimit(15.0,rate*.12,440.0*std::exp2((pitch-69.0)/12.0));}
        const double hz=cachedHz;
        const double a=juce::jmin(1.0,age/.002)*std::exp(-age/juce::jmax(.02,double(decay)));
        double x=std::sin(phase*juce::MathConstants<double>::twoPi);
        // Sınırlı harmonikler: sub mono kalır; yüksek notada Nyquist üstü kısmi üretilmez.
        for(int h=2;h<=4;++h)if(h*hz<rate*.45)x+=tone*.35/h*std::sin(h*phase*juce::MathConstants<double>::twoPi);
        phase+=hz/rate;phase-=std::floor(phase);
        if(cutoff!=lastCutoff){lastCutoff=cutoff;cachedPole=1-std::exp(-juce::MathConstants<double>::twoPi*juce::jlimit(30.0,rate*.2,double(cutoff))/rate);}
        const double pole=cachedPole;
        low+=pole*(x*a*velocity-low);age+=1/rate;
        if(a<1e-6 && age>.01 && std::abs(low)<1e-7)active=false;
        return low;
    }
private:
    double rate=48000,phase=0,low=0,age=0,until=-1,pitch=36;
    long long lastStep=-999999999;
    double lastPitch=-999,lastCutoff=-999,cachedHz=55,cachedPole=.1;
    int note=60;float velocity=0;bool active=false;
};
}
