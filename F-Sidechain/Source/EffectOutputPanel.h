// Geliştirici: Fazıl Irak - Son düzenleme: 27.10.2026
// Dosyanın görevi: Envelope düzenleme ve ürün kontrol bileşeni; özgün kaynak atıfları korunur.
// Katkı kapsamı: F-Sidechain bakımı ve uyarlamaları. GATE-12/Shaper özgün telifleri korunur.
// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 Fazıl Irak; Shaper eklemeleri/değişiklikleri.
// Fazıl Irak tarafından oluşturuldu / değiştirildi: 23.10.2026
// Kapsam: Shaper için eklenen kod ve uyarlamalar; özgün üçüncü taraf telif/lisansları korunur.
// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: Efektin ortak Stereo/çıkış meter/dB/Mix panelini bağlı parametrelerle eşitler.
#pragma once
#include "MultibandProcessor.h"
#include "EffectPalette.h"

// Shared, unscaled output controls. All values address the currently edited band.
class EffectOutputPanel final : public juce::Component, private juce::Timer
{
    class Dial final : public juce::Component, public juce::SettableTooltipClient
    {
        EffectOutputPanel &panel;
        juce::String kind;
        // parameter alt adımı: Efektin ortak Stereo/çıkış meter/dB/Mix panelini bağlı parametrelerle eşitler.
        juce::RangedAudioParameter *parameter() const
        {
            return panel.owner.outputParameter(panel.owner.editorBand(), kind);
        }
        float start = 0;
        bool dragging = false;
        // change: Efektin ortak Stereo/çıkış meter/dB/Mix panelini bağlı parametrelerle eşitler. Burada verilen ayar türetilmiş duruma/çıktıya aktarılır; çağrı sırası aşağıdaki gövdededir.
        void change(float value)
        {
            if (auto *p = parameter())
                p->setValueNotifyingHost(juce::jlimit(0.0f, 1.0f, value));
            repaint();
        }

