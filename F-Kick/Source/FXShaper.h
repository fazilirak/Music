// Geliştirici: Fazıl Irak - Son düzenleme: 27.10.2026
// Dosyanın görevi: Yalnız efekt dönüşüne uygulanan dört eğri ve host PPQ eşlemesi.
// Copyright (c) 2026 Fazıl Irak - Özgün F-Kick uygulama kodu.
// SPDX-License-Identifier: AGPL-3.0-or-later
// Fazıl Irak · Dört özgün kazanç eğrisi; kick'i değil efekt dönüşlerini şekillendirir.
#pragma once
#include <juce_core/juce_core.h>
#include <cmath>
namespace fkick {
struct FXShaper {
    static double wrap(double p) noexcept {return p-std::floor(p);}
    static float curve(int shape,double phase,float shift=0) noexcept {
        const float p=float(wrap(phase-shift));
        float hold=.04f,end=.62f;
        if(shape==1){hold=0;end=.85f;}else if(shape==2){hold=.3f;end=.88f;}else if(shape==3){hold=.16f;end=.24f;}
        float t=juce::jlimit(0.f,1.f,(p-hold)/(end-hold));
        float y=t*t*(3-2*t);
        if(shape==1)y=std::sin(t*juce::MathConstants<float>::halfPi)*std::sin(t*juce::MathConstants<float>::halfPi);
        // Son %2'de yumuşak kapanış: döngü başına sıçrama bindirme.
        const float close=juce::jlimit(0.f,1.f,(1-p)/.02f);
        return juce::jlimit(0.f,1.f,y*close*close*(3-2*close));
    }
};
}
