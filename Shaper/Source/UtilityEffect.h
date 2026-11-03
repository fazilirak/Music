// SPDX-License-Identifier: GPL-3.0-only
// Fazıl Irak · 03.11.2026
// Width ve tek VST3 barındıran FX: aynı Gate eğrisi, ayrı ses motoru.
#pragma once
#include <JuceHeader.h>
#include "ForwardParameter.h"

class UtilityEffect final : public juce::AudioProcessor {
public:
    explicit UtilityEffect(bool external);
    ~UtilityEffect() override;
    const bool external;
    std::unique_ptr<juce::AudioProcessor> curve;
    juce::AudioProcessorValueTreeState state;
    juce::String loadPlugin(const juce::File&);
    juce::String pluginName();
    bool setHostedParameter(const juce::String&,float);
    void showPluginEditor();
    void closePluginEditor();
    void prepareToPlay(double,int) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
    bool isBusesLayoutSupported(const BusesLayout& l) const override {return l.getMainInputChannelSet()==juce::AudioChannelSet::stereo()&&l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo();}
    const juce::String getName() const override {return external?"FX":"Width";}
    bool acceptsMidi() const override{return true;}
    bool producesMidi() const override{return false;}
    bool isMidiEffect() const override{return false;}
    double getTailLengthSeconds() const override{return external?30.:0.;}
    bool hasEditor() const override{return true;}
    juce::AudioProcessorEditor* createEditor() override;
    int getNumPrograms() override{return 8;}
    int getCurrentProgram() override{return program;}
    const juce::String getProgramName(int) override;
    void setCurrentProgram(int) override;
    void changeProgramName(int,const juce::String&) override{}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*,int) override;
    // Sentetik testlerde de aynı Mid/Side dönüşümü kullanılır. Mono toplamı korunur.
    static std::pair<float,float> widthSample(float l,float r,float width) {
        const float mid=.5f*(l+r),side=.5f*(l-r)*width;return {mid+side,mid-side};
    }
private:
    juce::AudioPluginFormatManager formats;
    juce::CriticalSection pluginLock;
    std::unique_ptr<juce::AudioPluginInstance> plugin;
    std::unique_ptr<juce::DocumentWindow> pluginWindow;
    juce::PluginDescription description;
    juce::String status{"No VST3 loaded"};
    juce::AudioBuffer<float> cv,wet,delay,curveDelay;
    juce::MidiBuffer events;
    juce::SmoothedValue<float> motion,outputMix,outputGain,outputWidth;
    double sampleRate=48000.;
    int capacity=0,write=0,curveWrite=0,lookahead=0,program=0;
    static juce::AudioProcessorValueTreeState::ParameterLayout layout(bool);
};
