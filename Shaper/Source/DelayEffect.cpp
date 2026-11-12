// SPDX-License-Identifier: GPL-3.0-only
// Fazıl Irak · 13.11.2026. Türkçe bakım notları: hedefler normalize edilir; Filter logaritmiktir.
#include "DelayEffect.h"
#include "Embedded/RackKnob.h"
#include "Embedded/RackEditorCommands.h"
namespace delaycurve {
juce::AudioProcessor* createEmbeddedProcessor();
void forceCV(juce::AudioProcessor&);void shape(juce::AudioProcessor&,int,float);
bool audioSettingsShown(juce::AudioProcessor&);
}
namespace {
const char* ids[]{"send","blend","feedback","filter"};
const char* names[]{"Send / Input","Mix","Feedback","Filter"};
class DelayEditor final:public juce::AudioProcessorEditor,public RackEditorCommands,private juce::Timer {
    DelayEffect& owner;
    std::unique_ptr<juce::AudioProcessorEditor> child;
    struct Panel:juce::Component {void paint(juce::Graphics& g)override{g.fillAll(juce::Colour(0xff1c2027));}} panel;
    juce::ComboBox targetChoice,division,mode;
    juce::ToggleButton enable{"Curve target"};
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> enabled;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> divisionAttach,modeAttach;
    std::array<std::unique_ptr<shaperui::RackKnob>,4> knobs;
    std::array<std::unique_ptr<shaperui::RackKnob>,2> timing;
    juce::Label legend;
    int dock=316;
    RackEditorCommands* commands()const{return dynamic_cast<RackEditorCommands*>(child.get());}
    void chooseTarget(){
        const int t=juce::jlimit(0,3,targetChoice.getSelectedId()-1);const juce::String prefix=ids[t];
        enabled.reset();enabled=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(owner.state,prefix+"_curve",enable);
        const char* suffix[]{"_min","_max","_depth","_smooth"};const char* labels[]{"Min","Max","Depth","Smooth"};
        for(int k=0;k<4;++k){knobs[k]=std::make_unique<shaperui::RackKnob>(owner.state,(prefix+suffix[k]).toRawUTF8(),labels[k],juce::Colour(0xff70d99a));
            knobs[k]->format=[t,k](float v){return k==3?juce::String(v,1)+" ms":t==3&&k<2?juce::String(80.f*std::pow(225.f,v),0)+" Hz":juce::String(v*100,0)+" %";};panel.addAndMakeVisible(*knobs[k]);}
        resized();
    }
    void timerCallback()override{panel.setVisible(!delaycurve::audioSettingsShown(*owner.curve));}
public:
    DelayEditor(DelayEffect& p):AudioProcessorEditor(p),owner(p){
        child.reset(p.curve->createEditor());addAndMakeVisible(*child);addAndMakeVisible(panel);
        for(auto* c:std::initializer_list<juce::Component*>{&targetChoice,&division,&mode,&enable,&legend})panel.addAndMakeVisible(*c);
        for(int i=0;i<4;++i)targetChoice.addItem(names[i],i+1);
        targetChoice.setComponentID("delay-target");targetChoice.onChange=[this]{chooseTarget();};
        division.addItemList({"Free","1/64","1/32","1/16","1/8","1/4","1/2","1/1","1/8 T","1/8 D"},1);
        mode.addItemList({"Stereo","Ping-pong"},1);
        divisionAttach=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.state,"division",division);
        modeAttach=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.state,"mode",mode);
        timing[0]=std::make_unique<shaperui::RackKnob>(p.state,"time_ms","Time ms",juce::Colour(0xff70d99a));
        timing[1]=std::make_unique<shaperui::RackKnob>(p.state,"tempo","Tempo",juce::Colour(0xff70d99a));
        for(auto& k:timing)panel.addAndMakeVisible(*k);
        legend.setText("Each target keeps its own range. Send=0 preserves echoes.",juce::dontSendNotification);legend.setColour(juce::Label::textColourId,juce::Colour(0xff70d99a));legend.setFont(juce::FontOptions(12));
        enable.setColour(juce::ToggleButton::tickColourId,juce::Colour(0xff70d99a));
        for(auto* c:{&targetChoice,&division,&mode}){c->setColour(juce::ComboBox::textColourId,juce::Colour(0xff70d99a));c->setColour(juce::ComboBox::outlineColourId,juce::Colour(0xff70d99a));}
        targetChoice.setSelectedId(1,juce::dontSendNotification);chooseTarget();setSize(1060,700);startTimerHz(5);
    }
    ~DelayEditor()override{stopTimer();}
    void resized()override{
        child->setBounds(getLocalBounds());if(commands())commands()->rackDockHeader(dock);
        // Panel 292 pikselde biter; 300 pikselde başlayan Paint/Seq/Curve satırına taşmaz.
        panel.setBounds(dock,168,juce::jmax(100,getWidth()-dock-8),124);panel.toFront(false);
        division.setBounds(8,4,108,24);mode.setBounds(122,4,115,24);
        timing[0]->setBounds(8,30,68,64);timing[1]->setBounds(80,30,68,64);
        targetChoice.setBounds(246,4,118,24);enable.setBounds(368,4,134,24);
        for(int k=0;k<4;++k)if(knobs[k])knobs[k]->setBounds(246+72*k,30,68,64);
        legend.setBounds(8,100,panel.getWidth()-16,20);
    }
    juce::PopupMenu rackOptions()override{return commands()?commands()->rackOptions():juce::PopupMenu();}
    void rackCommand(int c)override{if(commands())commands()->rackCommand(c);}
    bool rackCanEdit()const override{return commands()&&commands()->rackCanEdit();}
    bool rackIsSequencing()const override{return commands()&&commands()->rackIsSequencing();}
    void rackDockHeader(int x)override{dock=x;resized();}
    int rackHeaderHeight()const override{return 330;}
};
}
juce::AudioProcessorValueTreeState::ParameterLayout DelayEffect::layout(){
    juce::AudioProcessorValueTreeState::ParameterLayout p;
    auto add=[&](const juce::String& id,const juce::String& name,float lo,float hi,float initial){p.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{id,1},name,lo,hi,initial));};
    add("mix","Output mix",0,1,1);add("output","Output dB",-60,12,0);add("width","Output stereo width",0,2,1);
    add("time_ms","Delay time ms",1,4000,250);add("tempo","Fallback tempo",30,300,120);
    p.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"division",1},"Delay division",juce::StringArray{"Free","1/64","1/32","1/16","1/8","1/4","1/2","1/1","1/8 T","1/8 D"},5));
    p.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"mode",1},"Delay mode",juce::StringArray{"Stereo","Ping-pong"},0));
    const float defaults[]{1,.35f,.4f,.85f};
    for(int i=0;i<4;++i){juce::String prefix=ids[i];
        p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{prefix+"_curve",1},juce::String(names[i])+" curve",i==0));
        add(prefix+"_min",juce::String(names[i])+" min",0,1,i==0?0:defaults[i]);
        add(prefix+"_max",juce::String(names[i])+" max",0,1,defaults[i]);
        add(prefix+"_depth",juce::String(names[i])+" depth",0,1,1);
        add(prefix+"_smooth",juce::String(names[i])+" smoothing ms",1,200,5);
    }return p;
}
DelayEffect::DelayEffect():AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withInput("Sidechain",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),state(*this,nullptr,"JELLY_DELAY",layout()){
    curve.reset(delaycurve::createEmbeddedProcessor());delaycurve::forceCV(*curve);
    for(auto* parameter:curve->getParameters())if(auto* p=dynamic_cast<juce::RangedAudioParameter*>(parameter))addParameter(new ForwardParameter("curve_"+p->paramID,"Curve "+p->getName(64),*p));
    for(int t=0;t<4;++t){const juce::String prefix=ids[t];values[t]={state.getRawParameterValue(prefix+"_min"),state.getRawParameterValue(prefix+"_max"),state.getRawParameterValue(prefix+"_depth"),state.getRawParameterValue(prefix+"_smooth"),state.getRawParameterValue(prefix+"_curve")};}
    const char* commonIds[]{"tempo","division","time_ms","mix","output","width","mode"};for(int i=0;i<7;++i)common[i]=state.getRawParameterValue(commonIds[i]);
    setCurrentProgram(0);
}
DelayEffect::~DelayEffect(){for(auto* p:getParameters())if(auto* f=dynamic_cast<ForwardParameter*>(p))f->detach();}
void DelayEffect::prepareToPlay(double sr,int n){
    rate=sr;capacity=juce::jmax(1,n);cv.setSize(4,capacity);events.ensureSize(16384);
    curve->setRateAndBufferSizeDetails(sr,capacity);curve->prepareToPlay(sr,capacity);lookahead=curve->getLatencySamples();
    dryDelay.setSize(2,lookahead+1);dryDelay.clear();write=0;setLatencySamples(lookahead);
    for(int c=0;c<2;++c){lines[c].resize(int(sr*6)+4);lines[c].clear();filters[c].clear(0);}
    for(int t=0;t<4;++t){target[t].reset(sr,.005);target[t].setCurrentAndTargetValue(value(juce::String(ids[t])+"_max"));}
    time.reset(sr,.15);time.setCurrentAndTargetValue(float(sr*.5));
    for(auto* s:{&mix,&gain,&width})s->reset(sr,.02);
    mix.setCurrentAndTargetValue(value("mix"));gain.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(value("output")));width.setCurrentAndTargetValue(value("width"));
}
void DelayEffect::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer& midi){
    juce::ScopedNoDenormals noDenormals;const int n=b.getNumSamples();if(capacity<=0||n>capacity)return;
    cv.setSize(4,n,false,false,true);for(int c=0;c<4;++c){if(c<b.getNumChannels())cv.copyFrom(c,0,b,c,0,n);else cv.clear(c,0,n);}
    delaycurve::forceCV(*curve);curve->setPlayHead(getPlayHead());events.clear();events.addEvents(midi,0,n,0);curve->processBlock(cv,events);
    double bpm=common[0]->load();if(auto* ph=getPlayHead())if(auto position=ph->getPosition())if(auto hostBpm=position->getBpm())bpm=*hostBpm;
    const float beats[]{0,.0625f,.125f,.25f,.5f,1,2,4,1.f/3.f,.75f};const int d=juce::jlimit(0,9,int(common[1]->load()));
    const float seconds=d?float(60./juce::jmax(1.,bpm))*beats[d]:common[2]->load()*.001f;
    time.setTargetValue(juce::jlimit(1.f,float(rate*5.9),float(rate)*seconds));
    float lo[4],hi[4],depth[4],alpha[4];bool enabled[4];
    for(int t=0;t<4;++t){const auto& v=values[t];lo[t]=v.lo->load();hi[t]=v.hi->load();depth[t]=v.depth->load();enabled[t]=v.enabled->load()>.5f;alpha[t]=1.f-std::exp(-1.f/(float(rate)*.001f*v.smoothing->load()));}
    mix.setTargetValue(common[3]->load());gain.setTargetValue(juce::Decibels::decibelsToGain(common[4]->load()));width.setTargetValue(common[5]->load());
    const bool ping=common[6]->load()>.5f;
    for(int i=0;i<n;++i){
        const float env=juce::jlimit(0.f,1.f,cv.getSample(0,i));float motion[4];
        // Bağımsız zaman sabitleri: reset() her örnekte çağrılmaz; bellek ayrılmaz.
        for(int t=0;t<4;++t){float v=enabled[t]?hi[t]+depth[t]*(lo[t]+(hi[t]-lo[t])*env-hi[t]):hi[t];float current=target[t].getCurrentValue();motion[t]=current+alpha[t]*(v-current);target[t].setCurrentAndTargetValue(motion[t]);}
        const float delaySamples=time.getNextValue();float dry[2],echo[2];
        const float cutoff=juce::jmin(float(rate*.45),80.f*std::pow(225.f,motion[3]));
        for(int c=0;c<2;++c){dryDelay.setSample(c,write,b.getSample(c,i));dry[c]=dryDelay.getSample(c,(write+dryDelay.getNumSamples()-lookahead)%dryDelay.getNumSamples());
            filters[c].lp(float(rate),cutoff,.70710678f);echo[c]=filters[c].process(lines[c].read(delaySamples));}
        write=(write+1)%dryDelay.getNumSamples();
        const float feedback=juce::jlimit(0.f,.94f,motion[2]);
        for(int c=0;c<2;++c){float input=dry[c]*motion[0];if(ping)input=c==0?.5f*(dry[0]+dry[1])*motion[0]:0.f;
            // Feedback yolu yumuşak sınırlandırılır; Send=0 eski kuyruğu kesmez.
            float next=input+feedback*echo[ping?1-c:c];if(!std::isfinite(next))next=0;lines[c].write(4.f*std::tanh(next*.25f));}
        const float wet=juce::jlimit(0.f,1.f,motion[1])*mix.getNextValue(),g=gain.getNextValue();
        const float l=dry[0]*(1-wet)+echo[0]*wet,r=dry[1]*(1-wet)+echo[1]*wet;
        const float mid=.5f*(l+r),side=.5f*(l-r)*width.getNextValue();b.setSample(0,i,(mid+side)*g);b.setSample(1,i,(mid-side)*g);
    }
}
const juce::String DelayEffect::getProgramName(int i){static const char* presets[]{"Echo","Throw","Fade","Pulse","Steps","Rise","Accent","Swell"};return presets[juce::jlimit(0,7,i)];}
void DelayEffect::setCurrentProgram(int i){program=juce::jlimit(0,7,i);delaycurve::shape(*curve,program,1);delaycurve::forceCV(*curve);}
void DelayEffect::getStateInformation(juce::MemoryBlock& data){auto xml=state.copyState().createXml();xml->setAttribute("program",program);juce::MemoryBlock bytes;curve->getStateInformation(bytes);xml->createNewChildElement("Curve")->addTextElement(bytes.toBase64Encoding());copyXmlToBinary(*xml,data);}
void DelayEffect::setStateInformation(const void* data,int size){if(auto xml=getXmlFromBinary(data,size))if(xml->hasTagName("JELLY_DELAY")){
    if(auto* node=xml->getChildByName("Curve")){juce::MemoryBlock bytes;if(bytes.fromBase64Encoding(node->getAllSubText()))curve->setStateInformation(bytes.getData(),int(bytes.getSize()));}
    auto tree=juce::ValueTree::fromXml(*xml);for(int i=tree.getNumChildren();--i>=0;)if(!tree.getChild(i).hasType("PARAM"))tree.removeChild(i,nullptr);state.replaceState(tree);program=juce::jlimit(0,7,xml->getIntAttribute("program"));delaycurve::forceCV(*curve);
}}
juce::AudioProcessorEditor* DelayEffect::createEditor(){return new DelayEditor(*this);}


// Fazıl Irak · 12.11.2026: render kuyruğu -80 dB eşiğine kadar güvenli üst sınırla bildirilir.
double DelayEffect::getTailLengthSeconds() const {
    const float feedback=juce::jlimit(0.f,.94f,juce::jmax(value("feedback_min"),value("feedback_max")));
    const double seconds=value("division")<.5f?value("time_ms")*.001:5.9;
    return seconds*(feedback<.001f?1.:1.+std::ceil(std::log(.0001)/std::log(feedback)));
}

