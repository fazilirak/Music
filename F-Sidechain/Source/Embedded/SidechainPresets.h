// Geliştirici: Fazıl Irak - Son düzenleme: 27.10.2026
// Dosyanın görevi: Envelope düzenleme ve ürün kontrol bileşeni; özgün kaynak atıfları korunur.
// Katkı kapsamı: F-Sidechain bakımı ve uyarlamaları. GATE-12/Shaper özgün telifleri korunur.
// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 Fazıl Irak; Shaper eklemeleri/değişiklikleri.
// Fazıl Irak tarafından oluşturuldu / değiştirildi: 23.10.2026
// Kapsam: Shaper için eklenen kod ve uyarlamalar; özgün üçüncü taraf telif/lisansları korunur.
// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: Volume/sidechain yönlü hazır envelope/desen verilerinin sözleşmesidir.
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "VolumePresetData.h"
#include <array>
#include <vector>
#include <sstream>
#include <functional>
#include <cmath>

struct SidechainPresets
{
    static constexpr int count = 8;
    struct Point
    {
        double x, y, tension;
        int type;
    };
    // name alt adımı: Volume/sidechain yönlü hazır envelope/desen verilerinin sözleşmesidir.
    static const char *name(int i)
    {
        static const char *names[]{"Volume 1", "Volume 2", "Volume 3", "Volume 4",
                                   "Volume 5", "Volume 6", "Volume 7", "Volume 8"};
        return names[juce::jlimit(0, count - 1, i)];
    }
    // state alt adımı: Volume/sidechain yönlü hazır envelope/desen verilerinin sözleşmesidir.
    static const char *state(int i)
    {
        return volumePresetStates[juce::jlimit(0, count - 1, i)];
    }
    // points alt adımı: Volume/sidechain yönlü hazır envelope/desen verilerinin sözleşmesidir.
    // Gövdedeki başlıca çağrılar: juce::parseXML → state → xml->getIntAttribute → stream. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    static std::vector<Point> points(int i)
    {
        std::vector<Point> result;
        if (auto xml = juce::parseXML(state(i)))
        {
            int pattern = xml->getIntAttribute("currpattern", 1) - 1;
            std::istringstream stream(
                xml->getStringAttribute("pattern" + juce::String(pattern)).toStdString());
            Point p;
            while (stream >> p.x >> p.y >> p.tension >> p.type)
                result.push_back(p);
        }
        return result;
    }
};

class SidechainPresetButton final : public juce::Button
{
  public:
    // SidechainPresetButton alt adımı: Volume/sidechain yönlü hazır envelope/desen verilerinin sözleşmesidir.
    explicit SidechainPresetButton(int index) : Button(SidechainPresets::name(index)), id(index)
    {
        setTooltip(juce::String(index + 1) + ". " + SidechainPresets::name(index));
        setComponentID("sidechain-preset-" + juce::String(index));
    }
    // setPreview: Volume/sidechain yönlü hazır envelope/desen verilerinin sözleşmesidir. Burada verilen ayar türetilmiş duruma/çıktıya aktarılır; çağrı sırası aşağıdaki gövdededir.
    void setPreview(const std::function<double(double)> &sample)
    {
        for (int n = 0; n <= 128; ++n)
            preview[(size_t)n] = (float)sample(n == 128 ? 0.999999 : n / 128.0);
    }
    // setSelected: Volume/sidechain yönlü hazır envelope/desen verilerinin sözleşmesidir. Burada verilen ayar türetilmiş duruma/çıktıya aktarılır; çağrı sırası aşağıdaki gövdededir.
    void setSelected(bool value)
    {
        selected = value;
        repaint();
    }
    // paintButton alt adımı: Volume/sidechain yönlü hazır envelope/desen verilerinin sözleşmesidir.
    // Gövdedeki başlıca çağrılar: getLocalBounds → toFloat → reduced → juce::Colour. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    void paintButton(juce::Graphics &g, bool over, bool down) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(0.5f);
        auto yellow = juce::Colour(0xffffd42a);
        g.setColour(selected ? yellow.withAlpha(0.24f)
                             : juce::Colour(over || down ? 0xff38372b : 0xff22231d));
        g.fillRoundedRectangle(bounds, 2);
        g.setColour(selected ? yellow : yellow.withAlpha(0.45f));
        g.drawRoundedRectangle(bounds, 2, 1);
        auto r = bounds.reduced(3, 3);
        juce::Path line;
        for (int n = 0; n <= 96; ++n)
        {
            float x = r.getX() + r.getWidth() * n / 96.0f,
                  y = r.getBottom() -
                      r.getHeight() *
                          preview[(size_t)juce::jlimit(0, 128, (int)std::round(n * 128.0 / 96.0))];
            if (n == 0)
                line.startNewSubPath(x, y);
            else
                line.lineTo(x, y);
        }
        g.setColour(yellow);
        g.strokePath(line, juce::PathStrokeType(1.15f));
    }

  private:
    std::array<float, 129> preview{};
    int id;
    bool selected = false;
};
