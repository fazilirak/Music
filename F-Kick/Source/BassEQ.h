// Geliştirici: Fazıl Irak - Son düzenleme: 27.10.2026
// Dosyanın görevi: Tam çıkıştaki bas raf filtresi ve 0 dB kuru geçiş sözleşmesi.
// Copyright (c) 2026 Fazıl Irak - Özgün F-Kick uygulama kodu.
// SPDX-License-Identifier: AGPL-3.0-or-later
// Fazıl Irak · F-EQ ile aynı 300 Hz low-shelf davranışı; sabit hafıza, tahsissiz katsayılar.
#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <array>
#include <cmath>

namespace fkick {
class BassEQ {
public:
    void prepare(double sr) noexcept {rate=sr;gain.reset(sr,.02);q.reset(sr,.02);reset();}
    void reset() noexcept {gain.setCurrentAndTargetValue(0);q.setCurrentAndTargetValue(.707f);z1={};z2={};lastGain=100;lastQ=0;}
    void begin(float db,float quality) noexcept {gain.setTargetValue(juce::jlimit(-12.f,12.f,db));q.setTargetValue(juce::jlimit(.1f,2.f,quality));}
    void tick() noexcept {
        const float db=gain.getNextValue(),quality=q.getNextValue();active=db!=0;
        if(!active){z1={};z2={};return;}
        if(db==lastGain&&quality==lastQ)return;
        lastGain=db;lastQ=quality;
        const double a=std::pow(10.,db/40.),w=juce::MathConstants<double>::twoPi*juce::jmin(300.,rate*.45)/rate;
        const double c=std::cos(w),t=std::sin(w)/quality*std::sqrt(a),a0=(a+1)+(a-1)*c+t;
        b0=a*((a+1)-(a-1)*c+t)/a0;b1=2*a*((a-1)-(a+1)*c)/a0;b2=a*((a+1)-(a-1)*c-t)/a0;
        a1=-2*((a-1)+(a+1)*c)/a0;a2=((a+1)+(a-1)*c-t)/a0;
    }
    double process(int channel,double x) noexcept {
        if(!active)return x;
        const auto i=(size_t)channel;const double y=b0*x+z1[i];
        z1[i]=b1*x-a1*y+z2[i];z2[i]=b2*x-a2*y;
        if(std::abs(z1[i])<1e-25)z1[i]=0;if(std::abs(z2[i])<1e-25)z2[i]=0;
        return y;
    }
private:
    double rate=48000,b0=1,b1=0,b2=0,a1=0,a2=0;
    float lastGain=100,lastQ=0;bool active=false;
    std::array<double,2> z1{},z2{};
    juce::SmoothedValue<float> gain,q;
};
}
