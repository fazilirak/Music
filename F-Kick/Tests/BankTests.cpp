// Geliştirici: Fazıl Irak - Son düzenleme: 27.10.2026
// Dosyanın görevi: Ses ve durum davranışını doğrulayan testler.
// Copyright (c) 2026 Fazıl Irak - Özgün F-Kick uygulama kodu.
// SPDX-License-Identifier: AGPL-3.0-or-later
// Fazıl Irak · 128 özgün preset, kategori, seviye, state ve ses çeşitliliği kanıtı.
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "FactoryBank.h"
#include <iostream>
#include <set>
#include <fstream>
namespace {
void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
juce::AudioBuffer<float> phrase(FKickProcessor& p,double sr,int beats,int total,int block=257){
    p.prepareToPlay(sr,block);juce::AudioBuffer<float> result(2,total);result.clear();const int step=juce::roundToInt(sr*60/135);
    for(int offset=0;offset<total;offset+=block){const int n=juce::jmin(block,total-offset);juce::AudioBuffer<float> audio(2,n);juce::MidiBuffer midi;
        for(int k=0;k<beats;++k)if(k*step>=offset&&k*step<offset+n)midi.addEvent(juce::MidiMessage::noteOn(1,60,(juce::uint8)127),k*step-offset);
        p.processBlock(audio,midi);for(int c=0;c<2;++c)result.copyFrom(c,offset,audio,c,0,n);
    }return result;
}
void wave(const juce::File& f,juce::AudioBuffer<float>& a){a.applyGainRamp(a.getNumSamples()-240,240,1,0);auto stream=f.createOutputStream();check(stream!=nullptr,"Bank WAV stream");stream->setPosition(0);stream->truncate();juce::WavAudioFormat fmt;std::unique_ptr<juce::AudioFormatWriter> w(fmt.createWriterFor(stream.get(),48000,2,24,{},0));check(w!=nullptr,"Bank WAV writer");stream.release();check(w->writeFromAudioSampleBuffer(a,0,a.getNumSamples()),"Bank WAV write");}
juce::Component* find(juce::Component& c,const juce::String& id){if(c.getComponentID()==id)return &c;for(auto* child:c.getChildren())if(auto* x=find(*child,id))return x;return nullptr;}
}
void runBankTests(const juce::File& folder){
    FKickProcessor p;check(p.getNumPrograms()==128,"Bank size is not 128");int counts[3]{};std::set<juce::String> names,hashes;
    juce::String report="number,category,name,peak,rms,dc,tail_rms\n";bool headroom=true;
    const auto presets=folder.getChildFile("Presets");check(presets.createDirectory().wasOk(),"Create preset directory");
    // Kategori başına 8 örnekten oluşan kısa dinleme seçkisi; bütün bankanın metrikleri ayrı CSV.
    juce::AudioBuffer<float> previews[3];for(auto& a:previews){a.setSize(2,8*48000*2);a.clear();}int previewCount[3]{};
    for(int index=0;index<128;++index){const auto& entry=fkick::factoryBank()[index];const int category=index<64?0:index<112?1:2;++counts[category];
        check(entry["category"].toString()==(category==0?"General":category==1?"Techno":"Experimental"),"Bank category mismatch");
        check(names.insert(p.getProgramName(index)).second,"Duplicate preset name");p.setCurrentProgram(index);check(p.value("mode")==0&&p.value("sampleGain")==-60,"Factory kick depends on a sample or gate");
        for(double sr:{44100.,96000.,192000.}){auto a=phrase(p,sr,1,int(sr*.22));for(int c=0;c<2;++c)for(int n=0;n<a.getNumSamples();++n)check(std::isfinite(a.getSample(c,n))&&std::abs(a.getSample(c,n))<=1,"Bank rate stability");}
        const auto s=p.settings();const int total=juce::roundToInt(48000*(1.4+s.length+s.release+fkick::TechnoFX::tailSeconds(s.fx,s.motion)));auto audio=phrase(p,48000,3,total);
        double sum=0,dc=0,tail=0;float peak=0;for(int c=0;c<2;++c)for(int n=0;n<total;++n){const float v=audio.getSample(c,n);check(std::isfinite(v),"Bank nonfinite audio");peak=juce::jmax(peak,std::abs(v));sum+=v*v;dc+=v;if(n>=total-4800)tail+=v*v;}
        if(peak<=.05f||peak>=.98f){std::cout<<"HEADROOM "<<index+1<<" "<<p.getProgramName(index)<<" peak="<<peak<<"\n";headroom=false;}check(std::sqrt(tail/9600)<.015,"Bank excessive residual tail");check(std::abs(dc/(2*total))<.015,"Bank DC offset");
        juce::uint64 digest=14695981039346656037ULL;const auto* bytes=reinterpret_cast<const unsigned char*>(audio.getReadPointer(0));for(size_t b=0;b<size_t(total)*sizeof(float);++b){digest^=bytes[b];digest*=1099511628211ULL;}auto hash=juce::String::toHexString((juce::int64)digest);check(hashes.insert(hash).second,"Duplicate rendered kick");
        report+=juce::String(index+1)+","+entry["category"].toString()+",\""+p.getProgramName(index)+"\","+juce::String(peak,6)+","+juce::String(std::sqrt(sum/(2*total)),6)+","+juce::String(dc/(2*total),8)+","+juce::String(std::sqrt(tail/9600),8)+"\n";
        const auto file=presets.getChildFile(juce::String(index+1).paddedLeft('0',3)+" - "+p.getProgramName(index)+".fkick");check(p.savePreset(file),"Save factory preset");
        FKickProcessor restored;check(restored.loadPreset(file)&&restored.getCurrentProgram()==index,"Bank state/program roundtrip");for(auto* param:p.getParameters())if(auto* rp=dynamic_cast<juce::RangedAudioParameter*>(param))check(std::abs(p.value(rp->paramID)-restored.value(rp->paramID))<.001f,"Bank state param mismatch");
        if(previewCount[category]<8&&((category==0&&index%8==0)||(category==1&&(index-64)%6==0)||category==2&&(index-112)%2==0)){
            const int offset=previewCount[category]++*96000;for(int c=0;c<2;++c)previews[category].copyFrom(c,offset,audio,c,0,juce::jmin(96000,total));for(int c=0;c<2;++c)previews[category].applyGainRamp(c,offset+95040,960,1,0);
        }
    }
    check(counts[0]==64&&counts[1]==48&&counts[2]==16,"64/48/16 bank counts");check(folder.getChildFile("PRESET_METRICS.csv").replaceWithText(report),"Write preset metrics");
    check(headroom,"Bank silent or clipped preset; see metrics");
    for(int c=0;c<3;++c)wave(folder.getChildFile(c==0?"F-Kick-General-Selection.wav":c==1?"F-Kick-Techno-Selection.wav":"F-Kick-Experimental-Selection.wav"),previews[c]);
    // Her iki saw aynı genlikte zıt yönlüdür; eski shape=2 hâlâ yükselir.
    check(std::abs(fkick::Motion::wave(2,.25)+.5f)<1e-6&&std::abs(fkick::Motion::wave(5,.25)-.5f)<1e-6,"Saw directions wrong");
    for(int k=0;k<100;++k)check(std::abs(fkick::Motion::wave(2,k/100.)+fkick::Motion::wave(5,k/100.))<1e-6,"Saw inverse mismatch");
    p.setParameter("lfoShape",5);juce::MemoryBlock state;p.getStateInformation(state);FKickProcessor saved;saved.setStateInformation(state.getData(),int(state.getSize()));check(saved.value("lfoShape")==5,"Saw Down state lost");
    auto xml=juce::AudioProcessor::getXmlFromBinary(state.getData(),int(state.getSize()));xml->removeAttribute("bankRevision");xml->setAttribute("program",10);juce::AudioProcessor::copyXmlToBinary(*xml,state);saved.setStateInformation(state.getData(),int(state.getSize()));check(saved.getCurrentProgram()==66,"Legacy bank program migration");
    std::unique_ptr<juce::AudioProcessorEditor> ui(p.createEditor());auto* hp=dynamic_cast<juce::Label*>(find(*ui,"hp-value"));check(hp!=nullptr,"HP text missing");hp->showEditor();hp->getCurrentTextEditor()->setText("1.2k");hp->hideEditor(false);check(std::abs(p.value("hp")-1200)<.01f,"HP typed frequency");hp->showEditor();hp->getCurrentTextEditor()->setText("Off");hp->hideEditor(false);check(p.value("hp")==0,"HP Off parsing");
    auto* punch=dynamic_cast<juce::Label*>(find(*ui,"punch-value"));check(punch!=nullptr,"Punch text missing");punch->showEditor();punch->getCurrentTextEditor()->setText("75%");punch->hideEditor(false);check(std::abs(p.value("punch")-.75f)<.001,"Punch typed value");
    check(dynamic_cast<juce::RangedAudioParameter*>(p.getParameters()[121])->paramID=="punch"&&dynamic_cast<juce::RangedAudioParameter*>(p.getParameters()[122])->paramID=="rumbleGain","Old parameter indices changed");
    auto* selector=dynamic_cast<juce::ComboBox*>(find(*ui,"preset-selector"));check(selector&&selector->getNumItems()==128,"GUI preset count");for(int i=1;i<=128;++i){selector->setSelectedId(i,juce::sendNotificationSync);check(p.getCurrentProgram()==i-1,"GUI preset selection");}
    auto* next=dynamic_cast<juce::TextButton*>(find(*ui,"preset-next"));for(int i=0;i<128;++i){next->onClick();check(p.getCurrentProgram()==i,"Next preset bank traversal");}
    auto* previous=dynamic_cast<juce::TextButton*>(find(*ui,"preset-previous"));for(int i=126;i>=-1;--i){previous->onClick();check(p.getCurrentProgram()==(i+128)%128,"Previous preset bank traversal");}
    check(find(*ui,"lfoShape")==nullptr&&find(*ui,"mod-target")==nullptr,"Extra modulation UI remains");
    std::cout<<"PASS 128 unique kicks / 64-48-16 categories / multi-rate / headroom / DC / tails / 128 portable presets / bank navigation / saw directions / HP text\n";
}
