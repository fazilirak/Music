// Fazıl Irak · 03.11.2026 · Özgün Classic/Groove/Hybrid başlangıç sesleri.
// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "PluginProcessor.h"
namespace fkick {
inline constexpr const char* rumbleRecipeNames[]={
    "Classic - Concrete Hall","Classic - Low Fog","Classic - Copper Dub","Classic - Short Cell",
    "Groove - Offbeat Tunnel","Groove - Rolling Foundry","Groove - Broken Orbit","Groove - Syncopated Dust",
    "Hybrid - Sub Runner","Hybrid - Fifth Avenue","Hybrid - Driving Current","Hybrid - Octave Transit"};
inline void applyRumbleRecipe(FKickProcessor& p,int index) {
    index=juce::jlimit(0,11,index);const int mode=index/4,v=index%4;
    p.setCurrentProgram(64+v*3);
    auto set=[&](const char* id,float x){p.setParameter(id,x);};
    set("sampleGain",-60);set("root",v==1?43.65f:v==2?55.f:49.f);
    set("length",mode==0?300.f+v*60.f:170.f+v*15.f);
    set("bodyGain",-8);set("output",-5);set("punch",.2f+v*.1f);
    set("rumbleMode",float(mode));set("returnRouting",mode==0?0.f:1.f);
    set("rumbleMix",.35f+v*.06f);set("rumbleDecay",mode==0?1.3f+v*.5f:2.1f);
    set("rumbleHP",mode==2?75.f:32.f+v*8);set("rumbleLP",mode==2?450.f:550.f+v*130);
    set("rumbleDrive",7.f+v*3);set("rumbleGain",0);set("duck",.85f);set("recover",90.f+v*20);
    set("echoMix",v==2?.2f:.08f);set("echoSync",1);set("echoDivision",float(v%3));set("echoFeedback",.25f);
    set("echoSource",v%2);set("room",.08f);set("scMix",mode==0?0:.4f);set("scShape",float(v));
    set("groovePattern",float(v));set("grooveDepth",.9f);set("rumbleLimit",mode==0?0:1);set("rumbleCeiling",-9);
    set("rumbleDuck",1);set("echoDuck",.6f);set("roomDuck",.4f);set("bassDuck",1);
    set("rumbleSC",1);set("echoSC",.55f);set("roomSC",.3f);set("bassSC",.65f);
    set("bassNote",v==1?29.f:v==2?33.f:31.f);set("bassLevel",-15.f);set("bassDecay",90.f+v*25);
    set("bassTone",.18f+v*.18f);set("bassCutoff",300.f+v*180);set("bassPattern",float(v));
}
}
