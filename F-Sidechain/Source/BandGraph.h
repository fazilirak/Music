// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 Fazıl Irak; Shaper eklemeleri/değişiklikleri.
// Fazıl Irak tarafından oluşturuldu / değiştirildi: 23.10.2026
// Kapsam: Shaper için eklenen kod ve uyarlamalar; özgün üçüncü taraf telif/lisansları korunur.
// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: Low/Mid/High bölme sınırlarının görsel hit-test ve sürükleme etkileşimini yönetir.
#pragma once
#include "MultibandProcessor.h"
#include "EffectPalette.h"

// Log-frequency crossover editor. The two edge handles start at 20 Hz / 20 kHz.
class BandGraph final : public juce::Component, public juce::SettableTooltipClient, private juce::Timer
{
  public:
    // BandGraph alt adımı: Low/Mid/High bölme sınırlarının görsel hit-test ve sürükleme etkileşimini yönetir.
    explicit BandGraph(MultibandProcessor &p) : owner(p)
    {
        setComponentID("band-frequency-graph");
        setTooltip(
            "Drag the left edge to add Low, or the right edge to add High. Click a band to edit it. Double-click an edge to remove that band.");
        startTimerHz(30);
        owner.spectrumVisible.store(true);
    }
    // ~BandGraph alt adımı: Low/Mid/High bölme sınırlarının görsel hit-test ve sürükleme etkileşimini yönetir.
    ~BandGraph() override
    {
        stopTimer();
        owner.spectrumVisible.store(false);
    }
    std::function<void()> onSelection;
    // frequencyX alt adımı: Low/Mid/High bölme sınırlarının görsel hit-test ve sürükleme etkileşimini yönetir.
    float frequencyX(float hz) const
    {
        return 8.0f +
               (getWidth() - 16.0f) * std::log(juce::jlimit(20.0f, 20000.0f, hz) / 20.0f) / std::log(1000.0f);
    }
    // Mevcut kontrol/durumun görselini çizer; ses callback’inde çağrılmamalıdır.
    // Gövdedeki başlıca çağrılar: rackEffectColour → getLocalBounds → toFloat → reduced. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    void paint(juce::Graphics &g) override
    {
        const auto colour = rackEffectColour(owner.effect);
        auto bounds = getLocalBounds().toFloat().reduced(0.5f);
        g.setColour(juce::Colour(0xff10151c));
        g.fillRoundedRectangle(bounds, 3);
        float l = frequencyX(owner.low->get()), r = frequencyX(owner.high->get());
        const float top = 23, bottom = (float)getHeight() - 18;
        auto band = [&](float a, float b, int index, bool active, const juce::String &text)
        {
            if (!active || b - a < 2)
                return;
            g.setColour(colour.withAlpha(owner.editorBand() == index ? 0.22f : 0.07f));
            g.fillRect(a, top, b - a, bottom - top);
            if (b - a > 42)
            {
                g.setColour(colour.withAlpha(owner.editorBand() == index ? 1.0f : 0.6f));
                g.setFont(juce::FontOptions(13));
                g.drawText(text, juce::Rectangle<float>(a, top, b - a, 22), juce::Justification::centred);
            }
        };
        band(8, l, 0, owner.hasLowBand(), "LOW");
        band(l, r, 1, true, owner.isMultiband() ? "MID" : "MID  /  FULL RANGE");
        band(r, (float)getWidth() - 8, 2, owner.hasHighBand(), "HIGH");
        g.setFont(juce::FontOptions(11));
        for (float hz : {20.f, 100.f, 1000.f, 10000.f, 20000.f})
        {
            auto x = frequencyX(hz);
            g.setColour(juce::Colours::white.withAlpha(0.09f));
            g.drawVerticalLine((int)x, top, bottom);
            auto label = hz >= 1000 ? juce::String(hz / 1000, 0) + "k" : juce::String(hz, 0);
            g.setColour(juce::Colours::white.withAlpha(0.5f));
            g.drawText(
                label,
                juce::Rectangle<float>(juce::jlimit(0.0f, (float)getWidth() - 40, x - 20), bottom, 40, 18),
                juce::Justification::centred);
        }
        if (haveSpectrum)
        {
            juce::Path path;
            path.startNewSubPath(8, bottom);
            for (int x = 8; x < getWidth() - 8; ++x)
            {
                const float hz = 20.0f * std::pow(1000.0f, (x - 8.0f) / (getWidth() - 16.0f));
                auto bin = juce::jlimit(
                    1, MultibandProcessor::spectrumSize / 2 - 1,
                    (int)(hz * MultibandProcessor::spectrumSize / juce::jmax(1.0, owner.getSampleRate())));
                float db = juce::Decibels::gainToDecibels(magnitudes[bin], -80.0f);
                path.lineTo((float)x, juce::jmap(juce::jlimit(-80.f, 0.f, db), -80.f, 0.f, bottom, top));
            }
            path.lineTo((float)getWidth() - 8, bottom);
            path.closeSubPath();
            g.setColour(colour.withAlpha(0.20f));
            g.fillPath(path);
        }
        auto handle = [&](float x, float hz, bool left)
        {
            g.setColour(colour);
            g.fillRect(x - 1, top, 2.0f, bottom - top);
            juce::Path triangle;
            triangle.addTriangle(x - 6, bottom, x + 6, bottom, x, bottom - 9);
            g.fillPath(triangle);
            auto text = hz < 1000 ? juce::String(hz, 0) + " Hz" : juce::String(hz / 1000, 2) + " kHz";
            g.drawText(text,
                       juce::Rectangle<float>(juce::jlimit(0.f, (float)getWidth() - 96, left ? x : x - 96), 1,
                                              96, 20),
                       left ? juce::Justification::centredLeft : juce::Justification::centredRight);
        };
        handle(l, owner.low->get(), true);
        handle(r, owner.high->get(), false);
        g.setColour(colour.withAlpha(0.6f));
        g.drawRoundedRectangle(bounds, 3, 1);
    }
    // mouseMove: kullanıcı/pencere olayını bu bileşenin parametre ve sahiplik durumuna taşır; kapanmış nesne ve boş seçim yollarını koruyun.
    void mouseMove(const juce::MouseEvent &e) override
    {
        setMouseCursor(hit(e.position.x) >= 0 ? juce::MouseCursor::LeftRightResizeCursor
                                              : juce::MouseCursor::PointingHandCursor);
    }
    // mouseDown: kullanıcı/pencere olayını bu bileşenin parametre ve sahiplik durumuna taşır; kapanmış nesne ve boş seçim yollarını koruyun.
    // Gövdedeki başlıca çağrılar: hit → parameter → beginChangeGesture → frequencyX. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    void mouseDown(const juce::MouseEvent &e) override
    {
        dragging = hit(e.position.x);
        if (dragging >= 0)
        {
            parameter()->beginChangeGesture();
            dragStart = e.position.x;
            return;
        }
        int b = e.position.x < frequencyX(owner.low->get())    ? 0
                : e.position.x > frequencyX(owner.high->get()) ? 2
                                                               : 1;
        owner.selectedBand = (b == 0 && !owner.hasLowBand()) || (b == 2 && !owner.hasHighBand()) ? 1 : b;
        if (onSelection)
            onSelection();
        repaint();
    }
    // mouseDrag: kullanıcı/pencere olayını bu bileşenin parametre ve sahiplik durumuna taşır; kapanmış nesne ve boş seçim yollarını koruyun.
    // Gövdedeki başlıca çağrılar: e.mods.isShiftDown → std::pow → juce::jlimit → getWidth. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    void mouseDrag(const juce::MouseEvent &e) override
    {
        if (dragging < 0)
            return;
        float x = e.mods.isShiftDown() ? dragStart + (e.position.x - dragStart) * 0.1f : e.position.x;
        const float hz = 20.0f * std::pow(1000.0f, juce::jlimit(0.f, 1.f, (x - 8) / (getWidth() - 16.0f)));
        const float value =
            dragging == 0 ? juce::jmin(hz, owner.high->get() - 1) : juce::jmax(hz, owner.low->get() + 1);
        if (!owner.enabled->get())
            owner.setMultiband(true);
        parameter()->setValueNotifyingHost(parameter()->convertTo0to1(value));
        if (onSelection)
            onSelection();
        repaint();
    }
    // mouseUp: kullanıcı/pencere olayını bu bileşenin parametre ve sahiplik durumuna taşır; kapanmış nesne ve boş seçim yollarını koruyun.
    void mouseUp(const juce::MouseEvent &) override
    {
        if (dragging >= 0)
            parameter()->endChangeGesture();
        dragging = -1;
    }
    // mouseDoubleClick: kullanıcı/pencere olayını bu bileşenin parametre ve sahiplik durumuna taşır; kapanmış nesne ve boş seçim yollarını koruyun.
    // Gövdedeki başlıca çağrılar: hit → parameter → endChangeGesture → p->beginChangeGesture. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    void mouseDoubleClick(const juce::MouseEvent &e) override
    {
        int which = hit(e.position.x);
        if (which < 0)
            return;
        if (dragging >= 0)
        {
            parameter()->endChangeGesture();
            dragging = -1;
        }
        auto *p = which == 0 ? owner.low : owner.high;
        p->beginChangeGesture();
        p->setValueNotifyingHost(p->convertTo0to1(which == 0 ? 20.0f : 20000.0f));
        p->endChangeGesture();
        owner.selectedBand = owner.editorBand();
        if (onSelection)
            onSelection();
        repaint();
    }

