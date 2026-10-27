// Geliştirici: Fazıl Irak - Son düzenleme: 27.10.2026
// Dosyanın görevi: Rumble, echo, oda ve eski modülasyon verilerinin ses işleme katmanı.
// Copyright (c) 2026 Fazıl Irak - Özgün F-Kick uygulama kodu.
// SPDX-License-Identifier: AGPL-3.0-or-later
// Fazıl Irak · 27.10.2026 · Ring/comb tamponları yalnız prepare'da ayrılır; process kilitsiz ve tahsissizdir.
#include "TechnoFX.h"
#include <cmath>
namespace fkick {
float Motion::wave(int shape,double p,float random) noexcept {
    p-=std::floor(p);
    switch(shape){case 1:return float(1-4*std::abs(p-.5));case 2:return float(2*p-1);case 5:return float(1-2*p);case 3:return p<.5?1.f:-1.f;case 4:return random;default:return float(std::sin(p*juce::MathConstants<double>::twoPi));}
}
float Motion::envelope(const MotionSettings& s,double age) noexcept {
    const double t=age-s.delay;if(t<0)return 0;
    const double a=juce::jmax(0.f,s.attack),d=juce::jmax(.005f,s.decay);
    const double exponent=std::exp2(s.curve*2.f);
    if(a>0&&t<a)return float(std::pow(t/a,exponent));
    return t<a+d?float(std::pow(juce::jlimit(0.,1.,1-(t-a)/d),exponent)):0;
}
double Motion::lfoHz(const MotionSettings& s) noexcept {constexpr double beats[]={.25,.5,1,2,4};return s.sync?juce::jlimit(40.,300.,s.bpm)/60/beats[juce::jlimit(0,4,s.division)]:juce::jlimit(.05f,20.f,s.hz);}
MotionFrame Motion::tick(const MotionSettings& s) noexcept {
    MotionFrame frame;frame.lfo=wave(s.shape,phase+s.phase/360.,random);frame.ad=envelope(s,age);
    for(int i=0;i<modTargetCount;++i)frame.amount[(size_t)i]=frame.lfo*s.lfo[(size_t)i]+frame.ad*s.ad[(size_t)i];
    phase+=lfoHz(s)/rate;if(phase>=1){phase-=std::floor(phase);seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;random=float(seed&0xffffff)/8388607.5f-1;}
    age=juce::jmin(1e9,age+1/rate);return frame;
}
// Fazıl Irak: Delay/reverb tamponlarını ses başlamadan ayırır; render sırasında kapasite büyütülmez.
void TechnoFX::prepare(double sr){
    bassEQ.prepare(sr);rate=sr;constexpr double times[]={.0297,.0371,.0411,.0437};
    for(int c=0;c<2;++c){echo[(size_t)c].assign((size_t)std::ceil(sr*2.4)+4,0);
        for(int k=0;k<4;++k)combs[(size_t)c][(size_t)k].data.assign((size_t)juce::jmax(2,int(sr*(times[k]+c*.0011))),0);}
    for(auto* s:{&delaySamples,&rumbleGain,&echoGain,&hpCutoff,&returnGain,&roomLevel,&scMix,&scShift,&wetHP,&wetLP})s->reset(sr,.02);
    roomVerb.setSampleRate(sr);juce::Reverb::Parameters rp;rp.roomSize=.25f;rp.damping=.65f;rp.wetLevel=1;rp.dryLevel=0;rp.width=.85f;rp.freezeMode=0;roomVerb.setParameters(rp);
    // JUCE'nin ilk parametre rampasını prepare'da sıfırlarla bitir; room ilk 10 ms'de kuru sinyal sızdırmasın.
    for(int i=0;i<int(std::ceil(sr*.012))+1;++i){float l=0,r=0;roomVerb.processStereo(&l,&r,1);}
    scPole=std::exp(-1/(sr*.002));
    reset();
}
void TechnoFX::reset() noexcept {
    for(auto& channel:combs)for(auto& comb:channel){std::fill(comb.data.begin(),comb.data.end(),0.f);comb.index=0;comb.damping=0;}
    for(auto& data:echo)std::fill(data.begin(),data.end(),0.f);
    echoIndex=0;echoLow={};rumbleLow={};rumbleLow2={};rumbleHighLow={};rumbleHighLow2={};hp1={};hp2={};
    lastDecay=-1;duckEnvelope=0;killGain=1;panicking=false;
    bassEQ.reset();roomVerb.reset();roomWasOn=false;hostClock=false;scPhase=0;scGain=1;wetHP1={};wetHP2={};wetLP1={};wetLP2={};
    roomLevel.setCurrentAndTargetValue(0);scMix.setCurrentAndTargetValue(0);scShift.setCurrentAndTargetValue(0);wetHP.setCurrentAndTargetValue(0);wetLP.setCurrentAndTargetValue(20000);
    delaySamples.setCurrentAndTargetValue(float(rate*.222));rumbleGain.setCurrentAndTargetValue(0);echoGain.setCurrentAndTargetValue(0);hpCutoff.setCurrentAndTargetValue(0);returnGain.setCurrentAndTargetValue(1);
}
void TechnoFX::trigger() noexcept {if(panicking){const auto clock=hostClock;const auto phase=scPhase;reset();hostClock=clock;scPhase=phase;}if(!hostClock)scPhase=0;duckEnvelope=1;}
double TechnoFX::echoSeconds(const FXSettings& s,double bpm) noexcept {constexpr double beats[]={.25,.5,.75,1,1.5};return s.echoSync?60/juce::jlimit(40.,300.,bpm)*beats[juce::jlimit(0,4,s.echoDivision)]:juce::jlimit(.01f,1.5f,s.echoTime);}
double TechnoFX::tailSeconds(const FXSettings& s,const MotionSettings& m) noexcept {
    const bool r=s.rumbleMix>0||m.lfo[RumbleMixMod]!=0||m.ad[RumbleMixMod]!=0;
    const bool e=s.echoMix>0||m.lfo[EchoMixMod]!=0||m.ad[EchoMixMod]!=0;
    const double decay=r||(e&&s.echoSource==1)?s.rumbleDecay*1.5:0;
    const double feedback=juce::jlimit(.01,.85,double(s.feedback+(std::abs(m.lfo[FeedbackMod])+std::abs(m.ad[FeedbackMod]))*.4));
    return juce::jlimit(0.,120.,juce::jmax(s.room?2.5:0.,decay+(e?echoSeconds(s,m.bpm)*std::log(.001)/std::log(feedback):0)));
}
double TechnoFX::distort(double x,float db,int type) noexcept {
    if(db<=.001f)return x;const double gain=std::pow(10.,db/20.);
    if(type==1)return juce::jlimit(-1.,1.,x*gain);
    if(type==2){const double a=std::fmod(x*gain+1,4.);return 1-std::abs((a<0?a+4:a)-2);}
    return std::tanh(x*gain)/std::tanh(gain);
}
void TechnoFX::begin(const FXSettings& s,const MotionSettings& m) noexcept {
    if(s.rumbleDecay!=lastDecay){lastDecay=s.rumbleDecay;for(auto& channel:combs)for(auto& c:channel)c.feedback=float(std::pow(.001,(c.data.size()/rate)/juce::jmax(.2f,s.rumbleDecay)));}
    duckCoefficient=float(std::exp(-1/(rate*juce::jmax(.02f,s.recover))));
    delaySamples.setTargetValue(float(echoSeconds(s,m.bpm)*rate));rumbleGain.setTargetValue(s.rumbleMix);echoGain.setTargetValue(s.echoMix);hpCutoff.setTargetValue(s.hp);returnGain.setTargetValue(juce::Decibels::decibelsToGain(s.rumbleGain));
    if(s.room>0&&!roomWasOn)roomVerb.reset();roomWasOn=s.room>0;roomLevel.setTargetValue(juce::jlimit(0.f,1.f,s.room)*.28f);
    bassEQ.begin(s.bassLift,s.bassQ);
    scMix.setTargetValue(s.scMix);scShift.setTargetValue(s.scShift);wetHP.setTargetValue(s.fxHP);wetLP.setTargetValue(s.fxLP);scStep=juce::jlimit(40.,300.,m.bpm)/60/rate;
}
std::array<double,2> TechnoFX::process(double left,double right,const FXSettings& s,const MotionFrame& m) noexcept {
    if(echo[0].empty())return {left,right}; // Hatalı host çağrısında prepare öncesi sıfır uzunluklu ring'e girme.
    const float mix=scMix.getNextValue(),shift=scShift.getNextValue();
    if(mix>0){const double target=1-mix+mix*FXShaper::curve(s.scShape,scPhase,shift);scGain=target+scPole*(scGain-target);}else scGain=1;
    scPhase=FXShaper::wrap(scPhase+scStep);
    const float roomGain=roomLevel.getNextValue();float roomL=float(left),roomR=float(right);if(roomGain>0)roomVerb.processStereo(&roomL,&roomR,1);
    const float wetH=wetHP.getNextValue(),wetL=wetLP.getNextValue();const bool highOn=wetH>1,lowOn=wetL<19999;
    const double hG=highOn?std::tan(juce::MathConstants<double>::pi*juce::jlimit(1.,rate*.4,double(wetH))/rate):0,hH=hG/(1+hG);
    const double lG=lowOn?std::tan(juce::MathConstants<double>::pi*juce::jlimit(40.,rate*.4,double(wetL))/rate):0,lH=lG/(1+lG);
    const float wetGain=returnGain.getNextValue();
    const float rg=juce::jlimit(0.f,1.f,rumbleGain.getNextValue()+m.amount[RumbleMixMod]);
    const float eg=juce::jlimit(0.f,1.f,echoGain.getNextValue()+m.amount[EchoMixMod]);
    const float rumbleLP=juce::jlimit(40.f,12000.f,s.rumbleLP*std::exp2(m.amount[RumbleLPMod]*4));
    const float rumbleDrive=juce::jlimit(0.f,36.f,s.rumbleDrive+m.amount[RumbleDriveMod]*24);
    const float fb=juce::jlimit(0.f,.85f,s.feedback+m.amount[FeedbackMod]*.4f);
    const double lp=1-std::exp(-juce::MathConstants<double>::twoPi*rumbleLP/rate),hp=1-std::exp(-juce::MathConstants<double>::twoPi*s.rumbleHP/rate);
    const double tone=1-std::exp(-juce::MathConstants<double>::twoPi*juce::jmin(double(s.echoLP),rate*.4)/rate);
    const double time=juce::jlimit(1.,double(echo[0].size()-2),double(delaySamples.getNextValue()));
    double pos=double(echoIndex)-time;while(pos<0)pos+=echo[0].size();const size_t p0=(size_t)pos,p1=(p0+1)%echo[0].size();const float frac=float(pos-p0);
    const float duck=1-s.duck*duckEnvelope;duckEnvelope*=duckCoefficient;
    if(panicking)killGain=juce::jmax(0.f,killGain-float(1/(rate*.005)));
    float masterHP=hpCutoff.getNextValue();if(m.amount[HPMod]!=0)masterHP=juce::jmax(20.f,masterHP)*std::exp2(m.amount[HPMod]*4);
    const bool hpOn=masterHP>1;const double hpG=std::tan(juce::MathConstants<double>::pi*juce::jlimit(5.,rate*.4,double(masterHP))/rate),h=hpG/(1+hpG);
    bassEQ.tick();
    std::array<double,2> out{};
    for(int c=0;c<2;++c){
        const double dry=c==0?left:right;float diffuse=0;
        for(auto& comb:combs[(size_t)c]){const float delayed=comb.data[comb.index];comb.damping+=.28f*(delayed-comb.damping);comb.data[comb.index]=juce::jlimit(-3.f,3.f,float(dry*.24f+comb.feedback*comb.damping));if(++comb.index==comb.data.size())comb.index=0;diffuse+=delayed*.25f;}
        auto& a=rumbleHighLow[(size_t)c];auto& b=rumbleHighLow2[(size_t)c];a+=float(hp)*(diffuse-a);float wet=diffuse-a;b+=float(hp)*(wet-b);wet-=b;
        wet=float(distort(wet,rumbleDrive,s.driveType));auto& l=rumbleLow[(size_t)c];auto& l2=rumbleLow2[(size_t)c];l+=float(lp)*(wet-l);l2+=float(lp)*(l-l2);wet=l2;
        const float delayed=juce::jmap(frac,echo[(size_t)c][p0],echo[(size_t)c][p1]);echoLow[(size_t)c]+=float(tone)*(delayed-echoLow[(size_t)c]);
        const float source=s.echoSource==0?float(dry):wet;echo[(size_t)c][echoIndex]=std::tanh(source*.7f+echoLow[(size_t)c]*fb);
        float returns=wet*rg*2.5f*wetGain+echoLow[(size_t)c]*eg;
        if(roomGain>0)returns+=(c==0?roomL:roomR)*roomGain;
        double toneReturn=returns;
        if(highOn){double v=(toneReturn-wetHP1[(size_t)c])*hH,lo=v+wetHP1[(size_t)c];wetHP1[(size_t)c]=lo+v;toneReturn-=lo;v=(toneReturn-wetHP2[(size_t)c])*hH;lo=v+wetHP2[(size_t)c];wetHP2[(size_t)c]=lo+v;toneReturn-=lo;}else {wetHP1[(size_t)c]=wetHP2[(size_t)c]=0;}
        if(lowOn){double v=(toneReturn-wetLP1[(size_t)c])*lH,lo=v+wetLP1[(size_t)c];wetLP1[(size_t)c]=lo+v;v=(lo-wetLP2[(size_t)c])*lH;lo=v+wetLP2[(size_t)c];wetLP2[(size_t)c]=lo+v;toneReturn=lo;}else {wetLP1[(size_t)c]=wetLP2[(size_t)c]=0;}
        if(highOn||lowOn)returns=float(toneReturn);if(mix>0)returns*=float(scGain);
        double combined=dry+returns*duck*killGain;
        // HP ayrı seri filtredir; Off durumunda eski ses yolunu bit düzeyinde korur.
        if(hpOn){double v=(combined-hp1[(size_t)c])*h,lo=v+hp1[(size_t)c];hp1[(size_t)c]=lo+v;combined-=lo;
            v=(combined-hp2[(size_t)c])*h;lo=v+hp2[(size_t)c];hp2[(size_t)c]=lo+v;combined-=lo;}
        out[(size_t)c]=bassEQ.process(c,combined);
    }
    if(++echoIndex==echo[0].size())echoIndex=0;return out;
}
} // namespace fkick
