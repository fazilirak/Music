// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 Fazıl Irak; Shaper eklemeleri/değişiklikleri.
// Fazıl Irak tarafından oluşturuldu / değiştirildi: 23.10.2026
// Ortak Hakkında görünümü; eğri editörü kullanım ipuçları GATE-12 / Tilr'den uyarlanmıştır.
#pragma once
#include "JuceHeader.h"

namespace shaperui
{
class RackAboutPage final : public juce::Component
{
  public:
    RackAboutPage(juce::String title, juce::String description, juce::Colour colour)
        : title(std::move(title)), description(std::move(description)), colour(colour) {}

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour(0xdd000000));
        auto bounds = getLocalBounds().reduced(50);
        g.setColour(juce::Colours::white);
        g.setFont(juce::FontOptions(30.0f));
        g.drawText(title, bounds.removeFromTop(35), juce::Justification::centred);
        g.setFont(juce::FontOptions(20.0f));
        g.drawText("v" + juce::String(SHAPER_VERSION_STRING), bounds.removeFromTop(25), juce::Justification::centred);
        g.drawText(juce::String::fromUTF8("Fazıl Irak | Based on GATE-12 by Tilr"),
                   bounds.removeFromTop(25), juce::Justification::centred);
        g.setColour(colour);
        g.drawText(description, bounds.removeFromTop(25), juce::Justification::centred);
        g.setColour(juce::Colours::white);
        bounds.setWidth(640);
        bounds.setX((getWidth() - bounds.getWidth()) / 2);
        bounds.removeFromTop(40);
        const char* tips[]{
            "- Shift for fine slider adjustments.",
            "- Shift toggles snap on/off.",
            "- Mouse wheel on view changes grid size.",
            "- Right click points changes point type.",
            "- Alt + click inserts new point.",
            "- Alt + drag selection handles skews selected points.",
            "- Right click + drag in paint mode changes paint tool tension.",
            "- Shift + mouse wheel in sequencer changes step size."};
        for (const auto* tip : tips)
            g.drawText(tip, bounds.removeFromTop(25), juce::Justification::centredLeft);
    }

  private:
    juce::String title, description;
    juce::Colour colour;
};
} // namespace shaperui
