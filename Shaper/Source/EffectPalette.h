// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 Fazıl Irak; Shaper eklemeleri/değişiklikleri.
// Fazıl Irak tarafından oluşturuldu / değiştirildi: 23.10.2026
// Kapsam: Shaper için eklenen kod ve uyarlamalar; özgün üçüncü taraf telif/lisansları korunur.
// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: Efekt kimliği, adı, rengi ve üretici fonksiyonunu tek kayıt sözleşmesinde tutar.
#pragma once
#include "Processor.h"
// rackEffectColour alt adımı: Efekt kimliği, adı, rengi ve üretici fonksiyonunu tek kayıt sözleşmesinde tutar.
inline juce::Colour rackEffectColour(int effect)
{
    static constexpr juce::uint32 colours[]{0xffff8050, 0xff00e672, 0xff00bbff, 0xff9ca7ff,
                                            0xffff505b, 0xffffd42a, 0xffed6c46, 0xff54dfdb};
    return juce::Colour(colours[juce::jlimit(0, 7, effect)]);
}
class EffectPalette final : public juce::Component,
                            public juce::DragAndDropContainer,
                            public juce::DragAndDropTarget
{
  public:
    explicit EffectPalette(ShaperProcessor &);
    std::function<void()> onChange;
    void refresh();
    void resized() override;
    void paint(juce::Graphics &) override;
    bool isInterestedInDragSource(const SourceDetails &) override;
    void itemDragMove(const SourceDetails &) override;
    void itemDragExit(const SourceDetails &) override;
    void itemDropped(const SourceDetails &) override;

  private:
    class Card final : public juce::Component
    {
      public:
        // Card alt adımı: Efekt kimliği, adı, rengi ve üretici fonksiyonunu tek kayıt sözleşmesinde tutar.
        Card(EffectPalette &p, int effect) : owner(p), id(effect)
        {
            setMouseCursor(juce::MouseCursor::DraggingHandCursor);
            setComponentID("effect-card-" + juce::String(id));
        }
        void paint(juce::Graphics &) override;
        void mouseDown(const juce::MouseEvent &) override;
        void mouseDrag(const juce::MouseEvent &) override;
        EffectPalette &owner;
        int id;
    };
    ShaperProcessor &processor;
    std::array<std::unique_ptr<Card>, ShaperProcessor::effectCount> cards;
    juce::TextButton add{"+"};
    std::vector<int> installed;
    int dropIndex = -1;
    int insertionAt(int x, int movingEffect) const;
    void changed();
};
