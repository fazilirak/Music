// SPDX-License-Identifier: GPL-3.0-only
// Fazıl Irak · 12.11.2026: QDelay ring buffer / ZDF filtresi üzerinde eğri kontrollü stereo delay.
#pragma once
#include <JuceHeader.h>
#include "ForwardParameter.h"
#include "DelayLine.h"
#include "SVF.h"
class DelayEffect final : public juce::AudioProcessor {
public:
    DelayEffect(); ~DelayEffect() override;
    std::unique_ptr<juce::AudioProcessor> curve;
    juce::AudioProcessorValueTreeState state;
    void prepareToPlay(double,int) override;
    void releaseResources() override {curve->releaseResources();}
    void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
    const juce::String getName() const override{return "Delay";}
    bool acceptsMidi() const override{return true;}
    bool producesMidi() const override{return false;}
    bool isMidiEffect() const override{return false;}
    double getTailLengthSeconds() const override;
    bool hasEditor() const override{return true;}
    juce::AudioProcessorEditor* createEditor() override;
    bool isBusesLayoutSupported(const BusesLayout& l) const override{return l.getMainInputChannelSet()==juce::AudioChannelSet::stereo()&&l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo();}
    int getNumPrograms() override{return 8;}
    int getCurrentProgram() override{return program;}
    const juce::String getProgramName(int) override;
    void setCurrentProgram(int) override;
    void changeProgramName(int,const juce::String&) override{}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*,int) override;
private:
    struct TargetValues {std::atomic<float>* lo;std::atomic<float>* hi;std::atomic<float>* depth;std::atomic<float>* smoothing;std::atomic<float>* enabled;};
    std::array<TargetValues,4> values;
    std::array<std::atomic<float>*,7> common;
    static juce::AudioProcessorValueTreeState::ParameterLayout layout();
    std::array<jellydelay::DelayLine,2> lines;
    std::array<jellydelay::SVF,2> filters;
    juce::AudioBuffer<float> cv,dryDelay;
    juce::MidiBuffer events;
    std::array<juce::SmoothedValue<float>,4> target;
    juce::SmoothedValue<float> time,mix,gain,width;
    double rate=48000.; int capacity=0,lookahead=0,write=0,program=0;
    float value(const juce::String& id) const{return state.getRawParameterValue(id)->load();}
};
