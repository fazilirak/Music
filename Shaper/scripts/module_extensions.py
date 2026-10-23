# SPDX-License-Identifier: GPL-3.0-only
# Copyright (c) 2026 Fazıl Irak; Shaper eklemeleri/değişiklikleri.
# Fazıl Irak tarafından oluşturuldu / değiştirildi: 23.10.2026
# Kapsam: Shaper için eklenen kod ve uyarlamalar; özgün üçüncü taraf telif/lisansları korunur.
# Türkçe bakım açıklamaları: Fazıl Irak
# Son düzenleme: 22.10.2026
# Görev: Volume/Pan/Time ve diğer gömülü modüllerin kaynak dönüştürme eklentilerini tanımlar.
"""Small source adaptations kept separate from the namespace/vendor bridge."""
import re


# insert_function: Volume/Pan/Time ve diğer gömülü modüllerin kaynak dönüştürme eklentilerini tanımlar.
def insert_function(text, signature, code, beginning=False):
    start = text.index('{', text.index(signature))
    if beginning:
        return text[:start + 1] + '\n' + code + text[start + 1:]
    depth, end = 1, start + 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[:end - 1] + '\n' + code + '\n' + text[end - 1:]



# extend_volume: Volume/Pan/Time ve diğer gömülü modüllerin kaynak dönüştürme eklentilerini tanımlar.
def extend_volume(path, text):
    if path.name == 'Globals.h':
        text = text.replace('0xffff8050','0xffffd42a').replace('0xffffb193','0xffffeb93').replace('0xff88442b','0xff88701b')
    elif path.name == 'PluginProcessor.h':
        text = text.replace('#include "Presets.h"', '#include "Presets.h"\n#include <SidechainPresets.h>')
        text = text.replace('    // Instance Settings', '''    int sidechainPreset = 0;
    void loadSidechainPreset(int index);
    // Instance Settings''', 1)
    elif path.name == 'PluginProcessor.cpp':
        # Quarter-note ducking, ready to use in tempo sync. Audio/MIDI modes remain available.
        text = text.replace('"1/4.", "1/2.", "1/1." }, 9)', '"1/4.", "1/2.", "1/1." }, 7)')
        text = text.replace('    loadSettings();\n}', '    loadSettings();\n    loadSidechainPreset(0);\n}', 1)
        start = text.index('int GATE12AudioProcessor::getNumPrograms()')
        end = text.index('void GATE12AudioProcessor::changeProgramName', start)
        text = text[:start] + '''int GATE12AudioProcessor::getNumPrograms() { return SidechainPresets::count; }
int GATE12AudioProcessor::getCurrentProgram() { return sidechainPreset; }
void GATE12AudioProcessor::setCurrentProgram(int index) { loadSidechainPreset(index); }
void GATE12AudioProcessor::loadProgram(int index) { loadSidechainPreset(index); }
void GATE12AudioProcessor::loadSidechainPreset(int index) {
    index=juce::jlimit(0,SidechainPresets::count-1,index);
    if(sequencer->isOpen) sequencer->close();
    auto snapshot=viewPattern->points;
    if(auto xml=juce::parseXML(SidechainPresets::state(index))) {
        xml->setAttribute("sidechainPreset",index); xml->setAttribute("currentProgram",index);
        juce::MemoryBlock data; juce::AudioProcessor::copyXmlToBinary(*xml,data); setStateInformation(data.getData(),(int)data.getSize());
    }
    viewPattern=pattern; uimode=UIMode::Normal; showPaintWidget=false; showSequencer=false;
    createUndoPointFromSnapshot(snapshot);
    sidechainPreset=index; currentProgram=index; sendChangeMessage();
}
const juce::String GATE12AudioProcessor::getProgramName(int index) { return SidechainPresets::name(juce::jlimit(0,SidechainPresets::count-1,index)); }

''' + text[end:]
        text = text.replace('state.setProperty("currentProgram", currentProgram, nullptr);', 'state.setProperty("currentProgram", currentProgram, nullptr);\n    state.setProperty("sidechainPreset", sidechainPreset, nullptr);')
        text = text.replace('        currentProgram = (int)state.getProperty("currentProgram");', '        currentProgram = (int)state.getProperty("currentProgram");\n        sidechainPreset=juce::jlimit(0,SidechainPresets::count-1,(int)state.getProperty("sidechainPreset"));')
    elif path.name == 'PluginEditor.h':
        text = text.replace('#include "ui/SettingsButton.h"', '#include "ui/SettingsButton.h"\n#include <SidechainPresets.h>')
        text = text.replace('    Label logoLabel;', '    void layoutSidechainButtons();\n    std::array<std::unique_ptr<SidechainPresetButton>,SidechainPresets::count> duckButtons;\n    Label logoLabel;')
    elif path.name == 'PluginEditor.cpp':
        code='''    for(int i=0;i<SidechainPresets::count;++i) {
        duckButtons[i]=std::make_unique<SidechainPresetButton>(i);
        Pattern preview(0); for(const auto& point:SidechainPresets::points(i)) preview.insertPoint(point.x,point.y,point.tension,point.type);
        if(auto xml=juce::parseXML(SidechainPresets::state(i))) {
            auto parameters=juce::ValueTree::fromXml(*xml->getChildElement(0));
            auto value=[&](const char* id) { return (double)parameters.getChildWithProperty("id",id).getProperty("value"); };
            preview.setTension(value("tension"),value("tensionatk"),value("tensionrel"),xml->getBoolAttribute("dualTension"));
        }
        preview.buildSegments(); duckButtons[i]->setPreview([&preview](double x) { return 1.0-preview.get_y_at(x); });
        addAndMakeVisible(*duckButtons[i]);
        duckButtons[i]->onClick=[this,i] { const juce::ScopedLock lock(audioProcessor.getCallbackLock()); audioProcessor.loadSidechainPreset(i); toggleUIComponents(); };
    }
'''
        text=text.replace('    col += 85;\n\n    addAndMakeVisible(loopButton);', code+'    col += 85;\n\n    addAndMakeVisible(loopButton);', 1)
        text+='''
void GATE12AudioProcessorEditor::layoutSidechainButtons() {
    const int left=pointMenu.getRight()+10+(loopButton.isVisible()?35:0);
    const int right=undoButton.getX()-12;
    const int width=juce::jmax(24,juce::jmin(45,(right-left)/SidechainPresets::count-3));
    for(int i=0;i<SidechainPresets::count;++i) duckButtons[i]->setBounds(left+i*(width+3),pointMenu.getY(),width,25);
}
'''
        text=insert_function(text,'void GATE12AudioProcessorEditor::resized()', '    layoutSidechainButtons();')
        text=insert_function(text,'void GATE12AudioProcessorEditor::toggleUIComponents()', '''    for(int i=0;i<SidechainPresets::count;++i) if(duckButtons[i]) { duckButtons[i]->setSelected(audioProcessor.sidechainPreset==i); duckButtons[i]->setVisible(audioProcessor.uimode!=UIMode::Seq); }
    layoutSidechainButtons();
''')
    return text


