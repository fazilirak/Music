// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (c) 2026 Fazıl Irak
// Geliştirici: Fazıl Irak | Oluşturma / son düzenleme: 27.10.2026
// Dosyanın görevi: Fazıl Irak ürün atfı, özgün kod/bağımlılık ayrımı ve gömülü lisans metinlerini göstermek.
#include "AboutPanel.h"
#include "FTLicences.h"
namespace ft {
ProductLogo::ProductLogo() {
 setComponentID("product-logo");setName("F-TransForm About and licences");
 setTitle("F-TransForm");setWantsKeyboardFocus(true);
 setMouseCursor(juce::MouseCursor::PointingHandCursor);setTooltip("About F-TransForm and licences");
}
void ProductLogo::paint(juce::Graphics& g){
 g.setColour(juce::Colour(0xffe5e7df));g.setFont(juce::FontOptions(32));
 g.drawText("F-TransForm",getLocalBounds(),juce::Justification::centredLeft);
}
void ProductLogo::mouseUp(const juce::MouseEvent& e){if(getLocalBounds().contains(e.getPosition())&&onActivate)onActivate();}
bool ProductLogo::keyPressed(const juce::KeyPress& key){
 if(key==juce::KeyPress::returnKey||key==juce::KeyPress::spaceKey){if(onActivate)onActivate();return true;}return false;
}
AboutPanel::AboutPanel(){
 setComponentID("about-panel");setName("About F-TransForm");setOpaque(true);setWantsKeyboardFocus(true);
 addAndMakeVisible(close);close.setComponentID("about-close");close.onClick=[this]{dismiss();};
 addAndMakeVisible(documents);documents.setComponentID("about-documents");
 documents.addItem("Licence and ownership overview",1);documents.addItem("GNU AGPLv3 - original application",2);
 documents.addItem("Third-party notices",3);documents.addItem("JUCE licence notice",4);
 documents.addItem("URW Gothic font licence and exception",5);documents.addItem("VST3 SDK licence notice",6);
 documents.addItem("GNU GPLv3 - VST3 SDK option",7);
 documents.onChange=[this]{selectDocument();};
 addAndMakeVisible(reader);reader.setComponentID("about-text");reader.setMultiLine(true,true);
 reader.setReadOnly(true);reader.setScrollbarsShown(true);reader.setCaretVisible(false);
 reader.setFont(juce::FontOptions(13));reader.setColour(juce::TextEditor::backgroundColourId,juce::Colour(0xff101516));
 reader.setColour(juce::TextEditor::textColourId,juce::Colour(0xffe5e7df));
 reader.setColour(juce::TextEditor::outlineColourId,juce::Colour(0xff343c3e));
 documents.setSelectedId(1,juce::dontSendNotification);selectDocument();
}
void AboutPanel::selectDocument(){
 // Bütün metinler ikiliye gömülüdür; dosya veya internet erişimi gerekmez.
 const char* names[]{"Overview_txt","AGPL_txt","Notices_txt","JUCE_txt","Font_txt","VST3_txt","GPL_txt"};
 int size=0;auto* content=ft_licences::getNamedResource(names[juce::jlimit(0,6,documents.getSelectedId()-1)],size);
 reader.setText(content?juce::String::fromUTF8(content,size):"Licence document unavailable.",false);
 reader.moveCaretToTop(false);
}
void AboutPanel::dismiss(){setVisible(false);if(onClose)onClose();}
bool AboutPanel::keyPressed(const juce::KeyPress& k){if(k==juce::KeyPress::escapeKey){dismiss();return true;}return false;}
void AboutPanel::resized(){
 close.setBounds(getWidth()-112,32,80,30);documents.setBounds(32,206,getWidth()-64,30);
 reader.setBounds(32,250,getWidth()-64,getHeight()-298);
}
void AboutPanel::paint(juce::Graphics& g){
 g.fillAll(juce::Colour(0xff141819));g.setColour(juce::Colour(0xffe5e7df));
 g.setFont(juce::FontOptions(32));g.drawText("F-TransForm",32,28,400,44,juce::Justification::centredLeft);
 g.setColour(juce::Colour(0xffed6c46));g.setFont(juce::FontOptions(14));
 g.drawText("MULTI-EFFECT TRANSITION ENGINE  /  v" FT_VERSION,34,78,800,24,juce::Justification::centredLeft);
 g.setColour(juce::Colour(0xffe5e7df));g.setFont(juce::FontOptions(17));
 g.drawText(juce::String::fromUTF8("Design, original application code and DSP: Fazıl Irak"),34,116,1000,28,juce::Justification::centredLeft);
 g.setFont(juce::FontOptions(13));g.setColour(juce::Colour(0xff8d9799));
 g.drawText(juce::String::fromUTF8("Copyright © 2026 Fazıl Irak  /  AGPL-3.0-or-later"),34,150,1000,22,juce::Justification::centredLeft);
 g.drawText("JUCE, VST3 SDK and URW Gothic retain their original authors and licences.",34,176,1000,22,juce::Justification::centredLeft);
 g.setFont(juce::FontOptions(10));
 g.drawText("Offline licence reader  /  Source: github.com/fazilirak/Music  /  Provided without warranty.",32,getHeight()-34,getWidth()-64,18,juce::Justification::centredLeft);
}
}
