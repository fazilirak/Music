// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (c) 2026 Fazıl Irak
// Geliştirici: Fazıl Irak | Son düzenleme: 26.10.2026
// Yeni düğmelerin yalnız görsel olmadığını, ölçülen ses ve preset davranışıyla doğrular.
#pragma once
#include <iostream>
inline bool channelStripChecks()
{
    auto signal=[](double bass,double treble,bool bt,bool tt,double freq)
    {
        irak::ChannelStripDSP strip;strip.prepare(48000);strip.configure(0,0,bass,treble,false,false,bt,tt);
        double energy=0;
        for(int i=0;i<48000;++i)
        {
            std::array<float,2> x{(float)(.1*std::sin(juce::MathConstants<double>::twoPi*freq*i/48000)),0};
            strip.processInput(x[0],x[1]);strip.processOutput(x);
            if(i>=24000)energy+=x[0]*x[0];
        }
        return std::sqrt(energy/24000);
    };
    // Sıfır kontroller ve Tight/fast anahtarları tek başına eski sesi değiştirmemeli.
    for(double sr : {44100.,48000.,96000.,192000.})
    {
        irak::ChannelStripDSP strip;strip.prepare(sr);strip.configure(0,0,0,0,true,true,true,true);
        for(int i=0;i<10000;++i)
        {
            std::array<float,2> x{(float)(.2*std::sin(i*.031)),(float)(.3*std::cos(i*.017))};const auto before=x;
            strip.processInput(x[0],x[1]);strip.processOutput(x);
            if(x!=before){std::cerr<<"Strip neutral changed audio\n";return false;}
        }
    }
    const double low=signal(0,0,false,false,40),high=signal(0,0,false,false,12000);
    const double bassBoost=20*std::log10(signal(6,0,false,false,40)/low);
    const double trebleBoost=20*std::log10(signal(0,6,false,false,12000)/high);
    const double tightBass=signal(6,0,true,false,40),plainBass=signal(6,0,false,false,40);
    const double tightTreble=signal(0,-8,false,true,2300),plainTreble=signal(0,-8,false,false,2300);
    std::cout<<"Strip EQ: bass="<<bassBoost<<" dB, treble="<<trebleBoost<<" dB; tight ratios="<<tightBass/plainBass<<", "<<tightTreble/plainTreble<<std::endl;
    if(bassBoost<4||bassBoost>6.5||trebleBoost<4||trebleBoost>6.5||tightBass>=plainBass*.97||std::abs(tightTreble-plainTreble)<.001)return false;
    auto dynamic=[](double transient,double comp,bool fa,bool fr,int mode)
    {
        irak::ChannelStripDSP strip;strip.prepare(48000);strip.configure(transient,comp,0,0,fa,fr,false,false);
        double energy=0;
        for(int i=0;i<96000;++i)
        {
            // mode 0: ilk atak; mode 1: yerleşmiş sıkıştırma; mode 2: sessizlikten sonra yeniden giriş.
            const bool silent=mode==2&&i>=48000&&i<50400;
            float l=silent?0.0f:.8f,r=l;strip.processInput(l,r);
            std::array<float,2> x{l,r};strip.processOutput(x);
            if(!std::isfinite(x[0])||x[0]!=x[1])return -1.0;
            if((mode==0&&i<480)||(mode==1&&i>=72000)||(mode==2&&i>=50400&&i<50500))energy+=x[0]*x[0];
        }
        return energy;
    };
    const double soft=dynamic(-1,0,false,false,0),sharp=dynamic(1,0,false,false,0);
    const double dry=dynamic(0,0,false,false,1),compressed=dynamic(0,1,false,false,1);
    const double slowAttack=dynamic(0,1,false,false,0),fastAttack=dynamic(0,1,true,false,0);
    const double slowRelease=dynamic(0,1,false,false,2),fastRelease=dynamic(0,1,false,true,2);
    std::cout<<"Strip dynamics: soft/sharp="<<soft<<"/"<<sharp<<", GR ratio="<<compressed/dry<<", fast ATT ratio="<<fastAttack/slowAttack<<", fast REL ratio="<<fastRelease/slowRelease<<std::endl;
    if(soft<=0||sharp<=soft*1.5||compressed<=0||compressed>=dry*.3||fastAttack>=slowAttack*.9||fastRelease<=slowRelease*1.05)return false;
    // Dört kontrol uçlarda ve anahtarlar çalışırken çoklu örnekleme hızında kararlılık/sessiz kuyruk.
    for(double sr : {44100.,48000.,96000.,192000.})for(int sign : {-1,1})
    {
        irak::ChannelStripDSP strip;strip.prepare(sr);strip.configure(sign,1,sign*12,sign*12,true,true,true,true);
        float tail=0;
        for(int i=0;i<(int)sr*2;++i)
        {
            float l=i<(int)sr/4?(float)(.8*std::sin(i*.1)):0,r=l;
            strip.processInput(l,r);std::array<float,2>x{l,r};strip.processOutput(x);
            if(!std::isfinite(x[0])||std::abs(x[0])>64)return false;
            tail=std::abs(x[0]);
        }
        if(tail>1e-5)return false;
    }
    return true;
}
