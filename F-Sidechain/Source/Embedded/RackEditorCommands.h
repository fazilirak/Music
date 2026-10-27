// Geliştirici: Fazıl Irak - Son düzenleme: 27.10.2026
// Dosyanın görevi: Envelope düzenleme ve ürün kontrol bileşeni; özgün kaynak atıfları korunur.
// Katkı kapsamı: F-Sidechain bakımı ve uyarlamaları. GATE-12/Shaper özgün telifleri korunur.
// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 Fazıl Irak; Shaper eklemeleri/değişiklikleri.
// Fazıl Irak tarafından oluşturuldu / değiştirildi: 23.10.2026
// Kapsam: Shaper için eklenen kod ve uyarlamalar; özgün üçüncü taraf telif/lisansları korunur.
// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: Gömülü editörün seçenek komutları ve başlık yerleşimi için ortak sanal sözleşmedir.
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

// Shared optional editor interface. The standalone project has no rack dependency.
struct RackEditorCommands
{
    // ~RackEditorCommands alt adımı: Gömülü editörün seçenek komutları ve başlık yerleşimi için ortak sanal sözleşmedir.
    virtual ~RackEditorCommands() = default;
    virtual juce::PopupMenu rackOptions() = 0;
    virtual void rackCommand(int command) = 0;
    virtual bool rackCanEdit() const = 0;
    virtual bool rackIsSequencing() const = 0;
    // rackDockHeader alt adımı: Gömülü editörün seçenek komutları ve başlık yerleşimi için ortak sanal sözleşmedir.
    virtual void rackDockHeader(int) {}
    // rackHeaderHeight alt adımı: Gömülü editörün seçenek komutları ve başlık yerleşimi için ortak sanal sözleşmedir.
    virtual int rackHeaderHeight() const
    {
        return 200;
    }
};
