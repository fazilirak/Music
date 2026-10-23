// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 Fazıl Irak; Shaper eklemeleri/değişiklikleri.
// Fazıl Irak tarafından oluşturuldu / değiştirildi: 23.10.2026
// Kapsam: Shaper için eklenen kod ve uyarlamalar; özgün üçüncü taraf telif/lisansları korunur.
// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: Time modülünün kısa hazır ayarlarını ve değer eşlemelerini tutar.
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>
#include <cmath>

// Original time-offset curves. Instantaneous playback speed = 1 - d(delay)/dx.
struct TimeQuickPresets
{
    static constexpr int count = 8, programOffset = 79;
    struct Point
    {
        double x, y, tension;
        int type;
    };
    // name alt adımı: Time modülünün kısa hazır ayarlarını ve değer eşlemelerini tutar.
    static const char *name(int i)
    {
        static const char *names[]{"Stutter 1/8",    "Stutter 1/16",   "Scratch Sweep", "Scratch Double",
                                   "Tape Stop Slow", "Tape Stop Fast", "Pitch -12 st",  "Pitch +12 st"};
        return names[juce::jlimit(0, count - 1, i)];
    }
    // delay alt adımı: Time modülünün kısa hazır ayarlarını ve değer eşlemelerini tutar.
    // Gövdedeki başlıca çağrılar: std::floor → juce::jmin → std::sin → std::pow. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    static double delay(int i, double x)
    {
        switch (i)
        {
        case 0:
            return std::floor(juce::jmin(x, 0.999999) * 8) / 8;
        case 1:
            return std::floor(juce::jmin(x, 0.999999) * 16) / 16;
        case 2:
            return 0.24 + 0.19 * std::sin(2 * juce::MathConstants<double>::pi * x);
        case 3:
            return 0.30 + 0.12 * std::sin(4 * juce::MathConstants<double>::pi * x) +
                   0.045 * std::sin(8 * juce::MathConstants<double>::pi * x);
        case 4:
            return 0.5 * x * x;
        case 5:
            return x < 0.5 ? 0 : x < 0.8 ? std::pow(x - 0.5, 2) / 0.6 : 0.15 + x - 0.8;
        case 6:
            return 0.5 * x;
        default:
            return 1 - x;
        }
    }
    // points alt adımı: Time modülünün kısa hazır ayarlarını ve değer eşlemelerini tutar.
    // Gövdedeki başlıca çağrılar: result.push_back → delay. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    static std::vector<Point> points(int i)
    {
        std::vector<Point> result;
        if (i < 2)
        {
            const int n = i == 0 ? 8 : 16;
            for (int k = 0; k < n; ++k)
                result.push_back({(double)k / n, (double)k / n, 0, 0});
            result.push_back({1.0, (double)(n - 1) / n, 0, 0});
        }
        else if (i >= 6)
        {
            result.push_back({0, delay(i, 0), 0, 1});
            result.push_back({1, delay(i, 1), 0, 1});
        }
        else
        {
            for (int k = 0; k <= 128; ++k)
            {
                double x = k / 128.0;
                result.push_back({x, delay(i, x), 0, 1});
            }
        }
        return result;
    }
};
class TimeQuickPresetButton final : public juce::Button
{
    int id;
    bool selected = false;

  public:
    // TimeQuickPresetButton alt adımı: Time modülünün kısa hazır ayarlarını ve değer eşlemelerini tutar.
    explicit TimeQuickPresetButton(int i) : juce::Button(TimeQuickPresets::name(i)), id(i)
    {
        setComponentID("time-quick-preset-" + juce::String(i));
        setTooltip(juce::String(i + 1) + ". " + TimeQuickPresets::name(i));
    }
    // setSelected: Time modülünün kısa hazır ayarlarını ve değer eşlemelerini tutar. Burada verilen ayar türetilmiş duruma/çıktıya aktarılır; çağrı sırası aşağıdaki gövdededir.
    void setSelected(bool value)
    {
        selected = value;
        repaint();
    }
    // paintButton alt adımı: Time modülünün kısa hazır ayarlarını ve değer eşlemelerini tutar.
    // Gövdedeki başlıca çağrılar: getLocalBounds → toFloat → reduced → juce::Colour. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    void paintButton(juce::Graphics &g, bool over, bool down) override
    {
        auto r = getLocalBounds().toFloat().reduced(0.5f);
        auto green = juce::Colour(0xff00e672);
        g.setColour(green.withAlpha(selected ? 0.22f : over || down ? 0.12f : 0.04f));
        g.fillRoundedRectangle(r, 2);
        g.setColour(green.withAlpha(selected ? 1.0f : 0.45f));
        g.drawRoundedRectangle(r, 2, 1);
        auto plot = r.reduced(3);
        juce::Path path;
        for (int n = 0; n <= 128; ++n)
        {
            float x = plot.getX() + plot.getWidth() * n / 128,
                  y = plot.getY() + plot.getHeight() * (float)TimeQuickPresets::delay(id, n / 128.0);
            if (n == 0)
                path.startNewSubPath(x, y);
            else
                path.lineTo(x, y);
        }
        g.setColour(green);
        g.strokePath(path, juce::PathStrokeType(1.15f));
    }
};
