// Geliştirici: Fazıl Irak - Son düzenleme: 27.10.2026
// Dosyanın görevi: Host parametreleri, ses callback, durum kaydı ve kaynak sahipliği köprüsü.
// Copyright (c) 2026 Fazıl Irak - Özgün F-Kick uygulama kodu.
// SPDX-License-Identifier: AGPL-3.0-or-later
// Fazıl Irak · 27.10.2026 · Audio callback'te dosya erişimi, sample tahsisi veya bekleyen kilit kullanılmaz.
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "FactoryBank.h"

namespace {
const char* ids[]={"mode","gate","loop","reverse","poly","tracking","sampleGain","sampleTune","samplePan",
    "bodyGain","root","sweep","phase","harmonics","length","attack","release","sustain","cutoff","resonance","drive","output","start","end"};

const char* fxIDs[]={"hp","rumbleMix","rumbleDecay","rumbleHP","rumbleLP","rumbleDrive","duck","recover","echoMix","echoTime","echoFeedback","echoLP","echoSync","echoDivision","echoSource","driveType","tempo","lfoHz","lfoSync","lfoShape","lfoDivision","lfoPhase","lfoRetrigger","adDelay","adAttack","adDecay","adCurve","punch","rumbleGain","scShape","scShift","scMix","room","fxHP","fxLP","bassLift","bassQ"};
juce::String curveID(int c,int point,const char* part) {return juce::String(c==0?"amp":"pitch")+juce::String(point)+part;}
fkick::Curve factoryCurve(bool pitch) {
    fkick::Curve c;c.count=5;
    c.x={0,.025f,.16f,.55f,1,1,1,1};
    c.y=pitch?std::array<float,8>{1,.48f,.08f,0,0,0,0,0}:std::array<float,8>{0,1,.82f,.42f,0,0,0,0};
    c.bend={-.15f,-.15f,0,.15f,0,0,0,0};
    if(pitch){c.x={0,.008f,.056f,.55f,1,1,1,1};c.y={1,.4f,0,0,0,0,0,0};}
    else {c.x={0,.0024f,.11f,.32f,1,1,1,1};c.y={0,1,.85f,.65f,0,0,0,0};}
    return c;
}
// WAV üst sınırı state içindeki örneklerin sınırsız tahsis oluşturmasını engeller.
constexpr int maxWavBytes=48*1024*1024;
std::unique_ptr<fkick::Sample> decode(juce::AudioFormatReader& r) {
    if(r.lengthInSamples<2||r.sampleRate<8000||r.sampleRate>192000||r.lengthInSamples>r.sampleRate*30||r.numChannels<1||r.numChannels>2) return {};
    auto s=std::make_unique<fkick::Sample>();s->rate=r.sampleRate;s->audio.setSize((int)r.numChannels,(int)r.lengthInSamples);
    if(!r.read(&s->audio,0,(int)r.lengthInSamples,0,true,true))return {};
    for(int c=0;c<s->audio.getNumChannels();++c)for(int i=0;i<s->audio.getNumSamples();++i) {
        auto& v=s->audio.getWritePointer(c)[i];v=std::isfinite(v)?juce::jlimit(-8.f,8.f,v):0;
    }
    return s;
}
juce::MemoryBlock encode(const fkick::Sample& s) {
    juce::MemoryBlock data;
    auto stream=std::make_unique<juce::MemoryOutputStream>(data,false);
    juce::WavAudioFormat format;
    std::unique_ptr<juce::AudioFormatWriter> writer(format.createWriterFor(stream.get(),s.rate,(unsigned)s.audio.getNumChannels(),24,{},0));
    if(!writer)return {};
    stream.release();writer->writeFromAudioSampleBuffer(s.audio,0,s.audio.getNumSamples());writer.reset();return data;
}
}
juce::AudioProcessorValueTreeState::ParameterLayout FKickProcessor::layout() {
    juce::AudioProcessorValueTreeState::ParameterLayout l;
    auto f=[&](const char* id,const char* name,float min,float max,float def,float skew=1.f){l.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{id,1},name,juce::NormalisableRange<float>(min,max,0,skew),def));};
    auto b=[&](const char* id,const char* name,bool def){l.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{id,1},name,def));};
    l.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"mode",1},"Mode",juce::StringArray{"Kick","Sampler"},0));
    b("gate","MIDI gate",false);b("loop","Sample loop",false);b("reverse","Reverse sample",false);
    b("poly","Polyphonic",false);b("tracking","Track body tail",false);
    f("sampleGain","Sample level",-60,6,-6);f("sampleTune","Sample tune",-24,24,0);f("samplePan","Sample pan",-1,1,0);
    f("bodyGain","Body level",-60,6,-5);f("root","Body root Hz",25,140,50,.55f);f("sweep","Pitch sweep st",0,84,48);
    f("phase","Body phase",0,360,0);f("harmonics","Harmonics",0,1,0);
    f("length","Length ms",40,3000,500,.45f);f("attack","Attack ms",0,200,1,.4f);f("release","Release ms",2,1500,80,.45f);f("sustain","Sampler sustain",0,1,.8f);
    f("cutoff","Filter Hz",80,20000,18000,.3f);f("resonance","Filter Q",.5f,2,.707f);f("drive","Drive dB",0,24,0);f("output","Output dB",-24,6,0);
    f("start","Sample start",0,.999f,0);f("end","Sample end",.001f,1,1);
    for(int c=0;c<2;++c) {
        auto initial=factoryCurve(c==1);
        l.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{curveID(c,0,"Count"),1},c==0?"Amp points":"Pitch points",2,fkick::maxPoints,5));
        for(int p=0;p<fkick::maxPoints;++p)for(int v=0;v<3;++v) {
            auto id=curveID(c,p,v==0?"X":v==1?"Y":"Curve");
            f(id.toRawUTF8(),id.toRawUTF8(),v==2?-1.f:0.f,1,v==0?initial.x[(size_t)p]:v==1?initial.y[(size_t)p]:initial.bend[(size_t)p]);
        }
    }
    // Eski 74 parametrenin indeksleri korunur; FX ve modülasyon parametreleri sona eklenir.
    auto choice=[&](const char* id,const char* name,juce::StringArray items,int def){l.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{id,1},name,items,def));};
    f("hp","Independent HP Hz",0,2000,0,.35f);f("rumbleMix","Rumble amount",0,1,0);f("rumbleDecay","Rumble decay s",.2f,4,1.8f,.6f);
    f("rumbleHP","Rumble HP Hz",20,400,30,.5f);f("rumbleLP","Rumble LP Hz",100,4000,700,.4f);f("rumbleDrive","Rumble drive dB",0,30,8);
    f("duck","FX duck",0,1,.8f);f("recover","Duck recover ms",20,600,120,.5f);f("echoMix","Echo amount",0,1,0);
    f("echoTime","Echo time ms",10,1500,222,.5f);f("echoFeedback","Echo feedback",0,.8f,.35f);f("echoLP","Echo LP Hz",200,12000,3500,.4f);
    b("echoSync","Echo sync",true);choice("echoDivision","Echo division",{"1/16","1/8","1/8 D","1/4","1/4 D"},1);choice("echoSource","Echo source",{"Kick","Rumble"},0);
    choice("driveType","Drive character",{"Soft","Clip","Fold"},0);f("tempo","Fallback tempo BPM",40,240,135);
    f("lfoHz","LFO rate Hz",.05f,20,2,.35f);b("lfoSync","LFO sync",true);choice("lfoShape","LFO shape",{"Sine","Triangle","Saw Up","Square","Random","Saw Down"},0);
    choice("lfoDivision","LFO division",{"1/16","1/8","1/4","1/2","1 bar"},1);f("lfoPhase","LFO phase",0,360,0);b("lfoRetrigger","LFO retrigger",true);
    f("adDelay","AD delay ms",0,500,0,.5f);f("adAttack","AD attack ms",0,500,10,.5f);f("adDecay","AD decay ms",5,3000,300,.5f);f("adCurve","AD curve",-1,1,0);
    for(const auto* prefix:{"lfo_","ad_"})for(int i=0;i<fkick::modTargetCount;++i){auto id=juce::String(prefix)+fkick::modIDs[i];auto name=juce::String(juce::String(prefix).startsWith("lfo")?"LFO > ":"AD > ")+fkick::modNames[i];f(id.toRawUTF8(),name.toRawUTF8(),-1,1,0);}
    // İlk 121 host parametresi aynıdır; yeni iki parametre routing alanlarından sonra eklenir.
    f("punch","Body punch",0,1,0);f("rumbleGain","Rumble return gain dB",-24,24,0);
    choice("scShape","Sidechain curve",{"Pump","Smooth","Late","Gate"},0);f("scShift","Sidechain beat shift",-.5f,.5f,0);f("scMix","Sidechain mix",0,1,0);f("room","Room mix",0,1,0);f("fxHP","FX return HP Hz",0,4000,0,.35f);f("fxLP","FX return LP Hz",80,20000,20000,.3f);
    f("bassLift","Bass Lift dB",-12,12,0);f("bassQ","Bass Q",.1f,2,.707f);
    return l;
}
FKickProcessor::FKickProcessor():AudioProcessor(BusesProperties().withOutput("Output",juce::AudioChannelSet::stereo(),true)),state(*this,&undo,"FKICK_PARAMETERS",layout()) {
    keyboard.addListener(this);
    for(size_t i=0;i<params.size();++i)params[i]=state.getRawParameterValue(ids[i]);
    for(size_t i=0;i<fxParams.size();++i)fxParams[i]=state.getRawParameterValue(fxIDs[i]);
    for(int c=0;c<2;++c)for(int i=0;i<fkick::modTargetCount;++i)routeParams[(size_t)c][(size_t)i]=state.getRawParameterValue(juce::String(c==0?"lfo_":"ad_")+fkick::modIDs[i]);
    for(int c=0;c<2;++c) {
        curves[(size_t)c][0]=state.getRawParameterValue(curveID(c,0,"Count"));
        for(int p=0;p<fkick::maxPoints;++p)for(int v=0;v<3;++v)curves[(size_t)c][(size_t)(1+p*3+v)]=state.getRawParameterValue(curveID(c,p,v==0?"X":v==1?"Y":"Curve"));
    }
}
FKickProcessor::~FKickProcessor() {keyboard.removeListener(this);worker.removeAllJobs(true,-1);delete pending.exchange(nullptr);delete active;drainRetired();}
void FKickProcessor::enqueueKey(int channel,int note,float velocity,bool on) noexcept {
    int a,n,b,m;keyFifo.prepareToWrite(1,a,n,b,m);if(n>0){keyEvents[(size_t)a]={note,channel,velocity,on};keyFifo.finishedWrite(1);}else requestPanic();
}
void FKickProcessor::handleNoteOn(juce::MidiKeyboardState*,int channel,int note,float velocity) {enqueueKey(channel,note,velocity,true);}
void FKickProcessor::handleNoteOff(juce::MidiKeyboardState*,int channel,int note,float velocity) {enqueueKey(channel,note,velocity,false);}
void FKickProcessor::drainRetired() {
    int a,n,b,m;retiredFifo.prepareToRead(retiredFifo.getNumReady(),a,n,b,m);
    for(int i=0;i<n;++i)delete retired[(size_t)(a+i)];for(int i=0;i<m;++i)delete retired[(size_t)(b+i)];
    retiredFifo.finishedRead(n+m);
}
void FKickProcessor::prepareToPlay(double sr,int block) {setRateAndBufferSizeDetails(sr,block);engine.prepare(sr);}
bool FKickProcessor::isBusesLayoutSupported(const BusesLayout& l) const {return l.getMainInputChannelSet().isDisabled()&&(l.getMainOutputChannelSet()==juce::AudioChannelSet::mono()||l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo());}
fkick::Curve FKickProcessor::curve(int c) const noexcept {
    fkick::Curve out;const auto& p=curves[(size_t)juce::jlimit(0,1,c)];out.count=int(p[0]->load());
    for(int i=0;i<fkick::maxPoints;++i){out.x[(size_t)i]=p[(size_t)(1+i*3)]->load();out.y[(size_t)i]=p[(size_t)(2+i*3)]->load();out.bend[(size_t)i]=p[(size_t)(3+i*3)]->load();}
    out.sanitise();return out;
}
fkick::Settings FKickProcessor::settings() const noexcept {
    auto v=[this](P p){return params[(size_t)p]->load(std::memory_order_relaxed);};
    fkick::Settings s;s.mode=int(v(Mode));s.gate=v(Gate)>.5f;s.loop=v(Loop)>.5f;s.reverse=v(Reverse)>.5f;s.poly=v(Poly)>.5f;s.trackTail=v(Tracking)>.5f;
    s.sampleGain=juce::Decibels::decibelsToGain(v(SampleGain),-59.9f);s.bodyGain=juce::Decibels::decibelsToGain(v(BodyGain),-59.9f);
    s.sampleTune=v(SampleTune);s.samplePan=v(SamplePan);s.root=v(Root);s.sweep=v(Sweep);s.phase=v(Phase);s.harmonics=v(Harmonics);
    s.length=v(Length)*.001f;s.attack=v(Attack)*.001f;s.release=v(Release)*.001f;s.sustain=v(Sustain);
    s.cutoff=v(Cutoff);s.resonance=v(Resonance);s.drive=v(Drive);s.output=juce::Decibels::decibelsToGain(v(Output));
    s.start=v(Start);s.end=juce::jmax(s.start+.001f,v(End));s.amp=curve(0);s.pitch=curve(1);
    auto fx=[this](F p){return fxParams[(size_t)p]->load(std::memory_order_relaxed);};
    s.fx.scShape=int(fx(SCShape));s.fx.scShift=fx(SCShift);s.fx.scMix=fx(SCMix);s.fx.room=fx(Room);s.fx.fxHP=fx(FXHP);s.fx.fxLP=fx(FXLP);s.fx.bassLift=fx(BassLift);s.fx.bassQ=fx(BassQ);
    s.punch=fx(Punch);s.fx.rumbleGain=fx(RumbleGain);s.fx.hp=fx(HP);s.fx.rumbleMix=fx(RMix);s.fx.rumbleDecay=fx(RDecay);s.fx.rumbleHP=fx(RHP);s.fx.rumbleLP=fx(RLP);s.fx.rumbleDrive=fx(RDrive);s.fx.duck=fx(Duck);s.fx.recover=fx(Recover)*.001f;
    s.fx.echoMix=fx(EMix);s.fx.echoTime=fx(ETime)*.001f;s.fx.feedback=fx(EFeedback);s.fx.echoLP=fx(ELP);s.fx.echoSync=fx(ESync)>.5f;s.fx.echoDivision=int(fx(EDivision));s.fx.echoSource=int(fx(ESource));s.fx.driveType=int(fx(DriveType));
    const double host=hostTempo.load();s.motion.bpm=host>0?host:fx(Tempo);s.motion.hz=fx(LHz);s.motion.sync=fx(LSync)>.5f;s.motion.shape=int(fx(LShape));s.motion.division=int(fx(LDivision));s.motion.phase=fx(LPhase);s.motion.retrigger=fx(LRetrigger)>.5f;
    s.motion.delay=fx(ADDelay)*.001f;s.motion.attack=fx(ADAttack)*.001f;s.motion.decay=fx(ADDecay)*.001f;s.motion.curve=fx(ADCurve);
    for(int i=0;i<fkick::modTargetCount;++i){s.motion.lfo[(size_t)i]=routeParams[0][(size_t)i]->load();s.motion.ad[(size_t)i]=routeParams[1][(size_t)i]->load();}return s;
}
double FKickProcessor::getTailLengthSeconds() const {const auto s=settings();return juce::jmax(5.,s.length+s.release+fkick::TechnoFX::tailSeconds(s.fx,s.motion));}
void FKickProcessor::processBlock(juce::AudioBuffer<float>& audio,juce::MidiBuffer& midi) {
    juce::ScopedNoDenormals noDenormals;audio.clear();
    double bpm=0,beat=0;bool clock=false;if(auto* head=getPlayHead())if(auto pos=head->getPosition()){if(auto tempo=pos->getBpm())if(std::isfinite(*tempo)&&*tempo>=40&&*tempo<=300)bpm=*tempo;if(auto ppq=pos->getPpqPosition())if(std::isfinite(*ppq)){beat=*ppq;clock=pos->getIsPlaying();}}hostTempo.store(bpm);
    if(retiredFifo.getFreeSpace()>0) if(auto* next=pending.exchange(nullptr,std::memory_order_acquire)) {
        if(active) {int a,n,b,m;retiredFifo.prepareToWrite(1,a,n,b,m);retired[(size_t)a]=active;retiredFifo.finishedWrite(1);}
        active=next;engine.setSample(active->audio.getNumSamples()>1?active:nullptr);
    }
    const auto s=settings();engine.syncFX(beat,clock);int cursor=0;
    // MidiKeyboardState::processNextMidiBuffer kilit alır ve buffer büyütebilir; callback'te çağrılmaz.
    // Ekran klavyesi listener'ı yalnız GUI tarafında bu sabit SPSC kuyruğuna yazar.
    const bool panic=panicRequested.exchange(false);
    if(panic)engine.panic();
    int ka,kn,kb,km;keyFifo.prepareToRead(keyFifo.getNumReady(),ka,kn,kb,km);
    auto playKey=[&](const KeyEvent& key){if(panic)return;if(key.on)engine.noteOn(key.note,key.channel,key.velocity,s);else engine.noteOff(key.note,key.channel,s);};
    for(int i=0;i<kn;++i)playKey(keyEvents[(size_t)(ka+i)]);for(int i=0;i<km;++i)playKey(keyEvents[(size_t)(kb+i)]);keyFifo.finishedRead(kn+km);
    for(const auto metadata:midi) {
        if(metadata.numBytes>3)continue; // SysEx kopyalama/tahsis yapmadan atlanır.
        const int time=juce::jlimit(cursor,audio.getNumSamples(),metadata.samplePosition);
        engine.render(audio,cursor,time-cursor,s);cursor=time;const auto message=metadata.getMessage();
        if(message.isNoteOn())engine.noteOn(message.getNoteNumber(),message.getChannel(),message.getFloatVelocity(),s);
        else if(message.isNoteOff())engine.noteOff(message.getNoteNumber(),message.getChannel(),s);
        else if(message.isPitchWheel())engine.pitchWheel(message.getChannel(),(message.getPitchWheelValue()-8192)*2.f/8192.f);
        else if(message.isSustainPedalOn()||message.isSustainPedalOff())engine.sustainPedal(message.getChannel(),message.isSustainPedalOn(),s);
        else if(message.isAllNotesOff()||message.isAllSoundOff())engine.panic();
    }
    engine.render(audio,cursor,audio.getNumSamples()-cursor,s);
    float peak=0;for(int c=0;c<audio.getNumChannels();++c)peak=juce::jmax(peak,audio.getMagnitude(c,0,audio.getNumSamples()));
    meter.store(peak);voiceCount.store(engine.activeVoices());lfoMeter.store(engine.lfoValue());adMeter.store(engine.adValue());scPhase.store(engine.sidechainPhase());midi.clear();
}
void FKickProcessor::setParameter(const juce::String& id,float plain) {if(auto* p=state.getParameter(id)){p->beginChangeGesture();p->setValueNotifyingHost(p->convertTo0to1(plain));p->endChangeGesture();}}
float FKickProcessor::value(const juce::String& id) const {auto* p=state.getRawParameterValue(id);return p?p->load():0;}
void FKickProcessor::setCurve(int c,const fkick::Curve& input) {
    auto a=input;a.sanitise();
    for(int i=0;i<fkick::maxPoints;++i){setParameter(curveID(c,i,"X"),a.x[(size_t)i]);setParameter(curveID(c,i,"Y"),a.y[(size_t)i]);setParameter(curveID(c,i,"Curve"),a.bend[(size_t)i]);}
    setParameter(curveID(c,0,"Count"),float(a.count));
}
const juce::String FKickProcessor::getProgramName(int i) {const auto& p=fkick::factoryBank()[juce::jlimit(0,127,i)];return p["name"].toString();}
void FKickProcessor::setCurrentProgram(int index) {
    index=juce::jlimit(0,127,index);undo.beginNewTransaction("Factory preset");
    for(auto* parameter:getParameters())if(auto* p=dynamic_cast<juce::RangedAudioParameter*>(parameter))p->setValueNotifyingHost(p->getDefaultValue());
    // Fabrika kick'i açık sample'a bağımlı değildir; sample dosyası instance içinde korunur.
    setParameter("sampleGain",-60);
    const auto& values=fkick::factoryBank()[index]["values"];
    if(auto* object=values.getDynamicObject())for(const auto& item:object->getProperties())if(item.name.toString()!="contour")setParameter(item.name.toString(),float(item.value));
    auto amp=factoryCurve(false),pitch=factoryCurve(true);
    const int contour=values.hasProperty("contour")?int(values["contour"]):-1;
    if(contour>=0){
        // Konturlar: kısa transient, tok gövde, sub kuyruğu, iki darbe ve ters yükseliş.
        constexpr float ampY[][5]={{0,1,.72f,.25f,0},{0,1,.45f,.09f,0},{0,1,.32f,.04f,0},{0,1,.9f,.54f,0},{0,1,.65f,.13f,0},{0,1,.95f,.31f,0},{0,1,.6f,.2f,0},{0,1,.8f,.12f,0},{0,1,.2f,1,0},{0,.2f,.48f,1,0},{0,1,.7f,.4f,0}};
        constexpr float pitchY[][5]={{1,.3f,.03f,0,0},{1,.18f,0,0,0},{1,.22f,.02f,0,0},{1,.65f,.14f,0,0},{1,.45f,.01f,0,0},{1,.55f,.22f,0,0},{1,.72f,.18f,.02f,0},{1,.35f,.13f,.01f,0},{1,.1f,.4f,.04f,0},{0,.12f,.38f,.8f,1},{1,.78f,.4f,.14f,0}};
        const int c=juce::jlimit(0,10,contour);for(int k=0;k<5;++k){amp.y[(size_t)k]=ampY[c][k];pitch.y[(size_t)k]=pitchY[c][k];}
        if(c==1||c==2){amp.x={0,.012f,.1f,.38f,1,1,1,1};pitch.x={0,.012f,.07f,.3f,1,1,1,1};}
        if(c==8){amp.x={0,.025f,.38f,.52f,1,1,1,1};pitch.x=amp.x;}
        if(c==9){amp.x={0,.1f,.42f,.8f,1,1,1,1};pitch.x=amp.x;}
        if(c==10){pitch.bend={0,0,0,0,0,0,0,0};}
    }
    if(contour<8){
        const float seconds=value("length")*.001f;
        // Attack/pitch düşüşü kuyruk süresiyle uzamasın; ilk 55 ms'de gövdeyi dolu tut.
        amp.x[1]=juce::jlimit(.001f,.02f,.0012f/seconds);amp.x[2]=juce::jlimit(.08f,.38f,.055f/seconds);
        amp.x[3]=juce::jlimit(amp.x[2]+.08f,.85f,.16f/seconds);amp.y[2]=juce::jmax(.8f,amp.y[2]);amp.y[3]=juce::jmax(.65f,amp.y[3]);
        pitch.x[1]=juce::jlimit(.003f,.08f,.004f/seconds);pitch.x[2]=juce::jlimit(.025f,.32f,.028f/seconds);pitch.x[3]=.55f;
        pitch.y[1]=juce::jmin(.4f,pitch.y[1]);pitch.y[2]=pitch.y[3]=pitch.y[4]=0;
    }
    setCurve(0,amp);setCurve(1,pitch);program.store(index);status({});
}
void FKickProcessor::status(const juce::String& text) {const juce::ScopedLock lock(sampleLock);view.status=text;}
FKickProcessor::SampleView FKickProcessor::sampleView() const {const juce::ScopedLock lock(sampleLock);return view;}
void FKickProcessor::publishSample(std::unique_ptr<fkick::Sample> s,juce::MemoryBlock wav,const juce::String& name) {
    drainRetired();SampleView next;next.loaded=s->audio.getNumSamples()>1;next.name=next.loaded?name:"Drop a sample here";
    if(next.loaded) {
        next.duration=s->audio.getNumSamples()/s->rate;
        for(int i=0;i<512;++i) {const int a=i*s->audio.getNumSamples()/512,b=juce::jmax(a+1,(i+1)*s->audio.getNumSamples()/512);
            for(int c=0;c<s->audio.getNumChannels();++c)next.peaks[(size_t)i]=juce::jmax(next.peaks[(size_t)i],s->audio.getMagnitude(c,juce::jmin(a,s->audio.getNumSamples()-1),juce::jmin(b-a,s->audio.getNumSamples()-juce::jmin(a,s->audio.getNumSamples()-1))));}
    }
    {const juce::ScopedLock lock(sampleLock);view=next;sampleWav=std::move(wav);}
    delete pending.exchange(s.release(),std::memory_order_release);
}
bool FKickProcessor::loadSample(const juce::File& file) {
    const juce::ScopedLock serialise(loadLock);
    juce::AudioFormatManager formats;formats.registerBasicFormats();std::unique_ptr<juce::AudioFormatReader> reader(formats.createReaderFor(file));
    if(!reader){status("Cannot read this audio file.");return false;}
    auto sample=decode(*reader);if(!sample){status("Use mono/stereo audio, up to 30 seconds, 8-192 kHz.");return false;}
    auto encoded=encode(*sample);if(encoded.isEmpty()){status("Could not prepare sample state.");return false;}
    publishSample(std::move(sample),std::move(encoded),file.getFileName());return true;
}
void FKickProcessor::loadSampleAsync(const juce::File& file) {status("Loading sample...");worker.addJob([this,file]{loadSample(file);});}
void FKickProcessor::clearSample() {const juce::ScopedLock serialise(loadLock);publishSample(std::make_unique<fkick::Sample>(),{},{});}
std::unique_ptr<juce::XmlElement> FKickProcessor::makeState() {
    auto xml=std::make_unique<juce::XmlElement>("FKICK_PRESET");xml->setAttribute("version",1);xml->setAttribute("bankRevision",2);xml->setAttribute("program",program.load());xml->addChildElement(state.copyState().createXml().release());
    const juce::ScopedLock lock(sampleLock);
    if(!sampleWav.isEmpty()){auto* s=xml->createNewChildElement("Sample");s->setAttribute("name",view.name);s->addTextElement(sampleWav.toBase64Encoding());}
    return xml;
}
void FKickProcessor::getStateInformation(juce::MemoryBlock& data) {copyXmlToBinary(*makeState(),data);}
bool FKickProcessor::restore(const void* data,int size) {
    if(size<=0||size>maxWavBytes*2)return false;
    auto xml=getXmlFromBinary(data,size);if(!xml||!xml->hasTagName("FKICK_PRESET")||xml->getIntAttribute("version")!=1)return false;
    auto* parameters=xml->getChildByName("FKICK_PARAMETERS");if(!parameters)return false;
    auto tree=juce::ValueTree::fromXml(*parameters);if(!tree.isValid())return false;
    const bool legacyImpact=!tree.getChildWithProperty("id","punch").isValid();
    // Dış presetin geçersiz float değerleri canlı motora taşınmaz.
    for(auto child:tree){const float v=(float)child.getProperty("value");if(!std::isfinite(v))return false;}
    // v1 eski presetlerinde bulunmayan yeni FX/mapping alanları, o anki ayarlardan değil varsayılandan gelsin.
    for(auto* parameter:getParameters())if(auto* p=dynamic_cast<juce::RangedAudioParameter*>(parameter))if(!tree.getChildWithProperty("id",p->paramID).isValid()){
        juce::ValueTree child("PARAM");child.setProperty("id",p->paramID,nullptr);child.setProperty("value",p->paramID=="punch"||p->paramID=="rumbleGain"?0.f:p->convertFrom0to1(p->getDefaultValue()),nullptr);tree.appendChild(child,nullptr);
    }
    std::unique_ptr<fkick::Sample> loaded=std::make_unique<fkick::Sample>();juce::MemoryBlock wav;juce::String name;
    if(auto* child=xml->getChildByName("Sample")) {
        if(child->getAllSubText().length()>maxWavBytes*2||!wav.fromBase64Encoding(child->getAllSubText())||wav.getSize()>maxWavBytes)return false;
        juce::WavAudioFormat format;std::unique_ptr<juce::AudioFormatReader> reader(format.createReaderFor(new juce::MemoryInputStream(wav,false),true));
        if(!reader||(loaded=decode(*reader))==nullptr)return false;name=child->getStringAttribute("name");
    }
    const juce::ScopedLock serialise(loadLock);publishSample(std::move(loaded),std::move(wav),name);
    state.replaceState(tree);const int savedProgram=xml->getIntAttribute("program");program.store(juce::jlimit(0,127,xml->getIntAttribute("bankRevision")<2?(savedProgram>=8&&savedProgram<=11?64+savedProgram-8:savedProgram==7?0:savedProgram):savedProgram));undo.clearUndoHistory();if(legacyImpact)status("Legacy sound restored. Reload a factory preset for Punch / Rumble Gain.");return true;
}
void FKickProcessor::setStateInformation(const void* data,int size) {if(!restore(data,size))status("Invalid or incompatible F-Kick state.");}
bool FKickProcessor::savePreset(const juce::File& file) {juce::MemoryBlock data;getStateInformation(data);return file.replaceWithData(data.getData(),data.getSize());}
bool FKickProcessor::loadPreset(const juce::File& file) {if(file.getSize()>maxWavBytes*2)return false;juce::MemoryBlock data;return file.loadFileAsData(data)&&restore(data.getData(),(int)data.getSize());}
bool FKickProcessor::exportWav(const juce::File& file,int note,int velocity) {
    const auto s=settings();juce::MemoryBlock wav;{const juce::ScopedLock lock(sampleLock);wav=sampleWav;}
    std::unique_ptr<fkick::Sample> source;
    if(!wav.isEmpty()){juce::WavAudioFormat format;std::unique_ptr<juce::AudioFormatReader> reader(format.createReaderFor(new juce::MemoryInputStream(wav,false),true));if(reader)source=decode(*reader);}
    const double rate=48000;fkick::Engine offline;offline.prepare(rate);offline.setSample(source.get());
    const int held=int(s.length*rate),tail=int(juce::jmax(s.mode==1?double(s.release):0.,fkick::TechnoFX::tailSeconds(s.fx,s.motion))*rate),total=held+tail+512;
    juce::AudioBuffer<float> audio(2,total);audio.clear();offline.noteOn(juce::jlimit(0,127,note),1,juce::jlimit(1,127,velocity)/127.f,s);
    offline.render(audio,0,held,s);offline.noteOff(note,1,s);offline.render(audio,held,total-held,s);
    // İsteğe bağlı yüksek son Amp noktası veya uzun DC kuyruğu WAV sınırında tıklama üretmez.
    const int fade=juce::jmin(240,total);
    for(int c=0;c<2;++c)for(int i=0;i<fade;++i)audio.getWritePointer(c)[total-fade+i]*=float(fade-1-i)/float(juce::jmax(1,fade-1));
    auto out=file.createOutputStream();if(!out)return false;out->setPosition(0);out->truncate();juce::WavAudioFormat format;
    std::unique_ptr<juce::AudioFormatWriter> writer(format.createWriterFor(out.get(),rate,2,24,{},0));if(!writer)return false;out.release();return writer->writeFromAudioSampleBuffer(audio,0,total);
}
void FKickProcessor::exportWavAsync(const juce::File& file) {status("Rendering WAV...");worker.addJob([this,file]{status(exportWav(file)?"WAV exported: "+file.getFileName():"WAV export failed.");});}
juce::AudioProcessorEditor* FKickProcessor::createEditor() {return new FKickEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {return new FKickProcessor();}

