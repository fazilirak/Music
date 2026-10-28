// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (c) 2026 Fazıl Irak
// Geliştirici: Fazıl Irak
// Son düzenleme: 26.10.2026
// Dosyanın görevi: GUI kontrollerinin, attachment’ların, tema, timer ve asenkron dosya seçicisinin sahipliği.
#pragma once
#include "PluginProcessor.h"
#include "RackEditorCommands.h"
#include "ConcentricKnob.h"
#include "DeskKnob.h"

namespace irak
{
// Yalnız mesaj/arayüz iş parçacığında çalışan panel; ses işleme burada yapılmaz.
class Editor final : public juce::AudioProcessorEditor, public RackEditorCommands, private juce::Timer
{
    // Gömülü Book/Demi fontları ve buton renkleri; kaynak fontların lisans dosyalarını koruyun.
    class Theme final : public juce::LookAndFeel_V4
    {
      public:
        Theme();
        juce::Typeface::Ptr book, demi;
        // Mesaj iş parçacığında istenen font ağırlığını gömülü Book/Demi typeface’e eşler; üçüncü taraf font lisansı değiştirilmez.
        juce::Typeface::Ptr getTypefaceForFont(const juce::Font &font) override
        {
            return font.isBold() ? demi : book;
        }
        // Düğme fontunun boyutunu belirler; ses parametresi veya DSP katsayısı değildir.
        juce::Font getTextButtonFont(juce::TextButton &button, int) override
        {
            return juce::FontOptions(button.getComponentID().startsWith("desk-") ? 9.5f : button.getComponentID().startsWith("irak-character-") ? 12.0f : 14.0f);
        }
        void drawButtonText(juce::Graphics&, juce::TextButton&, bool, bool) override;
        void drawRotarySlider(juce::Graphics&, int, int, int, int, float, float, float, juce::Slider&) override;
        void drawButtonBackground(juce::Graphics &, juce::Button &, const juce::Colour &, bool,
                                  bool) override;
    } theme;
    // Processor editörden daha uzun yaşar; kontroller onun APVTS parametrelerine bağlanır.
    Processor &processor;
    // Beş karakter seçicisi ve dört ayrı, üç halkalı kontrol grubu.
    std::array<juce::TextButton, 5> characters;
    std::array<std::unique_ptr<ConcentricKnob>, 4> groups;
    juce::ToggleButton heat{"HEAT +20"}, autoGain{"AUTO GAIN"}, bypass{"BYPASS"};
    // Attachment nesneleri kontrollerin ömrü boyunca yaşamalıdır; otomasyonla GUI’yi eşler.
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> heatAttachment, autoAttachment,
        bypassAttachment;
    // Solda Transient/Comp, sağda Bass/Treble. Her kontrol ayrı otomasyon/preset alanına bağlıdır.
    std::array<std::unique_ptr<DeskKnob>, 4> deskKnobs;
    std::array<juce::TextButton, 4> deskSwitches;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>, 4> deskAttachments;
    juce::ComboBox presets;
    juce::TextButton previous{"<"}, next{">"}, save{"Save"}, load{"Load"};
    // Asenkron preset penceresi callback bitene kadar saklanır; callback SafePointer kullanır.
    std::unique_ptr<juce::FileChooser> chooser;
    // Sıfır bağımsız panel; pozitif değer gömme başlığına ayrılan alan. Bu yapı bir rack uygulaması gerektirmez.
    int dockLeft = 0;
    int displayedProgram = -1;
    float inputLevel = 0, outputLevel = 0;
    juce::Rectangle<float> graphBounds, inputMeter, outputMeter;
    // Çizim, seviye güncellemesi ve dosya seçimi yardımcıları; timer 30 Hz ile ekranı yeniler.
    void meter(juce::Graphics &, juce::Rectangle<float>, float, const char *);
    void timerCallback() override;
    void choosePreset(bool saving);
    void curve(juce::Graphics &, juce::Rectangle<float>);

  public:
    explicit Editor(Processor &);
    ~Editor() override;
    void paint(juce::Graphics &) override;
    void resized() override;
    juce::PopupMenu rackOptions() override;
    void rackCommand(int) override;
    // F-Saturator gömme sözleşmesinde sekans/düzenleme özellikleri sunmaz.
    bool rackCanEdit() const override
    {
        return false;
    }
    bool rackIsSequencing() const override
    {
        return false;
    }
    void rackDockHeader(int left) override
    {
        dockLeft = left;
        resized();
        repaint();
    }
    // Gömme yerleşim sözleşmesindeki başlık yüksekliği. rackDockHeader ve resized aynı koordinat varsayımını paylaşır.
    int rackHeaderHeight() const override
    {
        return 210;
    }
};
} // namespace irak
