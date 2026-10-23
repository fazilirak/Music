// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 Fazıl Irak; Shaper eklemeleri/değişiklikleri.
// Fazıl Irak tarafından oluşturuldu / değiştirildi: 23.10.2026
// Kapsam: Shaper için eklenen kod ve uyarlamalar; özgün üçüncü taraf telif/lisansları korunur.
// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: Rack düğmelerinin görsel/etkileşim durumunu ve ortak ikon çizimini tanımlar.
#pragma once
#include <JuceHeader.h>

// Text remains available to accessibility clients; the visible control is vector art.
class CompactIconButton final : public juce::TextButton
{
  public:
    // CompactIconButton alt adımı: Rack düğmelerinin görsel/etkileşim durumunu ve ortak ikon çizimini tanımlar.
    explicit CompactIconButton(const juce::String &name = {}) : juce::TextButton(name)
    {
        setTooltip(name);
    }
    // paintButton alt adımı: Rack düğmelerinin görsel/etkileşim durumunu ve ortak ikon çizimini tanımlar.
    // Gövdedeki başlıca çağrılar: getLocalBounds → toFloat → reduced → g.setColour. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    void paintButton(juce::Graphics &g, bool over, bool down) override
    {
        auto r = getLocalBounds().toFloat().reduced(0.5f);
        g.setColour(juce::Colour(down ? 0xff384351 : over ? 0xff303945 : 0xff1c242e));
        g.fillRoundedRectangle(r, 4);
        g.setColour(juce::Colours::white.withAlpha(isEnabled() ? over ? 0.45f : 0.15f : 0.06f));
        g.drawRoundedRectangle(r, 4, 1);
        g.setColour(
            findColour(juce::TextButton::textColourOffId).withMultipliedAlpha(isEnabled() ? 1.0f : 0.25f));
        juce::Graphics::ScopedSaveState state(g);
        float scale = juce::jmin(getWidth(), getHeight()) / 34.0f;
        g.addTransform(juce::AffineTransform::scale(scale).translated((getWidth() - 24 * scale) / 2,
                                                                      (getHeight() - 24 * scale) / 2));
        juce::Path path;
        auto name = getButtonText();
        auto line = [&](float x, float y, float x2, float y2)
        {
            path.startNewSubPath(x, y);
            path.lineTo(x2, y2);
        };
        auto arrow = [&](float x, float y, bool right)
        {
            line(x + (right ? -4 : 4), y - 4, x, y);
            path.lineTo(x + (right ? -4 : 4), y + 4);
        };
        if (name == "<" || name == ">")
            arrow(name == "<" ? 9 : 15, 12, name == ">");
        else if (name == "Copy")
        {
            path.addRoundedRectangle(7, 7, 13, 14, 1);
            line(4, 16, 4, 3);
            path.lineTo(16, 3);
        }
        else if (name == "Paste")
        {
            path.addRoundedRectangle(5, 5, 14, 16, 1);
            path.addRoundedRectangle(8, 2, 8, 5, 1);
            line(8, 12, 16, 12);
            line(8, 16, 16, 16);
        }
        else if (name == "Invert")
        {
            line(12, 2, 12, 22);
            path.addTriangle(3, 16, 9, 6, 9, 16);
            path.addTriangle(15, 8, 15, 18, 21, 8);
        }
        else if (name == "Reverse")
        {
            line(3, 7, 20, 7);
            arrow(20, 7, true);
            line(21, 17, 4, 17);
            arrow(4, 17, false);
        }
        else if (name == "Double")
        {
            path.addRoundedRectangle(2, 4, 8, 16, 1);
            path.addRoundedRectangle(14, 4, 8, 16, 1);
            line(3, 15, 6, 8);
            path.lineTo(9, 15);
            line(15, 15, 18, 8);
            path.lineTo(21, 15);
        }
        else if (name == "Clear")
        {
            line(4, 6, 20, 6);
            path.addRectangle(9, 3, 6, 3);
            line(6, 6, 7, 21);
            path.lineTo(17, 21);
            path.lineTo(18, 6);
            line(10, 9, 10, 18);
            line(14, 9, 14, 18);
        }
        else if (name == "Reset")
        {
            path.addCentredArc(12, 13, 8, 8, 0, -1.5f, 3.9f, true);
            arrow(5, 7, false);
        }
        else if (name.containsIgnoreCase("save"))
        {
            path.addRoundedRectangle(3, 3, 18, 18, 1);
            path.addRectangle(7, 3, 10, 6);
            path.addRectangle(7, 14, 10, 7);
            line(14, 4, 14, 8);
        }
        else if (name.containsIgnoreCase("load"))
        {
            path.startNewSubPath(3, 8);
            path.lineTo(3, 4);
            path.lineTo(10, 4);
            path.lineTo(13, 7);
            path.lineTo(21, 7);
            path.lineTo(21, 19);
            path.lineTo(3, 19);
            path.closeSubPath();
            line(3, 10, 21, 10);
        }
        else if (name == "Options")
        {
            path.addEllipse(7, 7, 10, 10);
            path.addEllipse(10, 10, 4, 4);
            for (int i = 0; i < 8; ++i)
            {
                float a = i * juce::MathConstants<float>::twoPi / 8;
                line(12 + 7 * std::sin(a), 12 + 7 * std::cos(a), 12 + 10 * std::sin(a),
                     12 + 10 * std::cos(a));
            }
        }
        else if (name == "About")
        {
            path.addEllipse(3, 3, 18, 18);
            line(12, 10, 12, 17);
            path.addEllipse(11.3f, 6.5f, 1.4f, 1.4f);
        }
        g.strokePath(path,
                     juce::PathStrokeType(1.7f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
};
