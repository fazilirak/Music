// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (c) 2026 Fazıl Irak
// Geliştirici: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: İsteğe bağlı gömülü panel komut sözleşmesi; bağımsız üründe rack bağımlılığı değildir.
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

// Shared optional editor interface. The standalone project has no rack dependency.
// İsteğe bağlı gömme arayüzü: bu sözleşme bağımsız F-Saturator için bir rack bağımlılığı oluşturmaz.
// 9001–9003 komutlarının karşılığı PluginEditor.cpp içindedir; kimlikleri birlikte güncelleyin.
struct RackEditorCommands
{
    // Sanal yıkıcı, türetilmiş düzenleyicinin taban işaretçisi üzerinden güvenle silinmesini sağlar.
    virtual ~RackEditorCommands() = default;
    // Gömüldüğü uygulamanın açacağı seçenek menüsünü üretir.
    virtual juce::PopupMenu rackOptions() = 0;
    // Seçilen menü komutunu parametre değişikliğine dönüştürür.
    virtual void rackCommand(int command) = 0;
    // Gömme katmanına düzenleme ve sekans yeteneklerini bildirir; F-Saturator bunları kullanmaz.
    virtual bool rackCanEdit() const = 0;
    virtual bool rackIsSequencing() const = 0;
    // Başlık yerleşimine ayrılan sol alanı ve yüksekliği bildirir; varsayılanlar bağımsız kullanıma uygundur.
    virtual void rackDockHeader(int) {}
    virtual int rackHeaderHeight() const
    {
        return 200;
    }
};