      public:
        // Dial alt adımı: Efektin ortak Stereo/çıkış meter/dB/Mix panelini bağlı parametrelerle eşitler.
        Dial(EffectOutputPanel &p, const char *k) : panel(p), kind(k)
        {
            setComponentID("effect-output-" + kind);
            setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
        }
        // ~Dial alt adımı: Efektin ortak Stereo/çıkış meter/dB/Mix panelini bağlı parametrelerle eşitler.
        ~Dial() override
        {
            if (dragging)
                if (auto *p = parameter())
                    p->endChangeGesture();
        }
        // Mevcut kontrol/durumun görselini çizer; ses callback’inde çağrılmamalıdır.
        // Gövdedeki başlıca çağrılar: parameter → p->convertFrom0to1 → p->getValue → rackEffectColour. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
        void paint(juce::Graphics &g) override
        {
            auto *p = parameter();
            if (!p)
                return;
            const float value = p->convertFrom0to1(p->getValue());
            auto accent = rackEffectColour(panel.owner.effect);
            if (kind == "mix")
            {
                g.setColour(juce::Colour(0xff99a2af));
                g.setFont(juce::FontOptions(12));
                g.drawText("Mix", 0, 0, getWidth(), 20, juce::Justification::centred);
                g.setColour(accent);
                g.setFont(juce::FontOptions(20, juce::Font::bold));
                g.drawText(juce::String(juce::roundToInt(value * 100)) + "%", 0, 22, getWidth(), 28,
                           juce::Justification::centred);
            }
            else if (kind == "gain")
            {
                const float db = p->paramID == "gain" ? juce::Decibels::gainToDecibels(value, -60.0f) : value;
                g.setColour(juce::Colour(0xff394446));
                g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(.5f), 3, 1);
                const int b = panel.owner.editorBand();
                const float l = panel.owner.outputPeakLeft[(size_t)b].load(std::memory_order_relaxed),
                            r = panel.owner.outputPeakRight[(size_t)b].load(std::memory_order_relaxed);
                auto level = [](float v)
                { return juce::jlimit(0.0f, 1.0f, (juce::Decibels::gainToDecibels(v, -60.0f) + 60) / 60); };
                g.setColour(accent.withAlpha(.32f));
                g.fillRect(3.0f, 3.0f, (getWidth() - 6) * level(l), 10.0f);
                g.fillRect(3.0f, 15.0f, (getWidth() - 6) * level(r), 10.0f);
                g.setColour(juce::Colours::white);
                g.setFont(juce::FontOptions(13));
                g.drawText((db > 0 ? "+" : "") + juce::String(db, 1) + " dB", getLocalBounds(),
                           juce::Justification::centred);
            }
            else
            {
                g.setColour(accent.withAlpha(.65f));
                g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(.5f), 3, 1);
                juce::String text = "Stereo";
                if (isMouseOverOrDragging())
                    text = p->paramID == "stereo"
                               ? juce::String(juce::roundToInt(value)) + juce::String::fromUTF8("°")
                               : juce::String(juce::roundToInt(value * 100)) + "%";
                g.setColour(accent);
                g.setFont(juce::FontOptions(13));
                g.drawText(text, getLocalBounds(), juce::Justification::centred);
            }
        }
        // mouseDown: kullanıcı/pencere olayını bu bileşenin parametre ve sahiplik durumuna taşır; kapanmış nesne ve boş seçim yollarını koruyun.
        void mouseDown(const juce::MouseEvent &) override
        {
            if (auto *p = parameter())
            {
                start = p->getValue();
                p->beginChangeGesture();
                dragging = true;
            }
        }
        // mouseDrag: kullanıcı/pencere olayını bu bileşenin parametre ve sahiplik durumuna taşır; kapanmış nesne ve boş seçim yollarını koruyun.
        void mouseDrag(const juce::MouseEvent &e) override
        {
            change(start + (e.getDistanceFromDragStartX() - e.getDistanceFromDragStartY()) /
                               (e.mods.isShiftDown() ? 4000.0f : 400.0f));
        }
        // mouseUp: kullanıcı/pencere olayını bu bileşenin parametre ve sahiplik durumuna taşır; kapanmış nesne ve boş seçim yollarını koruyun.
        void mouseUp(const juce::MouseEvent &) override
        {
            if (dragging)
            {
                if (auto *p = parameter())
                    p->endChangeGesture();
                dragging = false;
            }
        }
        // mouseDoubleClick: kullanıcı/pencere olayını bu bileşenin parametre ve sahiplik durumuna taşır; kapanmış nesne ve boş seçim yollarını koruyun.
        void mouseDoubleClick(const juce::MouseEvent &) override
        {
            if (auto *p = parameter())
            {
                p->beginChangeGesture();
                change(p->getDefaultValue());
                p->endChangeGesture();
            }
        }
        // mouseWheelMove: kullanıcı/pencere olayını bu bileşenin parametre ve sahiplik durumuna taşır; kapanmış nesne ve boş seçim yollarını koruyun.
        void mouseWheelMove(const juce::MouseEvent &e, const juce::MouseWheelDetails &w) override
        {
            if (auto *p = parameter())
            {
                p->beginChangeGesture();
                change(p->getValue() + w.deltaY * (e.mods.isShiftDown() ? .02f : .2f));
                p->endChangeGesture();
            }
        }
        // mouseEnter: kullanıcı/pencere olayını bu bileşenin parametre ve sahiplik durumuna taşır; kapanmış nesne ve boş seçim yollarını koruyun.
        void mouseEnter(const juce::MouseEvent &) override
        {
            repaint();
        }
        // mouseExit: kullanıcı/pencere olayını bu bileşenin parametre ve sahiplik durumuna taşır; kapanmış nesne ve boş seçim yollarını koruyun.
        void mouseExit(const juce::MouseEvent &) override
        {
            repaint();
        }
    };
    MultibandProcessor &owner;
    Dial mix{*this, "mix"}, gain{*this, "gain"}, stereo{*this, "stereo"};
    // Mesaj iş parçacığındaki periyodik görünüm eşitlemesini yapar; audio callback değildir.
    void timerCallback() override
    {
        repaint();
    }

  public:
    // EffectOutputPanel alt adımı: Efektin ortak Stereo/çıkış meter/dB/Mix panelini bağlı parametrelerle eşitler.
    // Gövdedeki başlıca çağrılar: setComponentID → addAndMakeVisible → mix.setTooltip → gain.setTooltip. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    explicit EffectOutputPanel(MultibandProcessor &p) : owner(p)
    {
        setComponentID("effect-output-panel");
        for (auto *c : {&mix, &gain, &stereo})
            addAndMakeVisible(c);
        mix.setTooltip("Selected band wet/dry mix. Drag or scroll; double-click to reset.");
        gain.setTooltip(
            "Selected band output gain in dB / stereo level meter. Drag or scroll; double-click for unity gain.");
        stereo.setTooltip(owner.effect == 1 || owner.effect == 3 || owner.effect >= 6
                              ? "Stereo width of the selected band. 100% = original stereo, 0% = mono."
                              : "Left/right modulation phase offset. 0 degrees = linked stereo.");
        startTimerHz(30);
    }
    // Mevcut kontrol/durumun görselini çizer; ses callback’inde çağrılmamalıdır.
    void paint(juce::Graphics &g) override
    {
        g.setColour(juce::Colour(0xff1c2027));
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 6);
    }
    // Pencere/kontrol boyutuna bağlı yerleşimi yeniden hesaplar; hit-test aynı geometriyi izlemelidir.
    void resized() override
    {
        gain.setBounds(10, 8, 92, 28);
        stereo.setBounds(10, 44, 92, 26);
        mix.setBounds(110, 7, 62, 58);
    }
};
