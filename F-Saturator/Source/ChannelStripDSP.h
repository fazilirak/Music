// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (c) 2026 Fazıl Irak
// Geliştirici: Fazıl Irak | Son düzenleme: 26.10.2026
// Özgün transient, stereo bağlı feedback kompresör ve iki bant ton katı.
#pragma once
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <cmath>

namespace irak
{
class ChannelStripDSP
{
    // Transposed direct form II: katsayı değişiminde durum korunur; örnek başına bellek ayırmaz.
    struct Filter
    {
        std::array<double, 5> c{1, 0, 0, 0, 0};
        std::array<double, 2> z1{}, z2{};
        double tick(double x, int ch)
        {
            const auto k = (size_t) ch;
            const double y = c[0] * x + z1[k];
            z1[k] = c[1] * x - c[3] * y + z2[k];
            z2[k] = c[2] * x - c[4] * y;
            return y;
        }
        void set(double b0, double b1, double b2, double a0, double a1, double a2)
        {
            c = {b0 / a0, b1 / a0, b2 / a0, a1 / a0, a2 / a0};
        }
        // RBJ shelving denklemleri, geniş geçiş (S=1). Bunlar belirli bir cihazın ölçüm modeli değildir.
        void shelf(double sr, double hz, double db, bool high)
        {
            const double a = std::pow(10.0, db / 40), w = 2 * juce::MathConstants<double>::pi * std::min(hz, sr * .4) / sr;
            const double co = std::cos(w), beta = std::sin(w) * std::sqrt(2 * a);
            if (high)
                set(a*((a+1)+(a-1)*co+beta), -2*a*((a-1)+(a+1)*co), a*((a+1)+(a-1)*co-beta),
                    (a+1)-(a-1)*co+beta, 2*((a-1)-(a+1)*co), (a+1)-(a-1)*co-beta);
            else
                set(a*((a+1)-(a-1)*co+beta), 2*a*((a-1)-(a+1)*co), a*((a+1)-(a-1)*co-beta),
                    (a+1)+(a-1)*co+beta, -2*((a-1)+(a+1)*co), (a+1)+(a-1)*co-beta);
        }
        void bell(double sr, double hz, double db, double q)
        {
            const double a=std::pow(10.0,db/40), w=2*juce::MathConstants<double>::pi*std::min(hz,sr*.4)/sr;
            const double alpha=std::sin(w)/(2*q), co=std::cos(w);
            set(1+alpha*a,-2*co,1-alpha*a,1+alpha/a,-2*co,1-alpha/a);
        }
        void highpass(double sr)
        {
            const double w=2*juce::MathConstants<double>::pi*32/sr, co=std::cos(w), al=std::sin(w)/(2*.70710678);
            set((1+co)/2,-(1+co),(1+co)/2,1+al,-2*co,1-al);
        }
    } bassShelf, bassContour, trebleShelf, trebleContour, subCut;
    double rate=48000, fastEnv=0, slowEnv=0, feedbackEnv=0;
    double fastPole=0, slowPole=0, attackPole=0, releasePole=0;
    double lastBass=100, lastTreble=100, lastBT=-1, lastTT=-1;
    juce::uint32 controlTick=0; // İşaretsiz sayaç uzun oturumlarda tanımlı biçimde sarar.
    bool primed=false;
    // Host hızında rampalar. Anahtarlar da rampalanır; otomasyonda sert filtre atlaması olmaz.
    juce::SmoothedValue<double> transient, compression, bass, treble, tightBass, tightTreble;
    static double pole(double ms, double sr) { return std::exp(-1.0/(.001*ms*sr)); }
    static double follow(double input, double& memory, double coefficient)
    { return memory = input + coefficient*(memory-input); }

