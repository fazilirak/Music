// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (c) 2026 Fazıl Irak
// Geliştirici: Fazıl Irak | Oluşturma / son düzenleme: 27.10.2026
// Dosyanın görevi: DAW callback'leri, bus düzeni, otomasyon ve proje durumu sözleşmesi.
#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "TransformDSP.h"
#include "Techniques.h"
namespace ft {
class Processor final : public juce::AudioProcessor {
public:
 Processor();
 // APVTS kontrolleri host otomasyonuna ve DAW proje kaydına bağlar. Parametre kimlikleri kalıcıdır.
 juce::AudioProcessorValueTreeState state;
 TransformDSP dsp;
 // Ses iş parçacığı yazar, arayüz okur. Atomikler kilit beklemeyi önler.
 std::atomic<float> inPeak{0},outPeak{0},tempo{120};
 std::atomic<bool> sideConnected{false};
 Settings settings() const;
 void prepareToPlay(double sr,int) override {dsp.prepare(sr);}
 void releaseResources() override {}
 void reset() override {dsp.reset();}
 void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
 bool isBusesLayoutSupported(const BusesLayout&) const override;
 const juce::String getName() const override {return "F-TransForm";}
 bool acceptsMidi() const override {return false;}
 bool producesMidi() const override {return false;}
 double getTailLengthSeconds() const override {return 12;}
 bool hasEditor() const override {return true;}
 juce::AudioProcessorEditor* createEditor() override;
 // JUCE tek varsayılan program yuvası gerektirir. Preset bankası veya preset arayüzü yoktur.
 int getNumPrograms() override {return 1;}
 int getCurrentProgram() override {return 0;}
 void setCurrentProgram(int) override {}
 const juce::String getProgramName(int) override {return {};}
 void changeProgramName(int,const juce::String&) override {}
 juce::AudioProcessorParameter* getBypassParameter() const override {return state.getParameter("bypass");}
 void getStateInformation(juce::MemoryBlock&) override;
 void setStateInformation(const void*,int) override;
private:
 static juce::AudioProcessorValueTreeState::ParameterLayout layout();
};
}
