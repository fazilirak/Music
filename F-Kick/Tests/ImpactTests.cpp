// Geliştirici: Fazıl Irak - Son düzenleme: 27.10.2026
// Dosyanın görevi: Ses ve durum davranışını doğrulayan testler.
// Copyright (c) 2026 Fazıl Irak - Özgün F-Kick uygulama kodu.
// SPDX-License-Identifier: AGPL-3.0-or-later
// Fazıl Irak · Peak eşit kuru kick ve izole rumble dönüşü; 0.4 sesinin korunması.
#include "PluginProcessor.h"
#include "FactoryBank.h"
#include <iostream>
namespace {
void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
juce::AudioBuffer<float> render(FKickProcessor& p,int beats=1,int note=60){
    p.prepareToPlay(48000,257);juce::AudioBuffer<float> out(2,beats*24000+96000);out.clear();
    for(int offset=0;offset<out.getNumSamples();offset+=257){const int n=juce::jmin(257,out.getNumSamples()-offset);juce::AudioBuffer<float> a(2,n);juce::MidiBuffer midi;
        for(int b=0;b<beats;++b)if(b*24000>=offset&&b*24000<offset+n)midi.addEvent(juce::MidiMessage::noteOn(1,note,(juce::uint8)127),b*24000-offset);
        p.processBlock(a,midi);for(int c=0;c<2;++c)out.copyFrom(c,offset,a,c,0,n);
    }return out;
}
double rms(const juce::AudioBuffer<float>& a,int start,int count){double x=0;for(int c=0;c<2;++c)for(int i=start;i<start+count;++i)x+=double(a.getSample(c,i))*a.getSample(c,i);return std::sqrt(x/(2*count));}
double peak(const juce::AudioBuffer<float>& a){return juce::jmax(a.getMagnitude(0,0,a.getNumSamples()),a.getMagnitude(1,0,a.getNumSamples()));}
void save(const juce::File& f,juce::AudioBuffer<float>& a){a.applyGainRamp(a.getNumSamples()-240,240,1,0);auto out=f.createOutputStream();check(out!=nullptr,"Impact WAV stream");out->setPosition(0);out->truncate();juce::WavAudioFormat fmt;std::unique_ptr<juce::AudioFormatWriter> w(fmt.createWriterFor(out.get(),48000,2,24,{},0));check(w!=nullptr,"Impact WAV writer");out.release();check(w->writeFromAudioSampleBuffer(a,0,a.getNumSamples()),"Impact WAV write");}
}
void runImpactTests(const juce::File& folder){
    const juce::File fixtures(FKICK_TEST_FIXTURES);FKickProcessor legacy,now;
    FKickProcessor oldHouse;check(oldHouse.loadPreset(fixtures.getChildFile("Legacy-0.4-Deep-House.fkick")),"Legacy golden fixture");const auto legacyWav=folder.getChildFile("F-Kick-Legacy-Deep-House.wav");check(oldHouse.exportWav(legacyWav),"Legacy export");juce::MemoryBlock golden,actual;check(fixtures.getChildFile("Legacy-0.4-Deep-House.wav").loadFileAsData(golden)&&legacyWav.loadFileAsData(actual)&&golden==actual,"Legacy sound changed in new engine");
    check(legacy.loadPreset(fixtures.getChildFile("Legacy-0.4-Solid.fkick")),"Legacy kick fixture");check(legacy.value("punch")==0&&legacy.value("rumbleGain")==0,"Legacy neutral impact migration");now.setCurrentProgram(0);
    check(now.value("tracking")==0,"Factory kick must preserve its root at low MIDI notes");auto c1=render(now,1,36),c3=render(now,1,60);float keyDiff=0;for(int c=0;c<2;++c)for(int i=0;i<c1.getNumSamples();++i)keyDiff=juce::jmax(keyDiff,std::abs(c1.getSample(c,i)-c3.getSample(c,i)));check(keyDiff<1e-7,"Kick root changes between MIDI 36 and 60");
    auto oldC1=render(legacy,4,36),newC1=render(now,4,36);save(folder.getChildFile("F-Kick-C1-0.4.wav"),oldC1);save(folder.getChildFile("F-Kick-C1-0.5.wav"),newC1);
    auto before=render(legacy,4),after=render(now,4);before.applyGain(float(.5/peak(before)));after.applyGain(float(.5/peak(after)));
    const double oldBody=rms(before,960,6240),newBody=rms(after,960,6240);
    save(folder.getChildFile("F-Kick-Dry-0.4-EqualPeak.wav"),before);save(folder.getChildFile("F-Kick-Dry-0.5-EqualPeak.wav"),after);
    FKickProcessor base,boost,dry;for(auto* p:{&base,&boost,&dry}){check(p->loadPreset(fixtures.getChildFile("Legacy-0.4-Rumble.fkick")),"Legacy rumble fixture");p->setParameter("output",-18);p->setParameter("echoMix",0);}
    boost.setParameter("rumbleGain",12);dry.setParameter("rumbleMix",0);
    auto oldWet=render(base,4),newWet=render(boost,4),dryAudio=render(dry,4);
    for(int c=0;c<2;++c)for(int i=0;i<oldWet.getNumSamples();++i){oldWet.getWritePointer(c)[i]-=dryAudio.getSample(c,i);newWet.getWritePointer(c)[i]-=dryAudio.getSample(c,i);}
    const double oldR=rms(oldWet,4800,14400),newR=rms(newWet,4800,14400);
    check(newR>oldR*3.9&&newR<oldR*4.1,"Rumble return gain is not an independent +12 dB stage");
    now.setCurrentProgram(64);auto rumbleNew=render(now,4);FKickProcessor oldRumble;check(oldRumble.loadPreset(fixtures.getChildFile("Legacy-0.4-Rumble.fkick")),"Legacy rumble demo");auto rumbleOld=render(oldRumble,4);
    save(folder.getChildFile("F-Kick-Rumble-0.4.wav"),rumbleOld);save(folder.getChildFile("F-Kick-Rumble-0.5.wav"),rumbleNew);
    // Islak dönüşün kuru kick'e göre vuruşlar arasındaki enerjisini ölç.
    FKickProcessor onlyDry;onlyDry.setCurrentProgram(64);onlyDry.setParameter("rumbleMix",0);onlyDry.setParameter("echoMix",0);auto d=render(onlyDry,4);
    now.setParameter("echoMix",0);auto r=render(now,4);for(int c=0;c<2;++c)for(int i=0;i<r.getNumSamples();++i)r.getWritePointer(c)[i]-=d.getSample(c,i);
    const double ratio=rms(r,72000+7200,12000)/rms(d,72000+960,4800);
    const double oldRatio=rms(oldWet,79200,12000)/rms(dryAudio,72960,4800);
    const auto result="Equal-peak dry 20-150 ms RMS: old="+juce::String(oldBody,6)+" new="+juce::String(newBody,6)+" change="+juce::String(20*std::log10(newBody/oldBody),2)+" dB\nIsolated rumble +12 dB stage: "+juce::String(20*std::log10(newR/oldR),2)+" dB\nOld Deep Rumble wet-between / dry-hit RMS: "+juce::String(oldRatio,4)+"\nNew Deep Rumble wet-between / dry-hit RMS: "+juce::String(ratio,4)+" ("+juce::String(20*std::log10(ratio),2)+" dB)\n";
    check(folder.getChildFile("IMPACT_METRICS.txt").replaceWithText(result),"Impact report");std::cout<<result;
    FKickProcessor oldBasement,newBasement;check(oldBasement.loadPreset(fixtures.getChildFile("Legacy-0.4-Basement.fkick")),"069 reference fixture");newBasement.setCurrentProgram(68);
    auto old069=render(oldBasement,4),new069=render(newBasement,4);old069.applyGain(float(.65/peak(old069)));new069.applyGain(float(.65/peak(new069)));
    save(folder.getChildFile("F-Kick-069-Basement-0.4-EqualPeak.wav"),old069);save(folder.getChildFile("F-Kick-069-Basement-0.5-EqualPeak.wav"),new069);
    for(auto* p:{&oldBasement,&newBasement}){p->setParameter("rumbleMix",0);p->setParameter("echoMix",0);}
    auto oldDry=render(oldBasement,4),newDry=render(newBasement,4);oldDry.applyGain(float(.5/peak(oldDry)));newDry.applyGain(float(.5/peak(newDry)));
    const double old069Rms=rms(oldDry,960,6240),new069Rms=rms(newDry,960,6240);
    const auto basementResult="069 Basement equal-peak dry 20-150 ms RMS: old="+juce::String(old069Rms,6)+" new="+juce::String(new069Rms,6)+" change="+juce::String(20*std::log10(new069Rms/old069Rms),2)+" dB\n";
    check(folder.getChildFile("IMPACT_METRICS.txt").appendText(basementResult),"069 report");std::cout<<basementResult;check(new069Rms>old069Rms*1.3,"069 body density did not improve at equal peak");
    newBasement.setParameter("rumbleMix",float(fkick::factoryBank()[68]["values"]["rumbleMix"]));newBasement.setParameter("output",-24);FKickProcessor basementDry;basementDry.setCurrentProgram(68);basementDry.setParameter("rumbleMix",0);basementDry.setParameter("output",-24);auto mixed=render(newBasement,4),bd=render(basementDry,4);for(int c=0;c<2;++c)for(int i=0;i<mixed.getNumSamples();++i)mixed.getWritePointer(c)[i]-=bd.getSample(c,i);const double basementRatio=rms(mixed,79200,12000)/rms(bd,72960,4800);check(basementRatio>.35&&basementRatio<.6,"069 rumble masks the dry body or is too quiet");
    save(folder.getChildFile("F-Kick-069-Dry-0.4-EqualPeak.wav"),oldDry);save(folder.getChildFile("F-Kick-069-Dry-0.5-EqualPeak.wav"),newDry);
    juce::String balance="name,ratio,target,suggested_gain\n";
    for(int i=64;i<128;++i){FKickProcessor full,zero;full.setCurrentProgram(i);zero.setCurrentProgram(i);if(full.value("rumbleMix")==0)continue;
        for(auto* p:{&full,&zero}){p->setParameter("output",-24);p->setParameter("echoMix",0);}zero.setParameter("rumbleMix",0);zero.setParameter("lfo_rumbleMix",0);zero.setParameter("ad_rumbleMix",0);
        auto wet=render(full,4),clean=render(zero,4);for(int c=0;c<2;++c)for(int n=0;n<wet.getNumSamples();++n)wet.getWritePointer(c)[n]-=clean.getSample(c,n);
        const double measured=rms(wet,79200,12000)/juce::jmax(1e-9,rms(clean,72960,4800));const auto recipe=fkick::factoryBank()[i]["recipe"].toString();
        const double target=i==64?.75:i==68?.45:recipe=="Rumble"||recipe=="Hypnotic"?.55:recipe=="Minimal"?.2:i>=112?.4:.35;
        const double gain=juce::jlimit(-24.,24.,full.value("rumbleGain")+20*std::log10(target/juce::jmax(1e-6,measured)));
        balance+="\""+full.getProgramName(i)+"\","+juce::String(measured,6)+","+juce::String(target,3)+","+juce::String(gain,2)+"\n";
    }
    check(folder.getChildFile("RUMBLE_BALANCE.csv").replaceWithText(balance),"Rumble balance report");
    std::cout<<"PASS legacy impact defaults / independent rumble gain / equal-peak before-after renders\n";
}

