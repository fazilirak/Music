// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 Fazıl Irak; Shaper eklemeleri/değişiklikleri.
// Fazıl Irak tarafından oluşturuldu / değiştirildi: 23.10.2026
// Kapsam: Shaper için eklenen kod ve uyarlamalar; özgün üçüncü taraf telif/lisansları korunur.
// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: Geliştirici, üçüncü taraf kaynakları ve lisans/bilgilendirme görünümünü kurar.
#pragma once
#include "Processor.h"
#include "EffectPalette.h"
class AboutPanel final : public juce::Component
{
  public:
    AboutPanel();
    std::function<void()> onClose;
    juce::TabbedComponent tabs{juce::TabbedButtonBar::TabsAtTop};
    void resized() override;
    void paint(juce::Graphics &) override;

  private:
    juce::TextButton close{"Close"};
};
