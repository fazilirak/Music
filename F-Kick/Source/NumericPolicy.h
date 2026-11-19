// SPDX-License-Identifier: AGPL-3.0-or-later
// Fazıl Irak · 03.11.2026: gösterim en fazla iki ondalık; yeni kullanıcı girişleri görünen birimde yuvarlanır.
#pragma once
#include <juce_core/juce_core.h>
#include <cmath>
namespace fkick {
inline bool integerNumber(const juce::String& id) {
    return id=="bassNote"||id=="sweep"||id=="sampleTune"||id=="phase"||id=="lfoPhase"||id=="tempo"
        ||id=="length"||id=="release"||id=="recover"||id=="echoTime"||id=="bassDecay"||id=="adDelay"||id=="adAttack"||id=="adDecay"
        ||id=="cutoff"||id=="hp"||id=="fxHP"||id=="fxLP"||id=="rumbleHP"||id=="rumbleLP"||id=="echoLP"||id=="bassCutoff"||id.startsWith("ts");
}
inline bool percentNumber(const juce::String& id) {
    return id.endsWith("Duck")||id.endsWith("SC")||id=="grooveDepth"||id=="bassTone"||id.startsWith("lfo_")||id.startsWith("ad_")
        ||id=="rumbleMix"||id=="echoMix"||id=="duck"||id=="echoFeedback"||id=="room"||id=="scMix"||id=="punch"||id=="harmonics"||id=="sustain"||id=="scShift"||id=="samplePan";
}
inline float enteredNumber(const juce::String& id,float value) {
    // Mevcut preset/state/otomasyon verisi topluca yuvarlanmaz; sadece yeni arayüz girişleri adımlanır.
    const double step=integerNumber(id)?1.0:percentNumber(id)?.01:(id=="start"||id=="end")?.0001:.01;
    return float(std::round(double(value)/step)*step);
}
inline juce::String displayNumber(float value,int places) {
    return places==0?juce::String(juce::roundToInt(value)):juce::String(value,juce::jlimit(1,2,places));
}
inline juce::String hostNumber(const juce::String& id,float value) {
    auto out=displayNumber(value,integerNumber(id)?0:2);
    if(out.containsChar('.')){while(out.endsWithChar('0'))out=out.dropLastCharacters(1);if(out.endsWithChar('.'))out=out.dropLastCharacters(1);}
    return out=="-0"?juce::String("0"):out;
}
}
