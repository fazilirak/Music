// SPDX-License-Identifier: GPL-3.0-only
// Fazıl Irak · 03.11.2026: Width, tek VST3 barındırma, A/B ve Zoom regresyonları.
#include "UtilityEffect.h"
#include "Editor.h"
#include "Embedded/SidechainPresets.h"
#include <iostream>
#include <stdexcept>
#include <thread>
#include <atomic>
static void require(bool condition,const char* message){if(!condition)throw std::runtime_error(message);}
static juce::Component* find(juce::Component& c,const juce::String& id){if(c.getComponentID()==id)return &c;for(auto* k:c.getChildren())if(auto* r=find(*k,id))return r;return nullptr;}
static void set(juce::AudioProcessorValueTreeState& s,const char* id,float value){auto* p=s.getParameter(id);require(p!=nullptr,id);p->setValueNotifyingHost(p->convertTo0to1(value));}
static void image(juce::Component& c,const juce::String& name){auto f=juce::File::getCurrentWorkingDirectory().getChildFile("../dist/"+name+".png");f.getParentDirectory().createDirectory();juce::FileOutputStream out(f);out.setPosition(0);out.truncate();require(juce::PNGImageFormat{}.writeImageToStream(c.createComponentSnapshot(c.getLocalBounds()),out),"snapshot");}
int main(int argc,char** argv){try{
    juce::ScopedJuceInitialiser_GUI gui;std::cout.setf(std::ios::unitbuf);std::cerr<<"START utility\n";
    for(float w:{0.f,1.f,2.f}){auto lr=UtilityEffect::widthSample(.6f,-.2f,w);require(std::abs(lr.first+lr.second-.4f)<1e-6f,"Width mono sum");if(w==0)require(std::abs(lr.first-lr.second)<1e-6f,"Width mono");if(w==1)require(std::abs(lr.first-.6f)<1e-6f,"Width identity");}
    for(float width:{0.f,1.f,2.f}){
        UtilityEffect p(false);p.prepareToPlay(48000,256);set(p.state,"minimum",width);set(p.state,"maximum",width);
        juce::AudioBuffer<float> b(4,256);juce::MidiBuffer midi;
        for(int k=0;k<24;++k){b.clear();for(int i=0;i<256;++i){b.setSample(0,i,.6f);b.setSample(1,i,-.2f);}p.processBlock(b,midi);}
        auto expected=UtilityEffect::widthSample(.6f,-.2f,width);require(std::abs(b.getSample(0,255)-expected.first)<1e-5,"Width rendered output");
    }
    for(int preset=0;preset<8;++preset){double floor=0;for(auto point:SidechainPresets::points(preset))floor=juce::jmax(floor,point.y);require(std::abs(floor-1.)<1e-8,"Factory Volume floor Y=0");}
    {ShaperProcessor volume;volume.loadFactoryPreset(0);volume.module(5).setCurrentProgram(2);ShaperEditor ui(volume);ui.setZoom(.75f);image(ui,"preview-volume3-088");}
    std::cerr<<"START rack\n";ShaperProcessor rack;rack.addEffect(8);rack.prepareToPlay(48000,256);
    set(rack.state,"gain",-3.f);rack.switchAB(1);set(rack.state,"gain",-9.f);rack.addEffect(9);
    rack.switchAB(0);require(std::abs(rack.state.getRawParameterValue("gain")->load()+3)<1e-5,"A parameters");require(rack.palette()==std::vector<int>{8},"A rack palette");
    rack.switchAB(1);require(std::abs(rack.state.getRawParameterValue("gain")->load()+9)<1e-5,"B parameters");require(rack.palette()==std::vector<int>({8,9}),"B palette");
    juce::MemoryBlock saved;rack.getStateInformation(saved);ShaperProcessor restored;restored.setStateInformation(saved.getData(),(int)saved.getSize());restored.switchAB(0);require(restored.palette()==std::vector<int>{8},"A/B session persistence");
    {ShaperEditor ui(restored);
        for(int percent:{50,75,100,125,150,175,200}){ui.setZoom(percent/100.f);require(ui.getWidth()==11*percent,"Zoom width");require(std::abs(ui.getHeight()-9.6*percent)<.51,"Zoom height");}
        ui.setZoom(.75f);require(!ui.isResizable(),"Host edge resizing disabled");require(find(ui,"rack-resize-corner")!=nullptr,"Corner handle");
        require(find(ui,"rack-settings-button")&&find(ui,"rack-ab-a")&&find(ui,"rack-ab-b"),"A B gear buttons");image(ui,"preview-width-088");ui.showSettings();require(find(ui,"rack-settings-panel")->isVisible(),"Settings separate window");image(ui,"preview-settings-088");
    }
    // Fazıl Irak · 12.11.2026: parametre bildirimi kuyruğa girdikten sonra A/B eski editörü siler.
    // Her yerleşik efekt, kapanmış editör callback'leri teslim edilirken yaşatılmaz.
    {std::cerr<<"START stress\n";ShaperProcessor stress;for(int e=0;e<ShaperProcessor::effectCount;++e)stress.addEffect(e);
     stress.prepareToPlay(48000,128);
     for(int e=0;e<ShaperProcessor::effectCount;++e){std::cerr<<"Effect "<<e<<"\n";stress.selectedEffect=e;
        ShaperEditor ui(stress);
        for(int turn=0;turn<4;++turn){std::cerr<<" turn "<<turn<<"\n";
            for(auto* p:stress.module(e).getParameters())p->setValueNotifyingHost(p->getValue());
            auto* button=dynamic_cast<juce::Button*>(find(ui,turn%2?"rack-ab-a":"rack-ab-b"));
            require(button!=nullptr,"A/B stress button");button->onClick();
        }
     }
     std::cerr<<"DRAIN notifications\n";juce::MessageManager::callAsync([]{juce::MessageManager::getInstance()->stopDispatchLoop();});
     juce::MessageManager::getInstance()->runDispatchLoop();
     std::atomic<bool> stop{false},finite{true};std::atomic<int> blocks{0};
     std::thread audio([&]{juce::AudioBuffer<float> b(4,128);juce::MidiBuffer m;
         while(!stop.load()){b.clear();b.setSample(0,0,.1f);stress.processBlock(b,m);if(!std::isfinite(b.getMagnitude(0,0,128)))finite=false;++blocks;}});
     for(int i=0;i<30;++i)stress.switchAB(i%2);
     stop=true;audio.join();require(finite&&blocks>0,"Concurrent A/B audio finite");
     std::cout<<"PASS 44 editor A/B switches, queued callbacks and 30 concurrent audio/state switches\n";
    }
    {auto file=juce::File("C:/Program Files/Common Files/VST3/FAZIL IRAK/F-Saturator.vst3");if(file.exists()){
        ShaperProcessor host;host.addEffect(9);host.prepareToPlay(48000,128);
        auto& external=dynamic_cast<UtilityEffect&>(host.module(9));require(external.loadPlugin(file).isEmpty(),"A/B hosted fixture load");
        host.switchAB(1);host.addEffect(8);juce::AudioBuffer<float> b(4,128);juce::MidiBuffer m;
        for(int i=0;i<12;++i){host.switchAB(i%2);b.clear();b.setSample(0,0,.2f);host.processBlock(b,m);require(external.pluginName().containsIgnoreCase("Saturator"),"A/B hosted plugin retained");require(std::isfinite(b.getMagnitude(0,0,128)),"A/B hosted audio finite");}
        std::cout<<"PASS 12 A/B hosted F-Saturator state/audio switches\n";
    }}
    UtilityEffect fx(true);fx.prepareToPlay(48000,256);require(fx.loadPlugin(juce::File("bad.txt")).isNotEmpty(),"Reject non-VST3");
    if(argc>1){
        auto error=fx.loadPlugin(juce::File(juce::String(argv[1])));require(error.isEmpty(),error.toRawUTF8());
        juce::MemoryBlock state;fx.getStateInformation(state);UtilityEffect copy(true);copy.prepareToPlay(48000,256);copy.setStateInformation(state.getData(),(int)state.getSize());require(copy.pluginName()==fx.pluginName(),"Hosted effect recall");
        require(copy.setHostedParameter("Intensity",1.f),"Hosted Intensity parameter");
        juce::AudioBuffer<float> audio(4,256);juce::MidiBuffer midi;
        double errors[2]{};
        for(float mix:{0.f,1.f}){set(copy.state,"minimum",mix);set(copy.state,"maximum",mix);for(int b=0;b<12;++b){audio.clear();for(int i=0;i<256;++i){audio.setSample(0,i,.1f*std::sin(i*.1f));audio.setSample(1,i,.1f*std::cos(i*.1f));}copy.processBlock(audio,midi);}require(std::isfinite(audio.getMagnitude(0,0,256)),"Hosted output finite");
            for(int i=0;i<256;++i)errors[(int)mix]+=std::abs(audio.getSample(0,i)-.1f*std::sin(i*.1f));
        }
        require(errors[0]<1e-4,"FX curve dry endpoint exact");require(errors[1]>1e-3,"FX wet endpoint differs from dry");
        std::cout<<"Hosted VST3 load, audio and state recall: "<<copy.pluginName()<<"\n";
    }
    if(argc>1)require(dynamic_cast<UtilityEffect&>(rack.module(9)).loadPlugin(juce::File(juce::String(argv[1]))).isEmpty(),"FX screenshot plugin");
    rack.selectedEffect=9;{ShaperEditor ui(rack);ui.setZoom(.75f);image(ui,"preview-fx-088");}
    // Fazıl Irak · 03.11.2026: yeni instance pencere boyutunu hatırlar; About sekmeleri ortak düzendedir.
    {ShaperProcessor p;{ShaperEditor ui(p);ui.setZoom(1.25f);}}
    {ShaperProcessor p;require(p.editorWidth.load()==1375&&p.editorHeight.load()==1200,"Persistent editor dimensions");
     ShaperEditor ui(p);require(ui.getWidth()==1375&&ui.getHeight()==1200,"Reopened editor dimensions");ui.setZoom(.75f);}
    {AboutPanel page;page.setSize(1100,960);for(int tab:{7,8,9}) {
        page.tabs.setCurrentTabIndex(tab);image(page,"preview-about-"+juce::String(tab)+"-088");}}
    std::cout<<"PASS Width mono/identity/wide, A/B state recall, all seven zooms, separate settings and FX\n";return 0;
}catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<"\n";return 1;}}
