// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 Fazıl Irak
// Fazıl Irak — 26.10.2026: Kullanıcı metinleri İngilizcedir; tam lisanslar gömülü kaynaklardan okunur.
#include "AboutPanel.h"
#include <fs_licenses.h>
namespace {juce::String utf8(const char* s) {return juce::String::fromUTF8(s);}}

AboutPanel::AboutPanel()
{
    setComponentID("about-panel");
    setOpaque(true);
    addAndMakeVisible(close);
    close.onClick=[this]{setVisible(false);};
    addAndMakeVisible(documents);
    documents.addItem("Licence overview",1);
    documents.addItem(utf8("GPLv3 — F-Sidechain / GATE-12"),2);
    documents.addItem(utf8("AGPLv3 — JUCE"),3);
    documents.addItem("Third-party notices",4);
    documents.addItem("Ubuntu Font Licence",5);
    documents.addItem("JUCE licence notice",6);
    documents.addItem("VST3 SDK licence notice",7);
    documents.setComponentID("license-selector");
    documents.onChange=[this]{selectDocument();};
    addAndMakeVisible(text);
    text.setComponentID("license-text");
    text.setMultiLine(true,true);
    text.setReadOnly(true);
    text.setScrollbarsShown(true);
    text.setCaretVisible(false);
    text.setPopupMenuEnabled(true);
    text.setFont(juce::FontOptions(13.0f));
    text.setColour(juce::TextEditor::backgroundColourId,juce::Colour(0xff10151c));
    text.setColour(juce::TextEditor::textColourId,juce::Colour(0xffd0d6de));
    text.setColour(juce::TextEditor::outlineColourId,juce::Colour(0xff3b424c));
    documents.setSelectedId(1,juce::dontSendNotification);
    selectDocument();
}

void AboutPanel::selectDocument()
{
    // Dosya/ağ erişimi yapılmaz; derlemeye gömülen özgün lisans metni seçilir.
    const char* name="LICENSE_txt";
    switch(documents.getSelectedId()) {
        case 2:name="LICENSE";break;
        case 3:name="LICENSEAGPL3_0_txt";break;
        case 4:name="THIRDPARTYNOTICES_txt";break;
        case 5:name="UbuntuFontLicence1_0_txt";break;
        case 6:name="JUCELICENSE_md";break;
        case 7:name="VST3LICENSE_txt";break;
        default:break;
    }
    int size=0;
    auto* content=fs_licenses::getNamedResource(name,size);
    text.setText(content?juce::String::fromUTF8(content,size):"Licence text could not be loaded.",false);
    text.moveCaretToTop(false);
}

void AboutPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff181818));
    g.setColour(juce::Colour(0xffffd42a));
    g.setFont(juce::FontOptions(26.0f,juce::Font::bold));
    g.drawText("F-Sidechain",22,14,240,36,juce::Justification::centredLeft);
    g.setFont(juce::FontOptions(13.0f));
    g.setColour(juce::Colour(0xffaeb7c4));
    g.drawText("VOLUME SHAPER  /  v" PROJECT_VERSION,24,53,420,22,juce::Justification::centredLeft);
    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(15.0f));
    g.drawText(utf8("Design and development: Fazıl Irak  ·  © 2026"),24,84,getWidth()-48,24,juce::Justification::centredLeft);
    g.setFont(juce::FontOptions(12.5f));
    g.setColour(juce::Colour(0xffaeb7c4));
    g.drawText(utf8("Volume and envelope engine: Shaper / GATE-12 — Tilr (tiagolr)"),24,111,getWidth()-48,20,juce::Justification::centredLeft);
    g.drawText(utf8("F-Sidechain: GPLv3  ·  JUCE: AGPLv3  ·  VST3 SDK: GPLv3  ·  Ubuntu Font Licence 1.0"),24,133,getWidth()-48,20,juce::Justification::centredLeft);
    g.setFont(juce::FontOptions(11.5f));
    g.drawText("Source code and dependencies are included in the package. Provided without warranty.",24,getHeight()-34,getWidth()-48,20,juce::Justification::centredLeft);
}

void AboutPanel::resized()
{
    close.setBounds(getWidth()-92,22,68,27);
    documents.setBounds(24,166,getWidth()-48,28);
    text.setBounds(24,204,getWidth()-48,juce::jmax(70,getHeight()-250));
}