  private:
    MultibandProcessor &owner;
    juce::dsp::FFT fft{10};
    juce::dsp::WindowingFunction<float> window{MultibandProcessor::spectrumSize,
                                               juce::dsp::WindowingFunction<float>::hann};
    std::array<float, MultibandProcessor::spectrumSize * 2> fftData{};
    std::array<float, MultibandProcessor::spectrumSize / 2> magnitudes{};
    bool haveSpectrum = false;
    int dragging = -1;
    float dragStart = 0;
    // parameter alt adımı: Low/Mid/High bölme sınırlarının görsel hit-test ve sürükleme etkileşimini yönetir.
    juce::AudioParameterFloat *parameter()
    {
        return dragging == 0 ? owner.low : owner.high;
    }
    // hit: kullanıcı/pencere olayını bu bileşenin parametre ve sahiplik durumuna taşır; kapanmış nesne ve boş seçim yollarını koruyun.
    int hit(float x) const
    {
        auto left = std::abs(x - frequencyX(owner.low->get())),
             right = std::abs(x - frequencyX(owner.high->get()));
        return juce::jmin(left, right) > 12 ? -1 : left <= right ? 0 : 1;
    }
    // Mesaj iş parçacığındaki periyodik görünüm eşitlemesini yapar; audio callback değildir.
    // Gövdedeki başlıca çağrılar: owner.spectrumVisible.store → isShowing → owner.readSpectrum → std::fill. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    void timerCallback() override
    {
        owner.spectrumVisible.store(isShowing());
        std::array<float, MultibandProcessor::spectrumSize> frame;
        if (owner.readSpectrum(frame))
        {
            std::fill(fftData.begin(), fftData.end(), 0);
            std::copy(frame.begin(), frame.end(), fftData.begin());
            window.multiplyWithWindowingTable(fftData.data(), frame.size());
            fft.performFrequencyOnlyForwardTransform(fftData.data());
            for (size_t i = 0; i < magnitudes.size(); ++i)
                magnitudes[i] =
                    0.6f * magnitudes[i] + 0.4f * (fftData[i] * 4.0f / MultibandProcessor::spectrumSize);
            haveSpectrum = true;
        }
        repaint();
    }
};