# extend_pan: Volume/Pan/Time ve diğer gömülü modüllerin kaynak dönüştürme eklentilerini tanımlar.
def extend_pan(path,text):
    if path.name=='PluginProcessor.h':
        text=text.replace('#include "Presets.h"','#include "Presets.h"\n#include <PanPresets.h>')
        text=text.replace('    // Instance Settings','    int panPreset = 0;\n    void loadPanPreset(int index);\n    // Instance Settings',1)
    elif path.name=='PluginProcessor.cpp':
        text=text.replace('    loadSettings();\n}', '    loadSettings();\n    loadPanPreset(0);\n}',1)
        start=text.index('int GATE12AudioProcessor::getNumPrograms()')
        end=text.index('void GATE12AudioProcessor::changeProgramName',start)
        text=text[:start]+'''int GATE12AudioProcessor::getNumPrograms() { return PanPresets::count; }
int GATE12AudioProcessor::getCurrentProgram() { return panPreset; }
void GATE12AudioProcessor::setCurrentProgram(int index) { loadPanPreset(index); }
void GATE12AudioProcessor::loadProgram(int index) { loadPanPreset(index); }
void GATE12AudioProcessor::loadPanPreset(int index) {
    index=juce::jlimit(0,PanPresets::count-1,index);
    if(sequencer->isOpen) sequencer->close();
    auto snapshot=pattern->points; viewPattern=pattern; uimode=UIMode::Normal; showPaintWidget=false; showSequencer=false;
    pattern->clear(); for(const auto& p:PanPresets::points(index)) pattern->insertPoint(p.x,p.y,p.tension,p.type);
    pattern->buildSegments(); createUndoPointFromSnapshot(snapshot);
    auto set=[this](const char* id,float value) { auto* p=params.getParameter(id); p->setValueNotifyingHost(p->convertTo0to1(value)); };
    set("min",0); set("max",1); set("phase",0); set("stereo",0); set("attack",0.01f); set("release",0.01f); set("sync",(float)PanPresets::sync(index));
    panPreset=index; currentProgram=index; sendChangeMessage();
}
const juce::String GATE12AudioProcessor::getProgramName(int index) { return PanPresets::name(index); }

'''+text[end:]
        text=text.replace('state.setProperty("currentProgram", currentProgram, nullptr);','state.setProperty("currentProgram", currentProgram, nullptr);\n    state.setProperty("panPreset", panPreset, nullptr);')
        text=text.replace('        currentProgram = (int)state.getProperty("currentProgram");','        currentProgram = (int)state.getProperty("currentProgram");\n        panPreset=juce::jlimit(0,PanPresets::count-1,(int)state.getProperty("panPreset"));')
    return text


