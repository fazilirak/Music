// Geliştirici: Fazıl Irak - Son düzenleme: 27.10.2026
// Dosyanın görevi: Host parametreleri, ses callback, durum kaydı ve kaynak sahipliği köprüsü.
// Copyright (c) 2026 Fazıl Irak - Özgün F-Kick uygulama kodu.
// SPDX-License-Identifier: AGPL-3.0-or-later
// Fazıl Irak · 27.10.2026 · Host, parametreler, taşınabilir sample state ve iş parçacığı sahipliği.
#pragma once
#include "Engine.h"

class FKickProcessor final : public juce::AudioProcessor, private juce::MidiKeyboardState::Listener {
public:
    FKickProcessor();
    ~FKickProcessor() override;
    void prepareToPlay(double,int) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override {return true;}
    const juce::String getName() const override {return "F-Kick";}
    bool acceptsMidi() const override {return true;}
    bool producesMidi() const override {return false;}
    bool isMidiEffect() const override {return false;}
    double getTailLengthSeconds() const override;
    int getNumPrograms() override {return 128;}
    int getCurrentProgram() override {return program.load();}
    void setCurrentProgram(int) override;
    const juce::String getProgramName(int) override;
    void changeProgramName(int,const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*,int) override;
    bool savePreset(const juce::File&);
    bool loadPreset(const juce::File&);
    bool loadSample(const juce::File&); // GUI bunu worker'dan çağırır; testte senkron kullanılabilir.
    void loadSampleAsync(const juce::File&);
    void clearSample();
    bool exportWav(const juce::File&,int note=60,int velocity=110);
    void exportWavAsync(const juce::File&);
    fkick::Settings settings() const noexcept;
    fkick::Curve curve(int which) const noexcept;
    void setCurve(int,const fkick::Curve&);
    void setParameter(const juce::String& id,float plain);
    float value(const juce::String& id) const;
    void requestPanic() noexcept {panicRequested.store(true);}
    struct SampleView {
        juce::String name="Drop a sample here",status;
        std::array<float,512> peaks{};
        double duration=0;
        bool loaded=false;
    };
    SampleView sampleView() const;
    std::atomic<float> meter{0};
    std::atomic<int> voiceCount{0};
    std::atomic<float> lfoMeter{0},adMeter{0};
    std::atomic<float> scPhase{0};
    std::atomic<double> hostTempo{0};
    juce::MidiKeyboardState keyboard;
    juce::UndoManager undo;
    juce::AudioProcessorValueTreeState state;
private:
    enum P {Mode,Gate,Loop,Reverse,Poly,Tracking,SampleGain,SampleTune,SamplePan,BodyGain,Root,Sweep,
        Phase,Harmonics,Length,Attack,Release,Sustain,Cutoff,Resonance,Drive,Output,Start,End,NumParams};
    std::array<std::atomic<float>*,NumParams> params{};
    enum F {HP,RMix,RDecay,RHP,RLP,RDrive,Duck,Recover,EMix,ETime,EFeedback,ELP,ESync,EDivision,ESource,DriveType,Tempo,LHz,LSync,LShape,LDivision,LPhase,LRetrigger,ADDelay,ADAttack,ADDecay,ADCurve,Punch,RumbleGain,SCShape,SCShift,SCMix,Room,FXHP,FXLP,BassLift,BassQ,NumFX};
    std::array<std::atomic<float>*,NumFX> fxParams{};
    std::array<std::array<std::atomic<float>*,fkick::modTargetCount>,2> routeParams{};
    std::array<std::array<std::atomic<float>*,1+fkick::maxPoints*3>,2> curves{};
    static juce::AudioProcessorValueTreeState::ParameterLayout layout();
    fkick::Engine engine;
    std::atomic<int> program{0};
    std::atomic<bool> panicRequested{false};
    struct KeyEvent {int note=60,channel=1;float velocity=0;bool on=false;};
    juce::AbstractFifo keyFifo{256};
    std::array<KeyEvent,256> keyEvents{};
    void handleNoteOn(juce::MidiKeyboardState*,int channel,int note,float velocity) override;
    void handleNoteOff(juce::MidiKeyboardState*,int channel,int note,float velocity) override;
    void enqueueKey(int channel,int note,float velocity,bool on) noexcept;
    // SPSC: callback yalnız ham sahiplik taşır; sample silme/ayırma worker veya host state yolundadır.
    std::atomic<fkick::Sample*> pending{nullptr};
    fkick::Sample* active=nullptr;
    juce::AbstractFifo retiredFifo{32};
    std::array<fkick::Sample*,32> retired{};
    mutable juce::CriticalSection sampleLock;
    juce::CriticalSection loadLock;
    SampleView view;
    juce::MemoryBlock sampleWav;
    juce::ThreadPool worker{1};
    void drainRetired();
    void publishSample(std::unique_ptr<fkick::Sample>,juce::MemoryBlock,const juce::String&);
    bool restore(const void*,int);
    std::unique_ptr<juce::XmlElement> makeState();
    void status(const juce::String&);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FKickProcessor)
};
