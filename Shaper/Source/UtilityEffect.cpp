// SPDX-License-Identifier: GPL-3.0-only
// Fazıl Irak · 03.11.2026
// Dosya yükleme/UI mesaj iş parçacığında; audio callback yalnız önceden ayrılmış tamponları işler.
#include "UtilityEffect.h"
#include "Embedded/RackEditorCommands.h"
#include "Embedded/RackKnob.h"
namespace widthcurve {
juce::AudioProcessor* createEmbeddedProcessor();
void forceCV(juce::AudioProcessor&);
void shape(juce::AudioProcessor&,int,float);
bool audioSettingsShown(juce::AudioProcessor&);
}
namespace fxcurve {
juce::AudioProcessor* createEmbeddedProcessor();
void forceCV(juce::AudioProcessor&);
void shape(juce::AudioProcessor&,int,float);
bool audioSettingsShown(juce::AudioProcessor&);
}
namespace {
class HostedWindow final : public juce::DocumentWindow {
public:
    HostedWindow(juce::AudioPluginInstance& p):DocumentWindow(p.getName(),juce::Colour(0xff141820),closeButton){
        setUsingNativeTitleBar(true);setContentOwned(p.createEditorIfNeeded(),true);
        centreWithSize(getWidth(),getHeight());setVisible(true);
    }
    void closeButtonPressed() override{setVisible(false);}
};
class UtilityEditor final : public juce::AudioProcessorEditor, public RackEditorCommands,private juce::Timer {
    UtilityEffect& owner;
    std::unique_ptr<juce::AudioProcessorEditor> curveEditor;
    struct Settings : juce::Component {void paint(juce::Graphics& g) override {g.fillAll(juce::Colour(0xff1c2027));}} settings;
    std::unique_ptr<shaperui::RackKnob> low,high;
    juce::TextButton load{"Load VST3"},edit{"Edit plugin"};
    juce::Label info,fxTitle,mixLegend;
    Settings sidebar;
    std::unique_ptr<juce::FileChooser> chooser;
    int dock=316;
    RackEditorCommands* commands() const{return dynamic_cast<RackEditorCommands*>(curveEditor.get());}
    void timerCallback() override {
        settings.setVisible(!(owner.external?fxcurve::audioSettingsShown(*owner.curve):widthcurve::audioSettingsShown(*owner.curve)));
        if(owner.external){info.setText(owner.pluginName(),juce::dontSendNotification);}
    }
public:
    UtilityEditor(UtilityEffect& p):AudioProcessorEditor(p),owner(p) {
        curveEditor.reset(p.curve->createEditor());addAndMakeVisible(*curveEditor);addAndMakeVisible(settings);
        auto colour=juce::Colour(p.external?0xffe377a6:0xff87c9ff);
        low=std::make_unique<shaperui::RackKnob>(p.state,"minimum",p.external?"Min Mix":"Min width",colour);
        high=std::make_unique<shaperui::RackKnob>(p.state,"maximum",p.external?"Max Mix":"Max width",colour);
        for(auto* k:{low.get(),high.get()}) {k->format=[](float v){return juce::String(juce::roundToInt(v*100))+" %";};settings.addAndMakeVisible(*k);}
        settings.addAndMakeVisible(info);info.setColour(juce::Label::textColourId,juce::Colour(0xffd7dde5));
        if(p.external){
            addAndMakeVisible(sidebar);sidebar.addAndMakeVisible(load);sidebar.addAndMakeVisible(edit);
            sidebar.addAndMakeVisible(info);sidebar.addAndMakeVisible(fxTitle);
            fxTitle.setText("FX / VST3 EFFECT",juce::dontSendNotification);
            // Fazıl Irak · 03.11.2026: tüm etkin kontroller rack kartının pembe rengini izler.
            for(auto* button:{&load,&edit}) {
                button->setColour(juce::TextButton::textColourOffId,colour);
                button->setColour(juce::TextButton::buttonColourId,colour.withAlpha(.12f));
                button->setColour(juce::TextButton::buttonOnColourId,colour);
            }
            fxTitle.setColour(juce::Label::textColourId,colour);
            mixLegend.setColour(juce::Label::textColourId,colour);
            info.setText(p.pluginName(),juce::dontSendNotification);info.setJustificationType(juce::Justification::topLeft);
            addAndMakeVisible(mixLegend);mixLegend.setText("CURVE MIX\nDry to wet",juce::dontSendNotification);
            mixLegend.setJustificationType(juce::Justification::centred);mixLegend.setOpaque(true);
            mixLegend.setColour(juce::Label::backgroundColourId,juce::Colour(0xff1c2027));
            load.onClick=[this]{
                chooser=std::make_unique<juce::FileChooser>("Select an installed VST3 effect",juce::File::getSpecialLocation(juce::File::commonApplicationDataDirectory).getChildFile("VST3"),"*.vst3");
                juce::Component::SafePointer<UtilityEditor> safe(this);
                chooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles|juce::FileBrowserComponent::canSelectDirectories,[safe](const juce::FileChooser& c){
                    if(safe&&c.getResult().exists()){auto error=safe->owner.loadPlugin(c.getResult());if(error.isNotEmpty())juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,"Could not load VST3",error);safe->info.setText(error.isEmpty()?safe->owner.pluginName():error,juce::dontSendNotification);}
                });
            };
            edit.onClick=[this]{owner.showPluginEditor();};
        }else info.setText("0% Mono  /  100% Original stereo  /  200% Wide",juce::dontSendNotification);
        setSize(1060,700);startTimerHz(5);
    }
    ~UtilityEditor() override{stopTimer();owner.closePluginEditor();}
    void resized() override {
        curveEditor->setBounds(getLocalBounds());if(commands())commands()->rackDockHeader(dock);
        settings.setBounds(dock,168,getWidth()-dock-8,124);settings.toFront(false);
        low->setBounds(12,8,68,64);high->setBounds(88,8,68,64);
        load.setBounds(178,16,130,28);edit.setBounds(320,16,130,28);
        info.setBounds(12,82,settings.getWidth()-24,32);
        if(owner.external){
            sidebar.setBounds(0,0,dock-12,292);sidebar.toFront(false);
            fxTitle.setBounds(16,16,sidebar.getWidth()-32,28);
            info.setBounds(16,56,sidebar.getWidth()-32,72);
            load.setBounds(16,150,sidebar.getWidth()-32,30);edit.setBounds(16,192,sidebar.getWidth()-32,30);
            mixLegend.setBounds(getWidth()-184,2,180,76);mixLegend.toFront(false);
        }
    }
    juce::PopupMenu rackOptions() override{return commands()?commands()->rackOptions():juce::PopupMenu();}
    void rackCommand(int c) override{if(commands())commands()->rackCommand(c);}
    bool rackCanEdit() const override{return commands()&&commands()->rackCanEdit();}
    bool rackIsSequencing() const override{return commands()&&commands()->rackIsSequencing();}
    void rackDockHeader(int x) override{dock=x;resized();}
    int rackHeaderHeight() const override{return 330;}
};
}
juce::AudioProcessorValueTreeState::ParameterLayout UtilityEffect::layout(bool fx){
    juce::AudioProcessorValueTreeState::ParameterLayout p;
    p.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"mix",1},"Output mix",0.f,1.f,1.f));
    p.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"output",1},"Output dB",-60.f,12.f,0.f));
    p.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"width",1},"Output stereo width",0.f,2.f,1.f));
    p.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"minimum",1},"Minimum",0.f,fx?1.f:2.f,0.f));
    p.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"maximum",1},"Maximum",0.f,fx?1.f:2.f,1.f));return p;
}
UtilityEffect::UtilityEffect(bool fx):AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withInput("Sidechain",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),external(fx),state(*this,nullptr,"UTILITY_SETTINGS",layout(fx)){
    curve.reset(external?fxcurve::createEmbeddedProcessor():widthcurve::createEmbeddedProcessor());(external?fxcurve::forceCV(*curve):widthcurve::forceCV(*curve));
    for(auto* parameter:curve->getParameters())if(auto* p=dynamic_cast<juce::RangedAudioParameter*>(parameter))
        addParameter(new ForwardParameter("curve_"+p->paramID,"Curve "+p->getName(64),*p));
    formats.addDefaultFormats();setCurrentProgram(0);
}
UtilityEffect::~UtilityEffect(){closePluginEditor();for(auto* p:getParameters())if(auto* f=dynamic_cast<ForwardParameter*>(p))f->detach();}
void UtilityEffect::prepareToPlay(double sr,int n){
    sampleRate=sr;capacity=juce::jmax(1,n);cv.setSize(4,capacity);wet.setSize(2,capacity);
    curve->setRateAndBufferSizeDetails(sr,capacity);curve->prepareToPlay(sr,capacity);lookahead=curve->getLatencySamples();
    delay.setSize(2,int(sr*2)+capacity+1);curveDelay.setSize(2,lookahead+1);delay.clear();curveDelay.clear();write=curveWrite=0;
    events.ensureSize(16384);motion.reset(sr,.002);motion.setCurrentAndTargetValue(1);
    for(auto* smoother:{&outputMix,&outputGain,&outputWidth})smoother->reset(sr,.02);
    outputMix.setCurrentAndTargetValue(state.getRawParameterValue("mix")->load());
    outputGain.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(state.getRawParameterValue("output")->load()));
    outputWidth.setCurrentAndTargetValue(state.getRawParameterValue("width")->load());
    const juce::ScopedLock lock(pluginLock);if(plugin){plugin->setRateAndBufferSizeDetails(sr,capacity);plugin->prepareToPlay(sr,capacity);}
    setLatencySamples(lookahead+(plugin?plugin->getLatencySamples():0));
}
void UtilityEffect::releaseResources(){curve->releaseResources();const juce::ScopedLock lock(pluginLock);if(plugin)plugin->releaseResources();}
juce::String UtilityEffect::loadPlugin(const juce::File& file){
    if(!file.hasFileExtension("vst3"))return "Select a .vst3 effect or bundle";
    juce::OwnedArray<juce::PluginDescription> types;
    for(int i=0;i<formats.getNumFormats();++i)formats.getFormat(i)->findAllTypesForFile(types,file.getFullPathName());
    if(types.isEmpty())return "No compatible VST3 effect found";
    auto desc=*types[0];
    if(desc.isInstrument||desc.name.containsIgnoreCase("Jellybone")||desc.name=="Shaper")return "Choose an audio effect, not Jellybone or an instrument";
    juce::String error;auto next=formats.createPluginInstance(desc,sampleRate,juce::jmax(1,capacity),error);
    if(!next)return error;
    next->disableNonMainBuses();auto buses=next->getBusesLayout();buses.inputBuses.set(0,juce::AudioChannelSet::stereo());buses.outputBuses.set(0,juce::AudioChannelSet::stereo());
    if(!next->setBusesLayout(buses))return "This effect does not support stereo input/output";
    if(next->getLatencySamples()>int(sampleRate*2))return "Plugin latency exceeds the supported two-second compensation buffer";
    next->setRateAndBufferSizeDetails(sampleRate,juce::jmax(1,capacity));next->prepareToPlay(sampleRate,juce::jmax(1,capacity));
    // Eski instance ve penceresi audio callback içinde silinmez. Callback kilidi beklemez.
    const juce::ScopedLock lock(pluginLock);closePluginEditor();plugin=std::move(next);description=desc;status=desc.name;
    setLatencySamples(lookahead+plugin->getLatencySamples());return {};
}
juce::String UtilityEffect::pluginName(){const juce::ScopedLock lock(pluginLock);return status;}
void UtilityEffect::showPluginEditor(){const juce::ScopedLock lock(pluginLock);if(plugin&&plugin->hasEditor()){if(!pluginWindow)pluginWindow=std::make_unique<HostedWindow>(*plugin);pluginWindow->setVisible(true);pluginWindow->toFront(true);}}
void UtilityEffect::closePluginEditor(){pluginWindow.reset();}
void UtilityEffect::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer& midi){
    const int n=b.getNumSamples();if(capacity<=0||n>capacity)return;
    cv.setSize(4,n,false,false,true);wet.setSize(2,n,false,false,true);
    for(int c=0;c<4;++c){if(c<b.getNumChannels())cv.copyFrom(c,0,b,c,0,n);else cv.clear(c,0,n);}
    (external?fxcurve::forceCV(*curve):widthcurve::forceCV(*curve));curve->setPlayHead(getPlayHead());events.clear();events.addEvents(midi,0,n,0);curve->processBlock(cv,events);
    const juce::ScopedTryLock lock(pluginLock); // UI yüklerken beklemek yerine kuru geç.
    auto* fx=lock.isLocked()?plugin.get():nullptr;
    const int fxLatency=fx?juce::jlimit(0,delay.getNumSamples()-capacity-1,fx->getLatencySamples()):0;
    if(fx){
        for(int i=0;i<n;++i){
            for(int c=0;c<2;++c){curveDelay.setSample(c,curveWrite,b.getSample(c,i));wet.setSample(c,i,curveDelay.getSample(c,(curveWrite+curveDelay.getNumSamples()-lookahead)%curveDelay.getNumSamples()));}
            curveWrite=(curveWrite+1)%curveDelay.getNumSamples();
        }
        fx->setPlayHead(getPlayHead());events.clear();events.addEvents(midi,0,n,0);fx->processBlock(wet,events);
    }
    outputMix.setTargetValue(state.getRawParameterValue("mix")->load());
    outputGain.setTargetValue(juce::Decibels::decibelsToGain(state.getRawParameterValue("output")->load()));
    outputWidth.setTargetValue(state.getRawParameterValue("width")->load());
    const float lo=state.getRawParameterValue("minimum")->load(),hi=state.getRawParameterValue("maximum")->load();
    for(int i=0;i<n;++i){
        const float value=juce::jlimit(0.f,1.f,cv.getSample(0,i));motion.setTargetValue(lo+(hi-lo)*value);const float amount=motion.getNextValue();
        float dry[2];for(int c=0;c<2;++c){delay.setSample(c,write,b.getSample(c,i));dry[c]=delay.getSample(c,(write+delay.getNumSamples()-lookahead-fxLatency)%delay.getNumSamples());}
        write=(write+1)%delay.getNumSamples();
        if(!external){auto stereo=widthSample(dry[0],dry[1],amount);b.setSample(0,i,stereo.first);b.setSample(1,i,stereo.second);}
        else {const float mix=juce::jlimit(0.f,1.f,amount);for(int c=0;c<2;++c)b.setSample(c,i,fx?dry[c]+mix*(wet.getSample(c,i)-dry[c]):dry[c]);}
        const float blend=outputMix.getNextValue(),gain=outputGain.getNextValue();
        auto stereo=widthSample(dry[0]+blend*(b.getSample(0,i)-dry[0]),dry[1]+blend*(b.getSample(1,i)-dry[1]),outputWidth.getNextValue());
        b.setSample(0,i,stereo.first*gain);b.setSample(1,i,stereo.second*gain);
    }
}
const juce::String UtilityEffect::getProgramName(int i){static const char* names[]{"Full","Rise","Fall","Pulse","Steps","Drop","Lift","Down"};return names[juce::jlimit(0,7,i)];}
void UtilityEffect::setCurrentProgram(int i){program=juce::jlimit(0,7,i);(external?fxcurve::shape(*curve,program,1):widthcurve::shape(*curve,program,1));(external?fxcurve::forceCV(*curve):widthcurve::forceCV(*curve));}
void UtilityEffect::getStateInformation(juce::MemoryBlock& data){
    auto xml=state.copyState().createXml();xml->setAttribute("kind",external?"FX":"Width");xml->setAttribute("program",program);
    juce::MemoryBlock bytes;curve->getStateInformation(bytes);xml->createNewChildElement("Curve")->addTextElement(bytes.toBase64Encoding());
    const juce::ScopedLock lock(pluginLock);
    if(plugin){xml->addChildElement(description.createXml().release());bytes.reset();plugin->getStateInformation(bytes);xml->createNewChildElement("PluginState")->addTextElement(bytes.toBase64Encoding());}
    copyXmlToBinary(*xml,data);
}
void UtilityEffect::setStateInformation(const void* data,int size){
    if(auto xml=getXmlFromBinary(data,size))if(xml->hasTagName("UTILITY_SETTINGS")&&xml->getStringAttribute("kind")==getName()){
        if(auto* c=xml->getChildByName("Curve")){juce::MemoryBlock b;if(b.fromBase64Encoding(c->getAllSubText()))curve->setStateInformation(b.getData(),(int)b.getSize());}
        auto tree=juce::ValueTree::fromXml(*xml);for(int i=tree.getNumChildren();--i>=0;)if(!tree.getChild(i).hasType("PARAM"))tree.removeChild(i,nullptr);state.replaceState(tree);
        program=juce::jlimit(0,7,xml->getIntAttribute("program"));
        if(external){
            if(auto* d=xml->getChildByName("PLUGIN")){juce::PluginDescription desc;if(desc.loadFromXml(*d)){
                auto error=loadPlugin(juce::File(desc.fileOrIdentifier));if(error.isNotEmpty()){const juce::ScopedLock lock(pluginLock);closePluginEditor();plugin.reset();status="Missing / unavailable: "+desc.name;}
                else if(auto* s=xml->getChildByName("PluginState")){juce::MemoryBlock b;if(b.fromBase64Encoding(s->getAllSubText())){const juce::ScopedLock lock(pluginLock);plugin->setStateInformation(b.getData(),(int)b.getSize());}}
            }}else{const juce::ScopedLock lock(pluginLock);closePluginEditor();plugin.reset();status="No VST3 loaded";}
        }
        (external?fxcurve::forceCV(*curve):widthcurve::forceCV(*curve));
    }
}
juce::AudioProcessorEditor* UtilityEffect::createEditor(){return new UtilityEditor(*this);}

// Fazıl Irak · 03.11.2026: Harici pluginin normalize parametre erişimi; UI/test iş parçacığında çağrılır.
// Bu fonksiyon parametreleri Jellybone otomasyon kimliği gibi sunmaz; plugin kendi state'inde saklar.
bool UtilityEffect::setHostedParameter(const juce::String& name,float normalized) {
    const juce::ScopedLock lock(pluginLock);
    if(plugin)for(auto* p:plugin->getParameters())if(p->getName(128)==name){p->beginChangeGesture();p->setValueNotifyingHost(juce::jlimit(0.f,1.f,normalized));p->endChangeGesture();return true;}
    return false;
}
