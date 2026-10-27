// Geliştirici: Fazıl Irak - Son düzenleme: 27.10.2026
// Dosyanın görevi: Envelope düzenleme ve ürün kontrol bileşeni; özgün kaynak atıfları korunur.
// Katkı kapsamı: F-Sidechain bakımı ve uyarlamaları. GATE-12/Shaper özgün telifleri korunur.
// Fazıl Irak tarafından değiştirildi: 23.10.2026; Shaper uyarlaması.
// Özgün telif ve lisans bildirimleri aşağıda korunur.
#include <JuceHeader.h>
#include <gate12_data.h>
#pragma once

#include <JuceHeader.h>
#include <functional>

namespace volumef {

class VOLUMEFAudioProcessor;

class SettingsButton : public juce::Component {
public:
    SettingsButton(VOLUMEFAudioProcessor& p) : audioProcessor(p) {}
    ~SettingsButton() override {}

    PopupMenu makeOptionsMenu();
    void executeCommand(int result);
    void mouseDown(const juce::MouseEvent& e) override;
    void paint(Graphics& g) override;

    std::function<void()> onScaleChange;
    std::function<void()> toggleUIComponents;
    std::function<void()> toggleAbout;

private:
    VOLUMEFAudioProcessor& audioProcessor;
};
} // namespace volumef
