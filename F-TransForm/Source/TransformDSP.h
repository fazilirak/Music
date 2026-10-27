// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (c) 2026 Fazıl Irak
// Geliştirici: Fazıl Irak | Oluşturma / son düzenleme: 27.10.2026
// Dosyanın görevi: Yedi makronun ortak efekt motoru, gecikme hafızası ve geçiş yumuşatması.
#pragma once
#include <juce_dsp/juce_dsp.h>
#include "Techniques.h"
namespace ft {
struct Settings { int technique=0, character=2; float intensity=0; bool bypass=false; };
class TransformDSP {
public:
 void prepare(double sampleRate);
 void reset();
 // İkinci bus eski host düzeni için ayrılmıştır; yeni makrolar ikinci kaynak gerektirmez.
 void process(juce::AudioBuffer<float>&,const juce::AudioBuffer<float>* sidechain,
              Settings,double bpm=120,double ppq=-1);
private:
 static constexpr int recipeFields=11,hilbertLength=63;
 double sr=48000,beat=0,shiftPhase=0,sweepPhase=0;
 int echoWrite=0,hilbertWrite=0,controlCounter=0;
 unsigned rng=0x735ad12u;
 juce::AudioBuffer<float> echo;
 juce::Reverb reverb;
 juce::dsp::StateVariableTPTFilter<float> highpass,reverbHighpass,outputLowpass;
 // İki allpass kolu yarım çevrim farkıyla yükselir; birbirini sarma bölgesinde devralır.
 float phaseMemory[2][2][6]{},phaserCoefficient[2][6]{};
 float analyticHistory[2][64]{},hilbertCoefficients[hilbertLength]{};
 float shiftSmooth[2]{},feedbackLow[2]{},noiseLow[2]{},noiseBass[2]{},dcIn[2]{},dcOut[2]{};
 double tonePhases[8]{};
 float inputEnvelope=0,delayTime=0,delayFrom=0,delayTarget=0,delayFade=1;
 juce::SmoothedValue<float> amount,hardness,enabled;
 std::array<juce::SmoothedValue<float>,recipeFields> recipeValues;
 float readEcho(int channel,float delay) const;
 float lowpass(float x,float hz,float& state) const;
 float phaser(float x,int channel) noexcept;
 float shift(float x,int channel,double oscillatorPhase) noexcept;
 static std::array<float,recipeFields> values(const Recipe&);
};
}

