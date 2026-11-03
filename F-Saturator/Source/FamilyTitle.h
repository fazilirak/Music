// Fazıl Irak · 03.11.2026: F ailesi ana başlıkları, URW Gothic Book / 24 mantıksal ekran pikseli.
#pragma once
#include <juce_graphics/juce_graphics.h>
#include "FamilyTitleFonts.h"
inline juce::Font familyTitleFont(float canvasScale=1.0f) {
    static auto face=juce::Typeface::createSystemTypefaceFor(family_title::Book_otf,family_title::Book_otfSize);
    return juce::Font(juce::FontOptions(face).withHeight(24.0f/canvasScale));
}

inline juce::Font familySubtitleFont(float canvasScale=1.0f) {
    return familyTitleFont(canvasScale).withHeight(9.0f/canvasScale);
}
