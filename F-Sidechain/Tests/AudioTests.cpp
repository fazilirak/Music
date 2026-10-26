// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 Fazıl Irak
// Fazıl Irak: Ses, bant/preset geri çağırma, logo erişimi ve gömülü lisans metinlerini doğrular.
#include "MultibandProcessor.h"
#include "Volume/PluginProcessor.h"
#include <iostream>
#include <sstream>
#include <limits>
juce::Component* findComponent(juce::Component& c,const juce::String& id) {
    if(c.getComponentID()==id)return &c;
    for(auto* child:c.getChildren())if(auto* result=findComponent(*child,id))return result;
    return nullptr;
}
void snapshot(juce::Component& c,const char* filename) {
    auto image=c.createComponentSnapshot(c.getLocalBounds());
    juce::FileOutputStream stream(juce::File::getCurrentWorkingDirectory().getChildFile(filename));
    stream.setPosition(0);stream.truncate();juce::PNGImageFormat{}.writeImageToStream(image,stream);
}
std::unique_ptr<MultibandProcessor> make() {return std::make_unique<MultibandProcessor>([]{return new volumef::VOLUMEFAudioProcessor();},5);}
void require(bool v,const char* msg) {if(!v) throw std::runtime_error(msg);}
// Fazıl Irak — 27.10.2026: XML'nin bayt dizilimi yerine tüm kayıt alanlarını doğrula.
// Normalleştirilmiş float parametreleri farklı işlemcilerde birkaç ULP değişebilir;
// eksik alan, farklı metin, farklı nota/eğri veya anlamlı sayısal değişiklik yine hatadır.
bool equivalentValue(const juce::String& a,const juce::String& b) {
    if(a==b)return true;
    std::istringstream sa(a.toStdString()),sb(b.toStdString());
    sa.imbue(std::locale::classic());sb.imbue(std::locale::classic());
    double va=0,vb=0;bool any=false;
    for(;;) {
        sa>>std::ws;sb>>std::ws;
        if(sa.eof()||sb.eof())return any&&sa.eof()&&sb.eof();
        if(!(sa>>va)||!(sb>>vb)||!std::isfinite(va)||!std::isfinite(vb))return false;
        if(std::abs(va-vb)>8.0*std::numeric_limits<float>::epsilon()*std::max({1.0,std::abs(va),std::abs(vb)}))return false;
        any=true;
    }
}
bool equivalentXml(const juce::XmlElement& a,const juce::XmlElement& b) {
    if(a.getTagName()!=b.getTagName()||a.getNumAttributes()!=b.getNumAttributes()||a.getNumChildElements()!=b.getNumChildElements())return false;
    for(int i=0;i<a.getNumAttributes();++i) {
        const auto name=a.getAttributeName(i);
        if(!b.hasAttribute(name)||!equivalentValue(a.getAttributeValue(i),b.getStringAttribute(name))) {
            std::cerr<<"State differs: "<<a.getTagName()<<" / "<<name<<": "<<a.getAttributeValue(i)<<" -> "<<b.getStringAttribute(name)<<'\n';return false;
        }
    }
    if(a.hasTagName("Band")) {
        juce::MemoryBlock ba,bb;
        if(!ba.fromBase64Encoding(a.getAllSubText())||!bb.fromBase64Encoding(b.getAllSubText()))return false;
        auto xa=juce::AudioProcessor::getXmlFromBinary(ba.getData(),(int)ba.getSize());
        auto xb=juce::AudioProcessor::getXmlFromBinary(bb.getData(),(int)bb.getSize());
        return xa&&xb&&equivalentXml(*xa,*xb);
    }
    if(a.isTextElement()&&!equivalentValue(a.getText(),b.getText()))return false;
    for(int i=0;i<a.getNumChildElements();++i)if(!equivalentXml(*a.getChildElement(i),*b.getChildElement(i)))return false;
    return true;
}
void set(juce::AudioProcessor& p,const char* id,float v) {
    for(auto* param:p.getParameters())if(auto* r=dynamic_cast<juce::RangedAudioParameter*>(param))if(r->paramID==id){r->setValueNotifyingHost(r->convertTo0to1(v));return;}
    throw std::runtime_error(id);
}
int main(int argc,char** argv) {
    juce::ScopedJuceInitialiser_GUI gui;
    try {
        auto p=make();
        require(!p->isMultiband() && p->editorBand()==1,"New instances must process the full range through Mid");
        p->setRateAndBufferSizeDetails(48000,512);p->prepareToPlay(48000,512);
        juce::AudioBuffer<float> a(2,512);juce::MidiBuffer midi;
        for(int preset=0;preset<p->getNumPrograms();++preset) {
            p->setCurrentProgram(preset);
            auto& engine=static_cast<volumef::VOLUMEFAudioProcessor&>(p->band(1));
            double floor=0;
            for(const auto& pt:engine.viewPattern->points)floor=juce::jmax(floor,pt.y);
            require(std::abs(floor-1.0)<1.0e-9,"Every factory envelope must reach visual floor / zero gain");
            for(int block=0;block<20;++block) {
                for(int c=0;c<2;++c)for(int i=0;i<512;++i)a.setSample(c,i,0.5f*std::sin((block*512+i)*0.06f));
                p->processBlock(a,midi);
                for(int c=0;c<2;++c)for(int i=0;i<512;++i)require(std::isfinite(a.getSample(c,i)),"Nonfinite audio");
            }
        }
        set(p->band(1),"mix",0);
        for(int b=0;b<20;++b){for(int c=0;c<2;++c)for(int i=0;i<512;++i)a.setSample(c,i,0.25f);p->processBlock(a,midi);}
        require(std::abs(a.getSample(0,400)-0.25f)<0.0001f,"Dry mix must preserve full-range signal");
        set(p->band(1),"mix",1); set(p->band(1),"min",0);set(p->band(1),"max",0);
        for(int b=0;b<20;++b){a.clear();for(int c=0;c<2;++c)for(int i=0;i<512;++i)a.setSample(c,i,0.25f);p->processBlock(a,midi);}
        require(a.getMagnitude(0,512)<0.001f,"Zero envelope range must mute wet signal");
        set(*p,"crossover_low",180);set(*p,"crossover_high",4000);p->selectedBand=0;p->setCurrentProgram(3);
        p->selectedBand=2;p->setCurrentProgram(6);
        juce::MemoryBlock state;p->getStateInformation(state);auto restored=make();restored->setStateInformation(state.getData(),(int)state.getSize());
        juce::MemoryBlock roundtrip;restored->getStateInformation(roundtrip);
        auto stateXml=juce::AudioProcessor::getXmlFromBinary(state.getData(),(int)state.getSize());
        auto roundtripXml=juce::AudioProcessor::getXmlFromBinary(roundtrip.getData(),(int)roundtrip.getSize());
        if(state!=roundtrip)std::cout<<"State encoding differs; verifying every decoded field with an eight-float-ULP numeric tolerance\n";
        require(stateXml&&roundtripXml&&equivalentXml(*stateXml,*roundtripXml),"Complete envelope/pattern state must roundtrip semantically");
        require(restored->hasLowBand() && restored->hasHighBand(),"Band edges did not restore");
        require(restored->band(0).getCurrentProgram()==3 && restored->band(2).getCurrentProgram()==6,"Independent band presets did not restore");
        restored->setRateAndBufferSizeDetails(44100,512);restored->prepareToPlay(44100,512);
        for(int i=0;i<20;++i){for(int c=0;c<2;++c)for(int j=0;j<512;++j)a.setSample(c,j,0.1f*std::sin((i*512+j)*0.1f));restored->processBlock(a,midi);require(std::isfinite(a.getMagnitude(0,512)),"Multiband invalid audio");}
        set(restored->band(0),"trigger",2);
        restored->processBlock(a,midi);
        require(restored->getLatencySamples()==restored->band(0).getLatencySamples(),"Host latency must follow active Audio trigger band");
        restored->resetBands();
        require(!restored->isMultiband() && restored->editorBand()==1,"Full range reset must remove both crossovers and select Mid");
        auto view=make();std::unique_ptr<juce::AudioProcessorEditor> editor(view->createEditor());
        auto* graph=findComponent(*editor,"band-frequency-graph");
        require(graph && graph->isVisible(),"Band graph must always be visible");
        for(auto* c:editor->getChildren()) {
            if(auto* b=dynamic_cast<juce::TextButton*>(c))require(b->getButtonText()!="Bands","Band toggle must be removed");
            if(auto* box=dynamic_cast<juce::ComboBox*>(c))require(!box->getText().contains("Pattern"),"Pattern selector must be removed");
        }
        if(argc>1)snapshot(*editor,"preview.png");
        require(findComponent(*editor,"about-button")==nullptr,"Separate About button must be removed");
        auto* aboutLogo=dynamic_cast<juce::Button*>(findComponent(*editor,"about-logo"));
        require(aboutLogo!=nullptr,"Clickable F-Sidechain logo missing");aboutLogo->onClick();
        auto* panel=findComponent(*editor,"about-panel");require(panel && panel->isVisible(),"About panel must open");
        auto* documents=dynamic_cast<juce::ComboBox*>(findComponent(*panel,"license-selector"));
        auto* license=dynamic_cast<juce::TextEditor*>(findComponent(*panel,"license-text"));
        require(documents && license,"Embedded licence controls missing");
        require(documents->getItemText(0)=="Licence overview","About labels must be English");
        for(int i=1;i<=7;++i) {
            documents->setSelectedId(i,juce::sendNotificationSync);
            require(license->getText().length()>200 && !license->getText().contains("could not be loaded"),"Embedded licence resource missing");
        }
        documents->setSelectedId(1,juce::sendNotificationSync);
        if(argc>1)snapshot(*editor,"preview-about.png");
        std::cout<<"PASS: eight deeper presets, finite audio, dry identity, wet mute, independent band/state recall, 44.1/48 kHz, active-band latency, permanent band UI, removed pattern selector, seven embedded licence documents\n";
        return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
