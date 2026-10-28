// Geliştirici: Fazıl Irak - 28.10.2026
// SPDX-License-Identifier: AGPL-3.0-or-later
// Görev: Temiz fabrika gövdesini ve gerçek render üzerinden harmonik bozulmayı doğrulamak.
#include "PluginProcessor.h"
#include "FactoryBank.h"
#include <iostream>
namespace {
void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
double harmonicRatio(bool saturated) {
    FKickProcessor p;p.setCurrentProgram(0);
    p.setParameter("root",100);p.setParameter("sweep",0);p.setParameter("length",1200);
    p.setParameter("attack",0);p.setParameter("bodyGain",-6);p.setParameter("output",-12);
    p.setParameter("harmonics",0);p.setParameter("punch",saturated?.65f:0);p.setParameter("drive",0);
    fkick::Curve flat;flat.count=2;flat.x[0]=0;flat.x[1]=1;flat.y[0]=flat.y[1]=1;
    p.setCurve(0,flat);flat.y[0]=flat.y[1]=0;p.setCurve(1,flat);
    p.prepareToPlay(48000,257);juce::AudioBuffer<float> out(2,48000);out.clear();
    for(int offset=0;offset<48000;offset+=257){const int n=juce::jmin(257,48000-offset);juce::AudioBuffer<float> a(2,n);juce::MidiBuffer midi;if(offset==0)midi.addEvent(juce::MidiMessage::noteOn(1,60,(juce::uint8)127),0);p.processBlock(a,midi);out.copyFrom(0,offset,a,0,0,n);}
    double fundamental=0,harmonics=0;
    // 0.5–0.9 s: transient ve DC oturması sonrası tam 40 periyot; pencere sızıntısı yok.
    for(int harmonic=1;harmonic<=10;++harmonic){double real=0,imaginary=0;for(int n=24000;n<43200;++n){const double phase=juce::MathConstants<double>::twoPi*100*harmonic*n/48000.;real+=out.getSample(0,n)*std::cos(phase);imaginary+=out.getSample(0,n)*std::sin(phase);}const double energy=real*real+imaginary*imaginary;if(harmonic==1)fundamental=energy;else harmonics+=energy;}
    require(fundamental>1,"Clean oscillator is silent");return std::sqrt(harmonics/fundamental);
}
}
void runCleanTests(const juce::File& folder) {
    for(int i=0;i<112;++i){FKickProcessor p;p.setCurrentProgram(i);for(const char* id:{"punch","drive","rumbleDrive","lfo_drive","ad_drive","lfo_rumbleDrive","ad_rumbleDrive"})require(p.value(id)==0,"Clean factory preset contains a saturation stage or hidden drive modulation");}
    const double clean=harmonicRatio(false),saturated=harmonicRatio(true);
    require(clean<.00031622777,"Clean sine path exceeds -70 dB harmonic distortion");
    require(saturated>.01&&saturated>clean*100,"Harmonic measurement did not detect the previous Punch saturation");
    const auto report="Clean sine harmonic distortion: "+juce::String(20*std::log10(juce::jmax(1e-15,clean)),2)+" dB\nPrevious 65% Punch comparison: "+juce::String(20*std::log10(saturated),2)+" dB\n112 General/Techno presets: Punch, Drive, Rumble Drive and hidden drive modulation are zero.\n";
    require(folder.getChildFile("CLEAN_METRICS.txt").replaceWithText(report),"Write clean metrics");std::cout<<report;
}

// Fazıl Irak: Kalibrasyon testi atlamaz; yalnız yeni gain tablosunu ölçer.
// Bu modun çıktısı uygulandıktan sonra normal CTest yeniden geçmelidir.
void calibrateCleanBank(const juce::File& folder) {
    auto render=[](FKickProcessor& p,int beats,int total,double bpm){p.prepareToPlay(48000,257);juce::AudioBuffer<float> out(2,total);out.clear();const int step=juce::roundToInt(48000*60/bpm);for(int offset=0;offset<total;offset+=257){const int n=juce::jmin(257,total-offset);juce::AudioBuffer<float> a(2,n);juce::MidiBuffer midi;for(int b=0;b<beats;++b)if(b*step>=offset&&b*step<offset+n)midi.addEvent(juce::MidiMessage::noteOn(1,60,(juce::uint8)127),b*step-offset);p.processBlock(a,midi);for(int c=0;c<2;++c)out.copyFrom(c,offset,a,c,0,n);}return out;};
    auto rms=[](const juce::AudioBuffer<float>& a,int offset,int count){double energy=0;for(int c=0;c<2;++c)for(int n=offset;n<offset+count;++n)energy+=double(a.getSample(c,n))*a.getSample(c,n);return std::sqrt(energy/(2*count));};
    auto* table=new juce::DynamicObject;
    for(int index=0;index<112;++index){
        FKickProcessor full,zero;full.setCurrentProgram(index);zero.setCurrentProgram(index);
        double gain=full.value("rumbleGain");
        if(full.value("rumbleMix")>0){
            for(auto* p:{&full,&zero}){p->setParameter("output",-24);p->setParameter("echoMix",0);}
            zero.setParameter("rumbleMix",0);zero.setParameter("lfo_rumbleMix",0);zero.setParameter("ad_rumbleMix",0);
            auto wet=render(full,4,192000,120),dry=render(zero,4,192000,120);
            for(int c=0;c<2;++c)for(int n=0;n<192000;++n)wet.getWritePointer(c)[n]-=dry.getSample(c,n);
            const auto recipe=fkick::factoryBank()[index]["recipe"].toString();
            const double target=index==64?.75:index==68?.45:recipe=="Rumble"||recipe=="Hypnotic"?.55:recipe=="Minimal"?.2:.35;
            const double ratio=rms(wet,79200,12000)/juce::jmax(1e-9,rms(dry,72960,4800));
            gain=juce::jlimit(-24.,24.,gain+20*std::log10(target/juce::jmax(1e-9,ratio)));
        }
        FKickProcessor peakProbe;peakProbe.setCurrentProgram(index);peakProbe.setParameter("rumbleGain",float(gain));peakProbe.setParameter("output",-24);
        const auto settings=peakProbe.settings();const int total=juce::roundToInt(48000*(1.4+settings.length+settings.release+fkick::TechnoFX::tailSeconds(settings.fx,settings.motion)));
        const auto audio=render(peakProbe,3,total,135);const double peak=audio.getMagnitude(0,audio.getNumSamples());
        auto* values=new juce::DynamicObject;values->setProperty("output",juce::jlimit(-24.,6.,-24+20*std::log10(.5/juce::jmax(1e-9,peak))));values->setProperty("rumbleGain",gain);
        table->setProperty(peakProbe.getProgramName(index),juce::var(values));
    }
    require(folder.getChildFile("CLEAN_CALIBRATION.json").replaceWithText(juce::JSON::toString(juce::var(table))),"Write calibration table");
    std::cout<<"Calibration table written for 112 presets. This is not a test pass; apply it and run CTest.\n";
}
