// Geliştirici: Fazıl Irak - Son düzenleme: 27.10.2026
// Dosyanın görevi: Rumble, echo, oda ve eski modülasyon verilerinin ses işleme katmanı.
// Copyright (c) 2026 Fazıl Irak - Özgün F-Kick uygulama kodu.
// SPDX-License-Identifier: AGPL-3.0-or-later
// Fazıl Irak · 27.10.2026 · Bağımsız rumble/echo yolu ve ortak LFO/AD modülasyonu.
#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <array>
#include <vector>
#include "FXShaper.h"
#include "BassEQ.h"

namespace fkick {
enum ModTarget { FilterMod,HPMod,DriveMod,RumbleMixMod,RumbleLPMod,RumbleDriveMod,EchoMixMod,FeedbackMod,PitchMod,LevelMod,modTargetCount };
inline constexpr const char* modIDs[]={"cutoff","hp","drive","rumbleMix","rumbleLP","rumbleDrive","echoMix","echoFeedback","pitch","level"};
inline constexpr const char* modNames[]={"Main low-pass","Main high-pass","Main drive","Rumble amount","Rumble low-pass","Rumble drive","Echo amount","Echo feedback","Body pitch","Output level"};
inline int modTargetFor(const juce::String& id) {if(id=="root")return PitchMod;if(id=="output")return LevelMod;for(int i=0;i<modTargetCount;++i)if(id==modIDs[i])return i;return -1;}
struct MotionSettings {
    bool sync=true,retrigger=true;
    int shape=0,division=1;
    float hz=2,phase=0,delay=0,attack=.01f,decay=.3f,curve=0;
    double bpm=135;
    std::array<float,modTargetCount> lfo{},ad{};
};
struct FXSettings {
    float hp=0,rumbleMix=0,rumbleDecay=1.8f,rumbleHP=65,rumbleLP=700,rumbleDrive=8,duck=.8f,recover=.12f;
    float rumbleGain=0;
    float echoMix=0,echoTime=.222f,feedback=.35f,echoLP=3500;
    bool echoSync=true;
    int echoDivision=1,echoSource=0,driveType=0;
    int scShape=0;
    float scShift=0,scMix=0,fxHP=0,fxLP=20000;
    float bassLift=0,bassQ=.707f;
    float room=0; // 0..1; önceki bool Room on=1 aynı wet seviyesini korur.
};
struct MotionFrame {
    float lfo=0,ad=0;
    std::array<float,modTargetCount> amount{};
};
class Motion {
public:
    void prepare(double sr) noexcept {rate=sr;reset();}
    void reset() noexcept {phase=0;age=1e9;random=0;seed=0x2468aceu;}
    void trigger(const MotionSettings& s) noexcept {age=0;if(s.retrigger)phase=0;}
    MotionFrame tick(const MotionSettings&) noexcept;
    static float wave(int shape,double phase,float random=0) noexcept;
    static float envelope(const MotionSettings&,double age) noexcept;
    static double lfoHz(const MotionSettings&) noexcept;
private:
    double rate=48000,phase=0,age=1e9;
    float random=0;
    uint32_t seed=0x2468aceu;
};
class TechnoFX {
public:
    void prepare(double sr);
    void reset() noexcept;
    void trigger() noexcept;
    void panic() noexcept {panicking=true;}
    void syncBeat(double beat,bool running) noexcept {hostClock=running;if(running)scPhase=FXShaper::wrap(beat);}
    float sidechainPhase() const noexcept {return float(scPhase);}
    void begin(const FXSettings&,const MotionSettings&) noexcept;
    std::array<double,2> process(double left,double right,const FXSettings&,const MotionFrame&) noexcept;
    static double echoSeconds(const FXSettings&,double bpm) noexcept;
    static double tailSeconds(const FXSettings&,const MotionSettings&) noexcept;
    static double distort(double x,float db,int type) noexcept;
private:
    struct Comb {std::vector<float> data;size_t index=0;float damping=0,feedback=.7f;};
    std::array<std::array<Comb,4>,2> combs;
    std::array<std::vector<float>,2> echo;
    std::array<float,2> echoLow{},rumbleLow{},rumbleLow2{},rumbleHighLow{},rumbleHighLow2{};
    std::array<double,2> hp1{},hp2{};
    std::array<double,2> wetHP1{},wetHP2{},wetLP1{},wetLP2{};
    juce::Reverb roomVerb;
    BassEQ bassEQ;
    bool roomWasOn=false,hostClock=false;
    double scPhase=0,scStep=0,scGain=1,scPole=0;
    double rate=48000;
    size_t echoIndex=0;
    float lastDecay=-1,duckEnvelope=0,duckCoefficient=.999f,killGain=1;
    bool panicking=false;
    juce::SmoothedValue<float> delaySamples,rumbleGain,echoGain,hpCutoff,returnGain;
    juce::SmoothedValue<float> roomLevel,scMix,scShift,wetHP,wetLP;
};
} // namespace fkick
