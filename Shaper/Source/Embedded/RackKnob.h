// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 Fazıl Irak; Shaper eklemeleri/değişiklikleri.
// Fazıl Irak tarafından oluşturuldu / değiştirildi: 23.10.2026
// Kapsam: Shaper için eklenen kod ve uyarlamalar; özgün üçüncü taraf telif/lisansları korunur.
// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: Shaper standart küçük rotary çizimi ve etiket/drag alanlarını ortak tutar; görünüm standardı AGENTS.md içindedir.
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

// Shaper's shared GATE-12 rotary design, used by all eight effects.
// Geometry and original drawing adapted from Tilr's GPL-3.0 Rotary.cpp.
namespace shaperui
{
inline constexpr int knobWidth = 68, knobHeight = 64;
// drawKnob alt adımı: Shaper standart küçük rotary çizimi ve etiket/drag alanlarını ortak tutar; görünüm standardı AGENTS.md içindedir.
// Gövdedeki başlıca çağrılar: bounds.getWidth → bounds.getHeight → disk → g.setColour. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
inline void drawKnob(juce::Graphics &g, juce::Rectangle<int> bounds, float value, juce::Colour accent,
                     bool symmetric = false, juce::Colour body = juce::Colour(0xff272727))
{
    constexpr float radius = 16, limit = 130 * juce::MathConstants<float>::pi / 180;
    const float cx = bounds.getWidth() * .5f, cy = bounds.getHeight() * .5f - 8,
                angle = -limit + value * 2 * limit;
    const juce::Rectangle<float> disk(cx - radius, cy - radius, 2 * radius, 2 * radius);
    g.setColour(body);
    g.fillEllipse(disk);
    g.setColour(juce::Colour(0x33000000));
    g.fillEllipse(disk.translated(0, 2).reduced(1));
    juce::Path track;
    track.addCentredArc(cx, cy, radius + 5, radius + 5, 0, -limit, limit, true);
    g.setColour(body);
    const juce::PathStrokeType stroke(3, juce::PathStrokeType::curved, juce::PathStrokeType::rounded);
    g.strokePath(track, stroke);
    if ((symmetric && value != .5f) || (!symmetric && value != 0))
    {
        juce::Path arc;
        arc.addCentredArc(cx, cy, radius + 5, radius + 5, 0, symmetric ? 0 : -limit, angle, true);
        g.setColour(accent);
        g.strokePath(arc, stroke);
    }
    juce::Path pointer;
    pointer.addLineSegment(juce::Line<float>(0, -5, 0, -radius + 5), .1f);
    stroke.createStrokedPath(pointer, pointer);
    g.setColour(juce::Colours::white);
    g.fillPath(pointer, juce::AffineTransform::rotation(angle).translated(cx, cy));
}
class RackKnob final : public juce::Component, public juce::SettableTooltipClient
{
    juce::RangedAudioParameter &parameter;
    juce::String caption;
    juce::Colour accent;
    bool symmetric = false, dragging = false;
    float plain = 0, dragValue = 0;
    juce::Point<int> last;
    juce::ParameterAttachment attachment;

  public:
    std::function<juce::String(float)> format;
    // RackKnob alt adımı: Shaper standart küçük rotary çizimi ve etiket/drag alanlarını ortak tutar; görünüm standardı AGENTS.md içindedir.
    RackKnob(juce::AudioProcessorValueTreeState &state, const char *id, juce::String label,
             juce::Colour colour, bool centred = false)
        : parameter(*state.getParameter(id)), caption(label), accent(colour), symmetric(centred),
          attachment(parameter,
                     [this](float v)
                     {
                         plain = v;
                         repaint();
                     })
    {
        setName(label);
        setSize(knobWidth, knobHeight);
        attachment.sendInitialUpdate();
        setTooltip(label + ": drag or scroll to adjust. Shift: fine. Double-click: reset.");
    }
    // ~RackKnob alt adımı: Shaper standart küçük rotary çizimi ve etiket/drag alanlarını ortak tutar; görünüm standardı AGENTS.md içindedir.
    ~RackKnob() override
    {
        if (dragging)
            attachment.endGesture();
    }
    // Mevcut kontrol/durumun görselini çizer; ses callback’inde çağrılmamalıdır.
    void paint(juce::Graphics &g) override
    {
        drawKnob(g, getLocalBounds(), parameter.convertTo0to1(plain), accent, symmetric);
        g.setColour(juce::Colours::white);
        g.setFont(juce::FontOptions(16));
        g.drawText(dragging ? (format ? format(plain) : parameter.getText(parameter.convertTo0to1(plain), 20))
                            : caption,
                   0, getHeight() - 16, getWidth(), 16, juce::Justification::centred, true);
    }
    // mouseDown: kullanıcı/pencere olayını bu bileşenin parametre ve sahiplik durumuna taşır; kapanmış nesne ve boş seçim yollarını koruyun.
    void mouseDown(const juce::MouseEvent &e) override
    {
        dragging = true;
        dragValue = parameter.getValue();
        last = e.getPosition();
        attachment.beginGesture();
        repaint();
    }
    // mouseDrag: kullanıcı/pencere olayını bu bileşenin parametre ve sahiplik durumuna taşır; kapanmış nesne ve boş seçim yollarını koruyun.
    void mouseDrag(const juce::MouseEvent &e) override
    {
        if (!dragging)
            return;
        const auto delta = e.getPosition() - last;
        last = e.getPosition();
        dragValue = juce::jlimit(0.0f, 1.0f,
                                 dragValue + (delta.x - delta.y) / (e.mods.isShiftDown() ? 4000.0f : 400.0f));
        attachment.setValueAsPartOfGesture(parameter.convertFrom0to1(dragValue));
    }
    // mouseUp: kullanıcı/pencere olayını bu bileşenin parametre ve sahiplik durumuna taşır; kapanmış nesne ve boş seçim yollarını koruyun.
    void mouseUp(const juce::MouseEvent &) override
    {
        if (dragging)
            attachment.endGesture();
        dragging = false;
        repaint();
    }
    // mouseWheelMove: kullanıcı/pencere olayını bu bileşenin parametre ve sahiplik durumuna taşır; kapanmış nesne ve boş seçim yollarını koruyun.
    void mouseWheelMove(const juce::MouseEvent &e, const juce::MouseWheelDetails &wheel) override
    {
        if (dragging || wheel.deltaY == 0)
            return;
        const float change = (wheel.deltaY > 0 ? 1.0f : -1.0f) * (e.mods.isShiftDown() ? .01f : .05f);
        attachment.setValueAsCompleteGesture(
            parameter.convertFrom0to1(juce::jlimit(0.0f, 1.0f, parameter.getValue() + change)));
    }
    // mouseDoubleClick: kullanıcı/pencere olayını bu bileşenin parametre ve sahiplik durumuna taşır; kapanmış nesne ve boş seçim yollarını koruyun.
    void mouseDoubleClick(const juce::MouseEvent &) override
    {
        attachment.setValueAsCompleteGesture(parameter.convertFrom0to1(parameter.getDefaultValue()));
    }
};
} // namespace shaperui
