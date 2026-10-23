// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 Fazıl Irak; Shaper eklemeleri/değişiklikleri.
// Fazıl Irak tarafından oluşturuldu / değiştirildi: 23.10.2026
// Kapsam: Shaper için eklenen kod ve uyarlamalar; özgün üçüncü taraf telif/lisansları korunur.
// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: Geliştirici, üçüncü taraf kaynakları ve lisans/bilgilendirme görünümünü kurar.
#include "AboutPanel.h"
namespace gate12
{
juce::Component *createAboutPage();
}
namespace time12
{
juce::Component *createAboutPage();
}
namespace filtr
{
juce::Component *createAboutPage();
}
namespace reevr
{
juce::Component *createAboutPage();
}
namespace pan12
{
juce::Component *createAboutPage();
}
namespace volumef
{
juce::Component *createAboutPage();
}
namespace irak
{
juce::Component *createAboutPage();
}
namespace pitchfx
{
juce::Component *createAboutPage();
}
namespace
{
class Page final : public juce::Component
{
    std::unique_ptr<juce::Component> content;
    int effect;

  public:
    // Page alt adımı: Geliştirici, üçüncü taraf kaynakları ve lisans/bilgilendirme görünümünü kurar.
    explicit Page(juce::Component *component, int effect) : content(component), effect(effect)
    {
        addAndMakeVisible(*content);
    }
    // Pencere/kontrol boyutuna bağlı yerleşimi yeniden hesaplar; hit-test aynı geometriyi izlemelidir.
    void resized() override
    {
        content->setBounds(getLocalBounds());
    }
    // Fazıl Irak tarafından değiştirildi: 23.10.2026; ortak lisans/telif bildirimi.
    void paintOverChildren(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().reduced(20);
        bounds = bounds.removeFromBottom(100);
        g.setColour(juce::Colours::white);
        g.setFont(juce::FontOptions(14.0f));
        const auto effectLicence = effect == 2 ? "AGPLv3" : "GPLv3";
        g.drawText(juce::String(ShaperProcessor::effectName(effect)) + ": " + effectLicence,
                   bounds.removeFromTop(20), juce::Justification::centred);
        g.drawText("JUCE 8.0.7: Copyright (c) Raw Material Software Ltd. - AGPLv3",
                   bounds.removeFromTop(20), juce::Justification::centred);
        if (effect == 7)
        {
            g.drawText("Signalsmith Stretch: Copyright (c) 2022 Geraint Luff / Signalsmith Audio Ltd. - MIT",
                       bounds.removeFromTop(20), juce::Justification::centred);
            g.drawText("Signalsmith Linear: Copyright (c) 2025 Signalsmith Audio - MIT",
                       bounds.removeFromTop(20), juce::Justification::centred);
        }
        else
            bounds.removeFromTop(40);
        g.drawText("No warranty. Licences: LICENSE, LICENSE-AGPL-3.0.txt, THIRD-PARTY-NOTICES.txt",
                   bounds.removeFromTop(20), juce::Justification::centred);
    }
};
} // namespace
// AboutPanel alt adımı: Geliştirici, üçüncü taraf kaynakları ve lisans/bilgilendirme görünümünü kurar.
// Gövdedeki başlıca çağrılar: setComponentID → addAndMakeVisible → setVisible → onClose. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
AboutPanel::AboutPanel()
{
    setComponentID("rack-about");
    addAndMakeVisible(tabs);
    addAndMakeVisible(close);
    close.onClick = [this]
    {
        setVisible(false);
        if (onClose)
            onClose();
    };
    juce::Component *pages[]{gate12::createAboutPage(), time12::createAboutPage(), filtr::createAboutPage(),
                             reevr::createAboutPage(),  pan12::createAboutPage(),  volumef::createAboutPage(),
                             irak::createAboutPage(),   pitchfx::createAboutPage()};
    for (int e = 0; e < ShaperProcessor::effectCount; ++e)
        tabs.addTab(ShaperProcessor::effectName(e), rackEffectColour(e).withMultipliedBrightness(0.25f),
                    new Page(pages[e], e), true);
    tabs.setTabBarDepth(36);
}
// Mevcut kontrol/durumun görselini çizer; ses callback’inde çağrılmamalıdır.
void AboutPanel::paint(juce::Graphics &g)
{
    g.fillAll(juce::Colour(0xee10141b));
    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(20, juce::Font::bold));
    g.drawText("FAZIL IRAK  /  SHAPER", 40, 18, getWidth() - 190, 30, juce::Justification::centredLeft);
}
// Pencere/kontrol boyutuna bağlı yerleşimi yeniden hesaplar; hit-test aynı geometriyi izlemelidir.
void AboutPanel::resized()
{
    close.setBounds(getWidth() - 135, 18, 95, 30);
    tabs.setBounds(40, 60, getWidth() - 80, getHeight() - 100);
}
