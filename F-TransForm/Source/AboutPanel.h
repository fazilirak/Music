// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (c) 2026 Fazıl Irak
// Geliştirici: Fazıl Irak | Oluşturma / son düzenleme: 27.10.2026
// Dosyanın görevi: Logodan açılan çevrimdışı About ve tam lisans okuyucusu.
#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
namespace ft {
// Logo bir yazı bileşenidir; çerçeveli düğme değildir. Fare ve klavyeyle aynı sayfayı açar.
class ProductLogo final : public juce::Component,public juce::SettableTooltipClient {
public:
 ProductLogo();
 void paint(juce::Graphics&) override;
 void mouseUp(const juce::MouseEvent&) override;
 bool keyPressed(const juce::KeyPress&) override;
 std::function<void()> onActivate;
};
class AboutPanel final : public juce::Component {
public:
 AboutPanel();
 void paint(juce::Graphics&) override;
 void resized() override;
 bool keyPressed(const juce::KeyPress&) override;
 std::function<void()> onClose;
private:
 juce::TextButton close{"Close"};
 juce::ComboBox documents;
 juce::TextEditor reader;
 void selectDocument();
 void dismiss();
};
}
