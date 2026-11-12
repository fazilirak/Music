// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 Fazıl Irak; Shaper eklemeleri/değişiklikleri.
// Fazıl Irak tarafından oluşturuldu / değiştirildi: 23.10.2026
// Kapsam: Shaper için eklenen kod ve uyarlamalar; özgün üçüncü taraf telif/lisansları korunur.
// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 13.11.2026
// Dosyanın görevi: Rack fabrika presetlerinin efekt dizilişi ve kaydedilen parametre durumlarını tanımlar.
#include "Processor.h"
#include "MultibandProcessor.h"
#include "Embedded/PanPresets.h"
#include "EnvelopePresets.h"
#include "DesignedCurves.h"
#include "DelayEffect.h"
#include "UtilityEffect.h"
#include <cmath>

namespace
{
constexpr const char *categories[]{"Ducking",           "Rhythm",      "Filter Motion", "Stereo",
                                   "Time Motion",       "Space",       "Multiband",     "Creative Racks",
                                   "Saturation Motion", "Pitch Motion"};
constexpr const char *names[10][16]{
    {"Clean Quarter", "Soft House", "Deep House", "Short Kick", "Long Kick", "Offbeat Lift", "Half Beat Pump",
     "Triplet Breath", "Gentle Bass Duck", "Sub Recovery", "Vocal Space", "Pad Breathing", "Tight Eighth",
     "Slow Swell", "Broken Pump", "Wide Pump"},
    {"Straight Eighths", "Sixteenth Chopper", "Trance Pulse", "Soft Tremolo", "Swing Chops", "Triplet Gate",
     "Half Time Pulse", "Dotted Rhythm", "Syncopated Steps", "Late Accent", "Double Hit", "Sparse Break",
     "Fading Chops", "Rising Pulse", "Rolling Rhythm", "Gate and Glide"},
    {"Warm Lowpass", "Bright Opening", "Dark Closing", "Resonant Pluck", "Slow Sweep", "Bandpass Bounce",
     "Highpass Lift", "Soft Acid", "Vowel Motion", "Phaser Drift", "Comb Ripple", "Warm Ladder",
     "Pulse Filter", "Dotted Pluck", "Stereo Sweep", "Filtered Pump"},
    {"Gentle Drift", "Wide Sine", "Slow Horizon", "Ping Pong", "Eighth Bounce", "Triplet Orbit",
     "Swing Motion", "Narrow Pulse", "Left Accent", "Right Accent", "Centre Bloom", "Wide Retreat",
     "Bass Anchored", "Air Movement", "Panned Chops", "Orbiting Space"},
    {"Half Time Bed", "Short Repeater", "Slow Tape Pull", "Beat Catch", "Stutter Eighth", "Triplet Repeat",
     "Dotted Echo", "Quick Turn", "Soft Rewind", "Backspin Tail", "Loop Nudge", "Time and Space",
     "Filtered Repeat", "Panning Repeater", "Gated Time", "Subtle Time Drift"},
    {"Small Breathing Room", "Gated Hall", "Soft Tail", "Pumping Ambience", "Rising Room", "Dark Chamber",
     "Bright Air", "Dotted Clouds", "Short Drum Space", "Vocal Halo", "Pad Cathedral", "Reverse Bloom",
     "Flutter Hall", "Stereo Mist", "Low End Dry", "Filtered Tail"},
    {"Solid Sub Moving Top", "Bass Duck Air Open", "Midrange Chops", "High Hat Motion", "Three Speed Tremolo",
     "Low Pulse High Drift", "Filtered Mids", "Air Filter", "Subtle Three Band Pan", "Wide Air Mono Bass",
     "Dark Low Bright High", "Bandwise Breathing", "Rhythmic High Space", "Midrange Room",
     "Split Stereo Gates", "Three Band Groove"},
    {"House Engine", "Trance Window", "Ambient Current", "Broken Neon", "Dusk Sequence", "Liquid Steps",
     "Vocal Underwater", "Drum Carousel", "Glass Pulse", "Slow Lift", "Night Drive", "Cloud Chopper",
     "Sideways Echo", "Open Horizon", "Strobe Garden", "Last Bar Bloom"},
    {"Quarter Heat Pump", "Breathing Tape Space", "Triode Chopper", "Transformer Staircase",
     "Pentode Drum Accents", "Soft Tube Swell", "Dark Tape Tail", "Triplet Grit Gate", "Offbeat Drive Duck",
     "Broken Distortion", "Gentle Console Glue", "Bass Growl Split", "Bright Drive Lift",
     "Parallel Crunch Room", "Slow Magnetic Orbit", "Last Beat Fire"},
    {"Vibrato Halo", "Tape Wow Space", "Octave Bounce Gate", "Fifth Arp Orbit", "Falling Fifth Room",
     "Rising Fifth Filter", "Octave Descent", "Fifth Bloom", "Minor Steps Space", "Major Steps Orbit",
     "Formant Cloud", "Talking Rhythm", "Parallel Fifth Air", "Low Octave Anchor", "Triplet Harmony",
     "Final Octave Lift"}};
// plain alt adımı: Rack fabrika presetlerinin efekt dizilişi ve kaydedilen parametre durumlarını tanımlar.
void plain(juce::AudioProcessor &m, const char *id, float v)
{
    for (auto *p : m.getParameters())
        if (auto *r = dynamic_cast<juce::RangedAudioParameter *>(p))
            if (r->paramID == id)
                r->setValueNotifyingHost(r->convertTo0to1(v));
}
// Original preset curves grouped into musical families.
// curve alt adımı: Rack fabrika presetlerinin efekt dizilişi ve kaydedilen parametre durumlarını tanımlar.
// Gövdedeki başlıca çağrılar: std::fmod → std::pow → juce::jlimit → std::cos. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
// Fazıl Irak · 13.11.2026: her yay gerçek segment eğrisi; sabit adımlı nokta örneklemesi yok.
juce::String curve(int kind,int variant,float depth=1.0f) {
    auto nodes=designedCurves::family(kind,variant);
    if(kind==4)for(auto& n:nodes)n.v=1.-n.v; // Time motoru doğrudan saklanan Y'yi okur.
    return designedCurves::encode(nodes,depth,kind==3);
}
// shape alt adımı: Rack fabrika presetlerinin efekt dizilişi ve kaydedilen parametre durumlarını tanımlar.
// Gövdedeki başlıca çağrılar: engine.getStateInformation → juce::AudioProcessor::getXmlFromBinary → state.getData → state.getSize. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
void shape(juce::AudioProcessor &engine, int kind, int variant, float depth = 1)
{
    juce::MemoryBlock state;
    engine.getStateInformation(state);
    if (auto xml = juce::AudioProcessor::getXmlFromBinary(state.getData(), (int)state.getSize()))
    {
        // Reverb her düğümde beşinci alan olarak clearsTails saklar; Gate/Time/Filter dört alan kullanır.
        const bool reverb=xml->hasAttribute("sendpattern0");
        for(int i=0;i<12;++i) {
            xml->setAttribute("pattern"+juce::String(i),reverb?
                designedCurves::encode(designedCurves::family(kind,variant),depth,false,true):curve(kind,variant,depth));
            if(reverb)xml->setAttribute("sendpattern"+juce::String(i),
                designedCurves::encode(designedCurves::space(variant),1,false,true));
        }
        if(reverb)xml->setAttribute("currsendpattern",1);
        xml->setAttribute("currpattern", 1);
        xml->setAttribute("currentProgram", 0);
        xml->setAttribute("outputCV", false);
        juce::AudioProcessor::copyXmlToBinary(*xml, state);
        engine.setStateInformation(state.getData(), (int)state.getSize());
    }
    plain(engine, "trigger", 0);
    plain(engine, "pattern", 1);
    plain(engine, "phase", 0);
    plain(engine, "tension", 0);plain(engine, "tensionatk",0);plain(engine,"tensionrel",0);
    plain(engine, "min", 0);plain(engine,"max",1);
    plain(engine, "mix", 1);
    plain(engine, "sync",
          variant % 5 == 0   ? 8
          : variant % 5 == 1 ? 7
          : variant % 5 == 2 ? 9
          : variant % 5 == 3 ? 14
                             : 10);
    plain(engine, "attack", 0.003f);
    plain(engine, "release", 0.008f);
}
} // namespace
// factoryPresetName alt adımı: Rack fabrika presetlerinin efekt dizilişi ve kaydedilen parametre durumlarını tanımlar.
juce::String ShaperProcessor::factoryPresetName(int i)
{
    if (i < 0 || i >= factoryPresetCount)
        return {};
    return juce::String(i + 1).paddedLeft('0', 3) + "  " + categories[i / 16] + " / " + names[i / 16][i % 16];
}
// loadFactoryPreset: Rack fabrika presetlerinin efekt dizilişi ve kaydedilen parametre durumlarını tanımlar. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
// Gövdedeki başlıca çağrılar: lock → getCallbackLock → setStateInformation → initialState.getData. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
// Eşzamanlılık sınırı: bu kilidin hangi thread’de alındığını kontrol edin; audio callback’e yeni bekleyen kilit taşımayın.
bool ShaperProcessor::loadFactoryPreset(int index)
{
    if (index < 0 || index >= factoryPresetCount)
        return false;
    const juce::ScopedLock lock(getCallbackLock());
    setRackStateInformation(initialState.getData(), (int)initialState.getSize());
    int group = index / 16, v = index % 16;
    auto engine = [&](int e) -> juce::AudioProcessor &
    {
        if (auto *m = dynamic_cast<MultibandProcessor *>(&module(e)))
            return m->band(1);
        return module(e);
    };
    auto add = [&](int e, int kind, float depth = 1.0f)
    {
        addEffect(e);
        shape(engine(e), kind, v, depth);
        if (kind == 0)
            plain(engine(e), "sync",
                  v == 6 || v == 12    ? 6
                  : v == 7             ? 14
                  : v == 11            ? 8
                  : v == 13 || v == 14 ? 9
                                       : 7);
        if (kind == 1)
            plain(engine(e), "sync", 9);
    };
    auto filter = [&]
    {
        constexpr int types[]{0, 0, 1, 5, 1, 3, 0, 7, 15, 8, 16, 11, 2, 5, 3, 4};
        add(2, 2);
        plain(engine(2), "ftype", (float)types[v]);
        plain(engine(2), "fmode", v == 5 ? 1.0f : v == 6 ? 2.0f : 0.0f);
        plain(engine(2), "min", 0.12f + 0.015f * v);
        plain(engine(2), "max", 0.72f + 0.015f * v);
        plain(engine(2), "fdrive", 0.04f * (v % 5));
        if (v == 4 || v == 9)
            plain(engine(2), "sync", 11);
    };
    auto pan = [&]
    {
        add(4, 3, (v == 1 || v == 3 || v == 4 ? 1.0f : v == 0 || v == 7 ? 0.35f : 0.7f));
        plain(engine(4), "min", 0);
        plain(engine(4), "max", 1);
        constexpr int periods[]{11,9,11,8,8,9,9,8,9,9,10,10,11,11,9,11};
        plain(engine(4),"sync",(float)periods[v]);
    };
    auto reverb = [&]
    {
        add(3, v == 1 || v == 12 ? 1 : 5, 0.6f);
        plain(engine(3), "drywet", 0.12f + 0.018f * v);
        plain(engine(3), "predelay", (float)(8 * (v % 6)));
        plain(engine(3), "irstretch", -0.3f + 0.035f * v);
        plain(engine(3), "irhighcut", v == 5 ? 3500.0f : 5000.0f + 500 * v);
        if (v == 11)
        {
            plain(engine(3), "irreverse", 1);
            plain(engine(3), "irattack", 0.2f);
        }
    };
    auto time = [&]
    {
        add(1, 4);
        plain(engine(1), "mix", v == 0 ? 1.0f : 0.25f + 0.03f * v);
        plain(engine(1), "min", 0);
        plain(engine(1), "max", 1);
        plain(engine(1), "sync", v == 1 || v == 7 ? 7 : v == 5 ? 14 : 9);
    };
    auto delay = [&](bool dotted=false) {
        addEffect(10);auto& d=dynamic_cast<DelayEffect&>(engine(10));
        shape(*d.curve,5,v,1);plain(*d.curve,"sync",9);
        plain(d,"division",dotted?9.f:4.f);plain(d,"mode",v%2?1.f:0.f);
        plain(d,"send_curve",1);plain(d,"send_min",0);plain(d,"send_max",.75f);
        plain(d,"blend_curve",0);plain(d,"blend_max",.25f);
        plain(d,"feedback_curve",1);plain(d,"feedback_min",.22f);plain(d,"feedback_max",.48f);
        plain(d,"filter_curve",1);plain(d,"filter_min",.32f);plain(d,"filter_max",.76f);
    };
    auto width = [&] {
        addEffect(8);auto& w=dynamic_cast<UtilityEffect&>(engine(8));
        shape(*w.curve,3,v,.7f);plain(w,"minimum",.7f);plain(w,"maximum",1.35f);
    };
    switch (group)
    {
    case 0:
        // Derinlik müzikal göreve göre seçilir; Clean/Short/Long/Sub tam dip yapar.
        {constexpr float depths[]{1,.85f,.95f,1,1,.92f,1,.96f,.35f,1,.7f,.6f,1,.85f,.95f,.7f};add(5,0,depths[v]);}
        if (v == 10 || v == 15)
            pan();
        break;
    case 1:
        add(0,1,v==3?.55f:1.f);
        if (v == 15)
            filter();
        break;
    case 2:
        filter();
        if (v == 15)
            add(5, 0, 0.65f);
        break;
    case 3:
        pan();
        if (v == 14)
            add(0, 1, 0.5f);
        if (v == 15)
            reverb();
        break;
    case 4:
        time();
        if (v == 11)
            reverb();
        if (v == 12)
            filter();
        if (v == 13)
            pan();
        if (v == 14)
            add(0, 1, 0.6f);
        break;
    case 5:
        reverb();
        if (v == 15)
            filter();
        break;
    case 6:
    {
        int e = v == 6 || v == 7 || v == 10 ? 2 : v == 8 || v == 9 ? 4 : v == 12 || v == 13 ? 3 : 0;
        addEffect(e);
        auto &mb = dynamic_cast<MultibandProcessor &>(module(e));
        for (int b = 0; b < 3; ++b)
        {
            shape(mb.band(b),
                  e == 4   ? 3
                  : e == 2 ? 2
                  : e == 3 ? 5
                           : 1,
                  (v + b * 3) % 16, b == 0 ? 0.12f : 0.45f + 0.15f * b);
            plain(mb.band(b), "sync", (float)(8 + b));
            if (e == 3)
                plain(mb.band(b), "drywet", b == 0 ? 0.0f : 0.12f + 0.08f * b);
            if (e == 4 && b == 0)
            {
                plain(mb.band(b), "min", 0.5f);
                plain(mb.band(b), "max", 0.5f);
            }
        }
        mb.setMultiband(true);
        mb.low->setValueNotifyingHost(mb.low->convertTo0to1(160.0f + v * 12));
        mb.high->setValueNotifyingHost(mb.high->convertTo0to1(2400.0f + v * 140));
        mb.selectedBand = 1;
        break;
    }
    case 7:
        if (v % 4 == 0)
        {
            add(5, 0, 0.7f);
            filter();
            pan();
        }
        else if (v % 4 == 1)
        {
            add(0, 1, 0.65f);
            time();
            reverb();
        }
        else if (v % 4 == 2)
        {
            filter();
            reverb();
            pan();
        }
        else
        {
            time();
            filter();
            add(0, 1, 0.5f);
            pan();
        }
        if (v >= 8)
            moveEffect(2, 0);
        state.getParameter("mix")->setValueNotifyingHost(0.7f + 0.015f * v);
        break;
    case 8:
    case 9:
    {
        const int effect = group == 8 ? 6 : 7;
        addEffect(effect);
        engine(effect).setCurrentProgram(24 + v);
        // Preserve dry bass in the dedicated band preset; the other racks
        // start full-range and add complementary effects in palette order.
        if ((group == 8 && v == 11) || (group == 9 && v == 13))
        {
            auto &mb = dynamic_cast<MultibandProcessor &>(module(effect));
            mb.setMultiband(true);
            envelopePresets::set(mb.band(0), "mix", 0);
            mb.low->setValueNotifyingHost(mb.low->convertTo0to1(180));
            mb.high->setValueNotifyingHost(mb.high->convertTo0to1(20000));
            mb.selectedBand = 1;
        }
        if (v % 4 == 1 || v == 10)
            reverb();
        if (v % 4 == 2)
            add(0, 1, .45f);
        if (v % 4 == 3)
            pan();
        if (v == 5 || v == 12)
            filter();
        if (v == 8)
            add(5, 0, .6f);
        if (v == 15)
        {
            reverb();
            plain(engine(3), "drywet", .2f);
        }
        break;
    }
    }
    // Seçilmiş rack'lerde stereo/echo hareketi diğer modüllere cevap verir; her rack'e efekt yığılmaz.
    if(group==3&&(v==1||v==10||v==11||v==15))width();
    if(group==5&&(v==7||v==13))delay(v==7);
    if(group==7&&(v==2||v==12||v==15))delay(v!=2);
    if(group==7&&(v==2||v==9||v==13))width();
    auto installed = palette();
    selectedEffect = installed.empty() ? -1 : installed.front();
    rackPresetName = factoryPresetName(index);
    factoryPresetIndex = index;
    return true;
}
