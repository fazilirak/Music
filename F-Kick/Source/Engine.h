// Geliştirici: Fazıl Irak - Son düzenleme: 27.10.2026
// Dosyanın görevi: Sample oynatma, kick gövdesi, ses yönetimi ve MIDI olaylarının gerçek zamanlı işlenmesi.
// Copyright (c) 2026 Fazıl Irak - Özgün F-Kick uygulama kodu.
// SPDX-License-Identifier: AGPL-3.0-or-later
// Fazıl Irak · 27.10.2026 · Özgün sample + kick motoru; callback'te I/O ve kilit yoktur.
#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <array>
#include "TechnoFX.h"

namespace fkick {
constexpr int maxPoints = 8;
struct Curve {
    int count = 5;
    std::array<float, maxPoints> x{}, y{}, bend{};
    float at(float t) const noexcept;
    void sanitise() noexcept;
};
struct Sample {
    juce::AudioBuffer<float> audio;
    double rate = 48000;
};
struct Settings {
    int mode = 0;
    bool gate = false, loop = false, reverse = false, poly = false, trackTail = false;
    float sampleGain = .6f, sampleTune = 0, samplePan = 0, bodyGain = .65f;
    float root = 50, sweep = 48, phase = 0, harmonics = 0, length = .5f;
    float punch = 0;
    float attack = .001f, release = .08f, sustain = .8f;
    float cutoff = 18000, resonance = .7f, drive = 0, output = .7f, start = 0, end = 1;
    Curve amp, pitch;
    FXSettings fx;
    MotionSettings motion;
};

class Engine {
public:
    void prepare(double rate) noexcept;
    void reset() noexcept;
    void setSample(const Sample* next) noexcept;
    void noteOn(int note, int channel, float velocity, const Settings&) noexcept;
    void noteOff(int note, int channel, const Settings&) noexcept;
    void pitchWheel(int channel, float semitones) noexcept { bend[(size_t) juce::jlimit(1,16,channel)-1] = semitones; }
    void sustainPedal(int channel, bool down, const Settings&) noexcept;
    void panic() noexcept;
    void render(juce::AudioBuffer<float>&, int start, int count, const Settings&) noexcept;
    int activeVoices() const noexcept;
    float lfoValue() const noexcept {return lastMotion.lfo;}
    float adValue() const noexcept {return lastMotion.ad;}
    float sidechainPhase() const noexcept {return effects.sidechainPhase();}
    void syncFX(double beat,bool running) noexcept {effects.syncBeat(beat,running);}
private:
    struct Voice {
        bool active = false, held = false, sustained = false, releasing = false, panic = false;
        int note = 60, channel = 1;
        uint64_t serial = 0;
        double age = 0, position = 0, phase = 0;
        float velocity = 1, releaseGain = 1, gain = 0, stealL = 0, stealR = 0;
        float lastL = 0, lastR = 0;
        int stealFade = 0;
    };
    std::array<Voice,16> voices{};
    std::array<float,16> bend{};
    std::array<bool,16> pedal{};
    const Sample* sample = nullptr;
    double rate = 48000;
    double dcPole = .9993457;
    uint64_t serial = 0;
    bool primed = false;
    std::array<double,2> ic1{}, ic2{}, dcX{}, dcY{};
    juce::SmoothedValue<float> sampleLevel, bodyLevel, cutoff, drive, output, punchAmount;
    TechnoFX effects;
    Motion motion;
    MotionFrame lastMotion;
    void beginRelease(Voice&) noexcept;
};
} // namespace fkick