# extend_time_quick: Volume/Pan/Time ve diğer gömülü modüllerin kaynak dönüştürme eklentilerini tanımlar.
def extend_time_quick(path,text):
    if path.name=='PluginProcessor.h':
        text=text.replace('#include "Presets.h"','#include "Presets.h"\n#include <TimeQuickPresets.h>')
        text=text.replace('    // Instance Settings','    void loadTimeQuickPreset(int index);\n    // Instance Settings',1)
    elif path.name=='PluginProcessor.cpp':
        text=text.replace('    return 79;','    return TimeQuickPresets::programOffset+TimeQuickPresets::count;',1)
        for signature in ('void TIME12AudioProcessor::setCurrentProgram','void TIME12AudioProcessor::loadProgram'):
            text=insert_function(text,signature,'    if(index>=TimeQuickPresets::programOffset) { loadTimeQuickPreset(index-TimeQuickPresets::programOffset); return; }',True)
        text=insert_function(text,'const juce::String TIME12AudioProcessor::getProgramName','    if(index>=TimeQuickPresets::programOffset) return TimeQuickPresets::name(index-TimeQuickPresets::programOffset);',True)
        text+='''
void TIME12AudioProcessor::loadTimeQuickPreset(int index) {
    index=juce::jlimit(0,TimeQuickPresets::count-1,index);
    if(sequencer->isOpen) sequencer->close();
    auto snapshot=viewPattern->points; viewPattern=pattern;
    uimode=UIMode::Normal; showPaintWidget=false; showSequencer=false; outputCV=false; useMonitor=false; useSidechain=false;
    pattern->clear(); for(const auto& point:TimeQuickPresets::points(index)) pattern->insertPoint(point.x,point.y,point.tension,point.type);
    pattern->buildSegments(); createUndoPointFromSnapshot(snapshot);
    auto set=[this](const char* id,float value) { auto* p=params.getParameter(id); p->setValueNotifyingHost(p->convertTo0to1(value)); };
    set("sync",9); set("trigger",0); set("phase",0); set("min",0); set("max",1); set("mix",1); set("smooth",0); set("attack",0); set("release",0);
    set("tension",0); set("tensionatk",0); set("tensionrel",0); set("anoise",2);
    currentProgram=TimeQuickPresets::programOffset+index; sendChangeMessage();
}
'''
    elif path.name=='PluginEditor.h':
        text=text.replace('#include "ui/SettingsButton.h"','#include "ui/SettingsButton.h"\n#include <TimeQuickPresets.h>')
        text=text.replace('    Label logoLabel;','    void layoutTimeQuickButtons();\n    std::array<std::unique_ptr<TimeQuickPresetButton>,TimeQuickPresets::count> quickButtons;\n    Label logoLabel;')
    elif path.name=='PluginEditor.cpp':
        code='''    for(int i=0;i<TimeQuickPresets::count;++i) {
        quickButtons[i]=std::make_unique<TimeQuickPresetButton>(i); addAndMakeVisible(*quickButtons[i]);
        quickButtons[i]->onClick=[this,i] { const juce::ScopedLock lock(audioProcessor.getCallbackLock()); audioProcessor.loadTimeQuickPreset(i); toggleUIComponents(); };
    }
'''
        text=text.replace('    col += 85;\n\n    addAndMakeVisible(loopButton);',code+'    col += 85;\n\n    addAndMakeVisible(loopButton);',1)
        text+='''
void TIME12AudioProcessorEditor::layoutTimeQuickButtons() {
    const int left=pointMenu.getRight()+10+(loopButton.isVisible()?35:0),right=undoButton.getX()-12;
    const int width=juce::jmax(24,juce::jmin(45,(right-left)/TimeQuickPresets::count-3));
    for(int i=0;i<TimeQuickPresets::count;++i) quickButtons[i]->setBounds(left+i*(width+3),pointMenu.getY(),width,25);
}
'''
        text=insert_function(text,'void TIME12AudioProcessorEditor::resized()','    layoutTimeQuickButtons();')
        text=insert_function(text,'void TIME12AudioProcessorEditor::toggleUIComponents()','''    for(int i=0;i<TimeQuickPresets::count;++i) if(quickButtons[i]) { quickButtons[i]->setSelected(audioProcessor.getCurrentProgram()==TimeQuickPresets::programOffset+i); quickButtons[i]->setVisible(audioProcessor.uimode!=UIMode::Seq); }
    layoutTimeQuickButtons();''')
    return text