  public:
    void prepare(double sr)
    {
        rate=sr;
        for (auto* v : {&transient,&compression,&bass,&treble,&tightBass,&tightTreble}) v->reset(sr,.02);
        fastPole=pole(1,sr); slowPole=pole(35,sr);
        reset();
    }
    void reset()
    {
        fastEnv=slowEnv=feedbackEnv=0; controlTick=0; primed=false;
        bassShelf={};bassContour={};trebleShelf={};trebleContour={};subCut={};
        subCut.highpass(rate);lastBass=lastTreble=100;lastBT=lastTT=-1;
    }
    void configure(double t, double c, double b, double h, bool fa, bool fr, bool bt, bool tt)
    {
        auto target=[&](auto& v,double x){if(!primed)v.setCurrentAndTargetValue(x);else v.setTargetValue(x);};
        target(transient,juce::jlimit(-1.0,1.0,t));target(compression,juce::jlimit(0.0,1.0,c));
        target(bass,juce::jlimit(-12.0,12.0,b));target(treble,juce::jlimit(-12.0,12.0,h));
        target(tightBass,bt?1:0);target(tightTreble,tt?1:0);
        // Stereo ortak feedback dedektörü: sol ve sağ aynı kazancı kullanır; görüntü sağa/sola kaymaz.
        attackPole=pole(fa?1.5:22,rate);releasePole=pole(fr?55:220,rate);primed=true;
    }
    // Saturation ÖNCESİ: hızlı/yavaş zarf farkı yalnız atak bölgesinin sürüşünü değiştirir.
    void processInput(float& l, float& r)
    {
        const double t=transient.getNextValue();
        if (t==0) { fastEnv=slowEnv=0; return; }
        const double peak=std::max(std::abs(l),std::abs(r));
        follow(peak,fastEnv,fastPole);follow(peak,slowEnv,slowPole);
        const double onset=juce::jlimit(0.0,1.0,(fastEnv-slowEnv)/(slowEnv+.02));
        const double gain=std::pow(10.0,t*9*onset/20);
        l=(float)(l*gain);r=(float)(r*gain);
    }
    // Saturation/Auto Gain SONRASI, Dry/Wet ÖNCESİ: kompresör ve ton yalnız wet kolundadır.
    void processOutput(std::array<float,2>& signal)
    {
        const double amount=compression.getNextValue(), b=bass.getNextValue(), h=treble.getNextValue();
        const double bt=tightBass.getNextValue(),tt=tightTreble.getNextValue();
        double reduction=1;
        if(amount>0)
        {
            const double db=20*std::log10(std::max(1e-12,feedbackEnv));
            const double over=db-(-12-18*amount), knee=8;
            const double curved=over<=-knee/2?0:over>=knee/2?over:(over+knee/2)*(over+knee/2)/(2*knee);
            // Feedback çıkış seviyesi kullanıldığı için eğim R-1'dir; sabit girişte hedef oran 1..6 olur.
            reduction=std::pow(10.0,-std::min(48.0,5*amount*curved)/20);
        }
        else feedbackEnv=0;
        double detector=0;
        for(auto& x:signal){x=(float)(x*reduction);detector=std::max(detector,std::abs((double)x));}
        if(amount>0)follow(detector,feedbackEnv,detector>feedbackEnv?attackPole:releasePole);
        // Katsayıları 16 host örnekte bir ve yalnız değişiklik varsa hesapla. Sabit ayarda trigonometrik yük yoktur.
        if(controlTick++%16==0 && (b!=lastBass||h!=lastTreble||bt!=lastBT||tt!=lastTT))
        {
            bassShelf.shelf(rate,120,b,false);
            bassContour.bell(rate,290,-.34*std::max(0.0,b)*bt,.65);
            trebleShelf.shelf(rate,6000-2500*tt,h,true);
            trebleContour.bell(rate,2300,-.22*std::min(0.0,h)*tt,.65);
            lastBass=b;lastTreble=h;lastBT=bt;lastTT=tt;
        }
        // Nötr değerlerde filtre hafızası da temizlenir; sıfıra dönünce önceki EQ kuyruğu sızmaz.
        if(b==0 && h==0){bassShelf.z1={};bassShelf.z2={};bassContour.z1={};bassContour.z2={};
            trebleShelf.z1={};trebleShelf.z2={};trebleContour.z1={};trebleContour.z2={};subCut.z1={};subCut.z2={};return;}
        if(b==0){bassShelf.z1={};bassShelf.z2={};bassContour.z1={};bassContour.z2={};subCut.z1={};subCut.z2={};}
        if(h==0){trebleShelf.z1={};trebleShelf.z2={};trebleContour.z1={};trebleContour.z2={};}
        for(int ch=0;ch<2;++ch)
        {
            double x=signal[(size_t)ch];
            if(b!=0){x=bassShelf.tick(x,ch);x=bassContour.tick(x,ch);
                const double high=subCut.tick(x,ch), depth=bt*std::min(1.0,std::abs(b)/6);x+=depth*(high-x);}
            if(h!=0){x=trebleShelf.tick(x,ch);x=trebleContour.tick(x,ch);}
            signal[(size_t)ch]=(float)x;
        }
    }
};
} // namespace irak
