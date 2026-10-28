# SPDX-License-Identifier: GPL-3.0-only
# Copyright (c) 2026 Fazıl Irak; Shaper eklemeleri/değişiklikleri.
# Fazıl Irak tarafından oluşturuldu / değiştirildi: 23.10.2026
# Kapsam: Shaper için eklenen kod ve uyarlamalar; özgün üçüncü taraf telif/lisansları korunur.
# Türkçe bakım açıklamaları: Fazıl Irak
# Son düzenleme: 22.10.2026
# Görev: Upstream kaynaklardan namespace ve Shaper standart UI uyarlamalarını build/modules altında üretir; üretilmiş dosya yerine bu üreticiyi düzenleyin.
"""Create namespaced build copies, preserving the original repositories untouched."""
import pathlib, re, subprocess, json, sys
from module_extensions import extend_volume, extend_pan, extend_time_quick, insert_function
from editor_layout import editor_methods, paint_sections
root = pathlib.Path(__file__).resolve().parents[1]
out = pathlib.Path(sys.argv[1]) if len(sys.argv) > 1 else root / 'build' / 'modules'
# Remove only the four generated Time extension files from releases before 0.4.
for relative in ('dsp/Splitter.h','dsp/Splitter.cpp','ui/BandsWidget.h','ui/BandsWidget.cpp'):
    obsolete=out/'time12'/relative
    if obsolete.is_file(): obsolete.unlink()
manifest = {}
for name in ('gate12', 'time12', 'filtr', 'reevr', 'pan12', 'volumef','satcurve','pitchcurve'):
    original = 'gate12' if name in ('pan12','volumef','satcurve','pitchcurve') else name
    src = root / 'upstream' / original / 'src'
    if name not in ('pan12','volumef','satcurve','pitchcurve'):
        if (src.parent / '.git').exists():
            manifest[name] = subprocess.check_output(['git', '-c', 'safe.directory=' + src.parent.as_posix(), '-C', str(src.parent), 'rev-parse', 'HEAD'], text=True).strip()
        else:
            # Temiz taşımada git nesneleri yoktur; kaynak sürümü doğrulanmış lock kaydından gelir.
            manifest[name] = json.loads((root / 'upstream-lock.json').read_text(encoding='utf-8'))[name]
    paths=list(src.rglob('*'))
    extras={}
    for path in paths:
        if path.suffix not in ('.h', '.cpp'): continue
        text = extras[path] if path in extras else path.read_text(encoding='utf-8-sig')
        if name == 'reevr' and path.name == 'Impulse.cpp':
            # Fazıl Irak, 23.10.2026: bağımsız üretilen dört kanallı IR kaynağı.
            text = text.replace('Hall_Quad_flac', 'Shaper_Hall_Quad_wav')
        if name=='time12': text=extend_time_quick(path,text)
        if name=='volumef': text=extend_volume(path,text)
        if name=='pan12': text=extend_pan(path,text)
        if name=='reevr' and path.name=='PluginProcessor.cpp':
            # Factory XMLs carry currentProgram=-1. Restore the user's selected
            # index after loading; ordinary state recall still uses its stored ID.
            text=insert_function(text,'void REEVRAudioProcessor::loadProgram',
                                 '    currentProgram = index;')
        if path.name == 'SettingsButton.h':
            text = text.replace('    void mouseDown', '    PopupMenu makeOptionsMenu();\n    void executeCommand(int result);\n    void mouseDown')
        if path.name == 'SettingsButton.cpp':
            # Keep the original per-effect Options and editing implementation, expose
            # them through the rack, and eliminate the old hamburger popup entirely.
            prefix = text[:text.index('void SettingsButton::paint')]
            options_start = text.index('\tPopupMenu midiTriggerChn;')
            options_end = text.index('\tPopupMenu load;')
            options = text[options_start:options_end]
            handler_start = text.index('[this](int result) {') + len('[this](int result) {')
            handler_end = text.rfind('\n\t\t}')
            handler = text[handler_start:handler_end]
            # Drop obsolete UI Scale / Load / About branches as well as their menus.
            obsolete = [m for m in re.finditer(r'else if \(([^\n]+)\)\s*\{', handler)
                        if any(token in m[1] for token in ('result >= 1 && result <= 5', 'result == 109', 'result == 110', 'result >= 100 && result <= 200',
                                                         'result >= 2000', 'result == 1000', 'result == 1001', 'result == 1002'))]
            for branch in reversed(obsolete):
                depth, end = 1, branch.end()
                while depth:
                    depth += (handler[end] == '{') - (handler[end] == '}')
                    end += 1
                handler = handler[:branch.start()] + handler[end:]
            # All commands are invoked on the message thread while the rack suspends
            # audio. Execute synchronously so queued callbacks cannot outlive editors.
            handler = handler.replace('MessageManager::callAsync(', 'invokeNow(')
            text = prefix + '''void SettingsButton::paint(Graphics&) {}
void SettingsButton::mouseDown(const juce::MouseEvent&) {}
PopupMenu SettingsButton::makeOptionsMenu() {
''' + options + '''\n    return options;
}
void SettingsButton::executeCommand(int result) {
    auto invokeNow = [](auto&& action) { action(); };
''' + handler + '\n    audioProcessor.sendChangeMessage();\n}\n'
        if path.name == 'PluginEditor.h':
            # The outer rack owns the only tooltip window. A default-constructed
            # embedded TooltipWindow watches the whole desktop and duplicates it.
            text = re.sub(r'^\s*TooltipWindow tooltipWindow;[^\n]*\n', '\n', text, flags=re.MULTILINE)
            if name=='time12':
                text=text.replace('    Label logoLabel;', '    std::array<std::unique_ptr<Rotary>,7> rackModulation;\n    Label logoLabel;')
            text = text.replace(': public juce::AudioProcessorEditor,', ': public juce::AudioProcessorEditor, public RackEditorCommands,')
            marker = '\nprivate:'
            methods = editor_methods('gate12' if name in ('satcurve','pitchcurve') else name)
            text = text.replace(marker, '\n' + methods + marker, 1)
        if path.name == 'PluginEditor.cpp':
            if name=='time12':
                # Pattern sync now lives on row one; row three must still follow
                # the pattern buttons so it cannot overlap them on refresh.
                text=text.replace(': patSyncMenu.getBottom() + 10', ': patterns.front()->getBottom() + 10')
            text = text.replace('addAndMakeVisible(*settingsButton);', 'addChildComponent(*settingsButton); // Commands live in the rack bar.')
            processor_class={'gate12':'GATE12','time12':'TIME12','filtr':'FILTR','reevr':'REEVR','pan12':'GATE12','volumef':'GATE12','satcurve':'GATE12','pitchcurve':'GATE12'}[name]+'AudioProcessorEditor'
            if name=='time12':
                text=insert_function(text,processor_class+'::'+processor_class, '''
    const char* ids[]={"phase","smooth","attack","release","tension","tensionatk","tensionrel"};
    const char* labels[]={"Phase","Smooth","Attack","Release","Tension","TAtk","TRel"};
    for(size_t i=0;i<rackModulation.size();++i) {
        rackModulation[i]=std::make_unique<Rotary>(p,ids[i],labels[i],RotaryLabel::percx100,i>=4);
        addAndMakeVisible(*rackModulation[i]);
    }
''')
            text=text.replace('g.fillAll(Colour(COLOR_BG));','g.fillAll(Colour(COLOR_BG));'+paint_sections(),1)
            text=insert_function(text,'void '+processor_class+'::resized()', '    rackApplyDock();')
            text=insert_function(text,'void '+processor_class+'::toggleUIComponents()', '    rackApplyDock();')

            text=text.replace('addAndMakeVisible(logoLabel);','addChildComponent(logoLabel);')
            text=insert_function(text,processor_class+'::'+processor_class,'    rackApplyDock();')

            text = text.replace('addAndMakeVisible(bandsBtn);', 'addChildComponent(bandsBtn);')
            # The wrapper owns real per-band processors and its own Bands controls.
            start = text.find('    // band button')
            if start >= 0:
                end = text.index('\n}', start)
                text = text[:start] + text[end:]
            text = text.replace('snapButton.setComponentID("button");', 'snapButton.setComponentID("snap-outline");\n    snapButton.setColour(TextButton::textColourOnId, Colour(COLOR_ACTIVE));')
        if path.name == 'CustomLookAndFeel.cpp':
            # Each editor owns its theme; changing JUCE's global default leaks
            # native styling into sibling rack controls and other instances.
            text=text.replace('this->setDefaultLookAndFeel(this);','')
            text=insert_function(text,'void CustomLookAndFeel::drawButtonBackground', '''    if(button.getComponentID()=="snap-outline" || button.getComponentID().startsWith("effect-")) {
        const auto bounds=button.getLocalBounds().toFloat();
        g.setColour(Colour(COLOR_ACTIVE).withAlpha(button.getToggleState()?1.0f:0.45f));
        g.drawRoundedRectangle(bounds.reduced(0.5f),3.0f,button.getToggleState()?1.5f:1.0f);
        if(isMouseOverButton) { g.setColour(Colour(COLOR_ACTIVE).withAlpha(0.1f)); g.fillRoundedRectangle(bounds,3.0f); }
        return;
    }
''',True)
        if path.name == 'CustomLookAndFeel.h':
            if 'juce::Font getTextButtonFont' not in text:
                text=text.replace('private:', '''
    juce::Font getTextButtonFont(juce::TextButton& button,int buttonHeight) override { return juce::LookAndFeel_V4::getTextButtonFont(button,buttonHeight); }
    juce::Font getComboBoxFont(juce::ComboBox& box) override { return juce::LookAndFeel_V4::getComboBoxFont(box); }
private:''',1)
            text=insert_function(text,'juce::Font getTextButtonFont', '''
        const auto id=button.getComponentID();
        if(id.startsWith("effect-")||id=="snap-outline"||id=="pattern"||id=="leftPattern"||id=="rightPattern") return juce::FontOptions(13.0f);
''',True)
            text=insert_function(text,'juce::Font getComboBoxFont', '''
        if(box.getComponentID().startsWith("effect-")) return juce::FontOptions(14.0f);
''',True)
        if path.name=='AudioWidget.cpp' and name in ('filtr','reevr'):
            text=insert_function(text,'void AudioWidget::resized()', '''
    threshold->setBounds(0,6,68,64); sense->setBounds(72,6,68,64);
    lowcut->setBounds(144,6,68,64); highcut->setBounds(216,6,68,64); offset->setBounds(288,6,68,64);
    audioDisplay->setBounds(364,6,juce::jmax(80,getWidth()-460),62);
    useSidechain.setBounds(getWidth()-88,6,88,25); useMonitor.setBounds(getWidth()-88,39,88,25);
''')
        if name in ('filtr','reevr') and path.name == 'Rotary.h':
            # Fazıl Irak: Bu iki modül rengi constructor'dan alır; Gate/Time bool alanını kullanmaz.
            text=text.replace('    bool isAudioKnob;','')
        if path.name == 'Rotary.cpp':
            if name in ('filtr','reevr'):
                # Kaynakta kendisiyle başlatılan bool rastgele sarı seçimine yol açıyordu.
                # Atanmış renk normal kontrollerde modül rengini, audio kontrollerinde sarıyı korur.
                text=text.replace('    , isAudioKnob(isAudioKnob)\n','')
            knob_colour='color' if name in ('filtr','reevr') else 'isAudioKnob?COLOR_AUDIO:COLOR_ACTIVE'
            text='#include "RackKnob.h"\n'+text
            signature='void Rotary::draw_rotary_slider(juce::Graphics& g, float slider_pos)'
            start=text.index(signature)
            opening=text.index('{',start)
            depth,end=1,opening+1
            while depth:
                depth+=(text[end]=='{')-(text[end]=='}')
                end+=1
            text=text[:opening+1]+('''\n    shaperui::drawKnob(g,getLocalBounds(),slider_pos,juce::Colour(KNOB_COLOUR),isSymmetric,juce::Colour(COLOR_KNOB));
}'''.replace('KNOB_COLOUR',knob_colour))+text[end:]
            # Preserve circular knobs inside the narrower settings panel.
            text=insert_function(text,'void Rotary::paint', '''    auto transform=getTransform();
    if(transform.mat00>0 && transform.mat11>transform.mat00)
        g.addTransform(juce::AffineTransform::scale(1.0f,transform.mat00/transform.mat11));
''',True)
        if name in ('satcurve','pitchcurve'):
            if path.name in ('PluginEditor.cpp','Rotary.cpp','GridSelector.cpp','TextDial.cpp','PaintToolWidget.cpp','SequencerWidget.cpp'):
                # Curve tabs and band switching can close native editors while
                # parameter/UI notifications are still queued on the message loop.
                text=re.sub(r'(?:juce::)?MessageManager::callAsync\(', 'rackCallIfAlive(this,',text)
                marker='GATE12AudioProcessorEditor::GATE12AudioProcessorEditor' if path.name=='PluginEditor.cpp' else path.stem+'::'+path.stem
                helper='''static void rackCallIfAlive(juce::Component* component,std::function<void()> callback) {
    juce::Component::SafePointer<juce::Component> safe(component);
    juce::MessageManager::callAsync([safe,callback=std::move(callback)] { if(safe) callback(); });
}
\n'''
                position=text.index(marker)
                text=text[:position]+helper+text[position:]
            if path.name=='PluginProcessor.h':
                text=text.replace('    static constexpr int GRID_SIZES', '''    bool rackFormantView=false;
    std::shared_ptr<int> rackLifetime=std::make_shared<int>(0);
    void rackPost(std::function<void()> callback) {
        std::weak_ptr<int> life=rackLifetime;
        juce::MessageManager::callAsync([life,callback=std::move(callback)] { if(!life.expired()) callback(); });
    }
    friend void shape(juce::AudioProcessor&,int,float);
    static constexpr int GRID_SIZES''',1)
            if path.name=='Globals.h':
                text=text.replace('0xffff8050','0xffed6c46' if name=='satcurve' else '0xff54dfdb')
                text=text.replace('0xffffb193','0xffffa88a' if name=='satcurve' else '0xffa0f2ef').replace('0xff88442b','0xff873d28' if name=='satcurve' else '0xff287a78')
            if path.name=='PluginProcessor.cpp':
                text=text.replace('buffer.setSample(channel, sampIdx, (float)env);','buffer.setSample(channel, sampIdx, (float)(channel==0?env:env2));')
                text=text.replace('options.applicationName = "GATE-12";', 'options.applicationName = "Shaper-'+name+'";')
                text=re.sub(r'MessageManager::callAsync\(\[this\]\(\)\s*\{\s*sendChangeMessage\(\);\s*\}\);','sendChangeMessage();',text)
                text=text.replace('MessageManager::callAsync(', 'rackPost(')
            if path.name=='Sequencer.cpp':
                text=re.sub(r'MessageManager::callAsync\(\[this\]\(\)\s*\{\s*audioProcessor.sendChangeMessage\(\);(?:\s*//[^\n]*)?\s*\}\);','audioProcessor.sendChangeMessage();',text)
            if path.name=='View.cpp':
                replacement='String ystr = String((.5-pt.y)*(audioProcessor.rackFormantView?24:48),1) + " st";' if name=='pitchcurve' else 'String ystr = String((1.0-pt.y)*100,1) + " %";'
                text=text.replace('String ystr = String("y = ") + dbstr;',replacement)
        if path.name=='About.cpp':
            text=re.sub(r'void About::mouseDown\([^}]+\}', 'void About::mouseDown(const juce::MouseEvent&) {}',text)
        if name == 'pan12':
            text = text.replace('GATE12', 'PAN12').replace('GATE-12', 'Pan-F').replace('gate12', 'pan12')
            if path.name=='About.cpp':
                text=text.replace('"Copyright (C) Tilr 2025"', 'juce::String::fromUTF8("Fazıl Irak | Based on GATE-12 by Tilr")').replace('"github.com/tiagolr/pan12"','"Pan - equal-power stereo balance"')
            if path.name == 'Globals.h':
                text = text.replace('0xffff8050', '0xffff505b').replace('0xffffb193', '0xffff9aa1').replace('0xff88442b', '0xff8e2730')
            if path.name == 'PluginProcessor.cpp':
                text = text.replace('auto wet = (channel == 0 ? lsample * env : rsample * env2);', '''// Equal-power stereo balance. 0 = left, 0.5 = centre, 1 = right.
            const double angle = std::clamp(channel == 0 ? env : env2, 0.0, 1.0) * juce::MathConstants<double>::halfPi;
            auto wet = (channel == 0 ? lsample * std::cos(angle) : rsample * std::sin(angle)) * std::sqrt(2.0);''')
                text = text.replace('patterns[i]->insertPoint(0, 1, 0, 1);', 'patterns[i]->insertPoint(0, 0.5, 0, 1);')
                text = text.replace('patterns[i]->insertPoint(0.5, 0, 0, 1);', '')
                text = text.replace('patterns[i]->insertPoint(1, 1, 0, 1);', 'patterns[i]->insertPoint(1, 0.5, 0, 1);')
                text = text.replace('patterns[i]->loadTriangle();', 'patterns[i]->clear(); patterns[i]->insertPoint(0, 0.5, 0, 1);')
                text = text.replace('"Gate ', '"Pan ').replace('"Mute"', '"Hard Right"')
            if path.name == 'PluginEditor.cpp':
                text = text.replace('"Min"', '"Left limit"').replace('"Max"', '"Right limit"')
        if name=='volumef':
            text=text.replace('GATE12','VOLUMEF').replace('GATE-12','Volume-F').replace('gate12','volumef')
            if path.name=='About.cpp':
                text=text.replace('"Copyright (C) Tilr 2025"','juce::String::fromUTF8("Fazıl Irak | Based on GATE-12 by Tilr")').replace('"github.com/tiagolr/volumef"','"Volume - sidechain volume shaper"')
        # Display names are independent of source namespaces and persistent IDs.
        if path.name in ('PluginEditor.cpp','About.cpp'):
            display={'gate12':'Gate','time12':'Time','filtr':'Filter','reevr':'Reverb','pan12':'Pan','volumef':'Volume','satcurve':'Saturation','pitchcurve':'Pitch'}[name]
            for old in ('GATE-12','TIME-12','FILT-R','REEV-R','Pan-F','Volume-F'):
                text=text.replace('"'+old+'"','"'+display+'"')
        # Conventional include guards become pragma once so the namespace is guarded too.
        guard = re.search(r'^#ifndef (\w+)\s*\n#define \1[^\n]*\n', text, re.M)
        if guard:
            text = text[:guard.start()] + '#pragma once\n' + text[guard.end():]
            end = list(re.finditer(r'^#endif[^\n]*', text, re.M))[-1]
            text = text[:end.start()] + text[end.end():]
        includes = list(re.finditer(r'^\s*#include[^\n]*\n', text, re.M))
        pos = includes[-1].end() if includes else (text.index('\n', text.index('#pragma once')) + 1 if '#pragma once' in text else 0)
        text = text[:pos] + f'\nnamespace {name} {{\n' + text[pos:] + f'\n}} // namespace {name}\n'
        text = text.replace('BinaryData::', original + '_data::')
        text = text.replace('ProjectInfo::projectName', '"Shaper-' + name + '"')
        text = text.replace('createPluginFilter', 'createEmbeddedProcessor')
        if path.name == 'PluginProcessor.cpp':
            cleanup = '\n    for (auto* p : getParameters()) p->removeListener(this);\n'
            if name in ('satcurve','pitchcurve'): cleanup='\n    rackLifetime.reset();\n'+cleanup
            cleanup += '    delete sequencer;\n    for (auto* p : patterns) delete p;\n    for (auto* p : paintPatterns) delete p;\n'
            cleanup += {'gate12': '    delete value; delete value2;\n',
                        'pan12': '    delete value; delete value2;\n',
                        'volumef': '    delete value; delete value2;\n',
                        'satcurve': '    delete value; delete value2;\n',
                        'pitchcurve': '    delete value; delete value2;\n',
                        'time12': '    delete value;\n',
                        'filtr': '    for (auto* p : respatterns) delete p;\n    delete value; delete value2; delete resvalue; delete resvalue2;\n',
                        'reevr': '    for (auto* p : sendpatterns) delete p;\n    delete revvalue; delete sendvalue;\n'}[name]
            text = re.sub(r'(~\w+AudioProcessor\(\)\s*\{)', lambda m: m[1] + cleanup, text)
        # Embedded editors must not change the entire desktop application's scale.
        text = re.sub(r'Desktop::getInstance\(\).setGlobalScaleFactor\([^;]+;', '', text)
        # Each embedded processor needs its own resource header.
        text = '// Fazıl Irak tarafından değiştirildi: 23.10.2026; Shaper uyarlaması.\n// Özgün telif ve lisans bildirimleri aşağıda korunur.\n#include <JuceHeader.h>\n#include <' + original + '_data.h>\n' + text
        target = out / name / path.relative_to(src)
        target.parent.mkdir(parents=True, exist_ok=True)
        if not target.exists() or target.read_text(encoding='utf-8') != text:
            target.write_text(text, encoding='utf-8')
    about_factory = '// Fazıl Irak tarafından oluşturuldu / değiştirildi: 23.10.2026\n#include "ui/About.h"\nnamespace ' + name + ' { juce::Component* createAboutPage() { return new About(); } }\n'
    target=out/name/'RackAbout.cpp'
    if not target.exists() or target.read_text(encoding='utf-8')!=about_factory: target.write_text(about_factory,encoding='utf-8')
    if name in ('satcurve','pitchcurve'):
        bridge='''// Fazıl Irak tarafından oluşturuldu / değiştirildi: 23.10.2026
#include "PluginProcessor.h"
namespace NAMESPACE {
void forceCV(juce::AudioProcessor& processor) { auto& p=dynamic_cast<GATE12AudioProcessor&>(processor); p.outputCV=true; }
void formantView(juce::AudioProcessor& processor,bool formant) { dynamic_cast<GATE12AudioProcessor&>(processor).rackFormantView=formant; }
bool audioSettingsShown(juce::AudioProcessor& processor) { return dynamic_cast<GATE12AudioProcessor&>(processor).showAudioKnobs; }
void shape(juce::AudioProcessor& processor,int kind,float neutral) {
    auto& p=dynamic_cast<GATE12AudioProcessor&>(processor);
    for(auto* pattern:p.patterns) {
        pattern->clear();
        auto add=[&](double x,double v,int type=1) { pattern->insertPoint(x,1.0-v,0,type); };
        if(kind==0) { add(0,neutral); add(1,neutral); }
        else if(kind==1) { add(0,0); add(1,1); }
        else if(kind==2) { add(0,1); add(1,0); }
        else if(kind==3) { add(0,.5); add(.25,.75); add(.5,.5); add(.75,.25); add(1,.5); }
        else if(kind==4) { add(0,0,0); add(.25,1,0); add(.5,0,0); add(.75,1,0); add(1,0,0); }
        else if(kind==5) { add(0,0,0); add(.25,.25,0); add(.5,.5,0); add(.75,.75,0); add(1,1,0); }
        else if(kind==6) { add(0,1); add(.15,0); add(1,0); }
        else if(kind==7) { add(0,neutral); add(.5,1); add(1,neutral); }
        else { add(0,neutral); add(.5,0); add(1,neutral); }
        pattern->buildSegments(); pattern->incrementVersion();
    }
    p.pattern=p.patterns[0]; p.viewPattern=p.pattern; p.uimode=UIMode::Normal;
    auto* param=p.params.getParameter("pattern"); param->setValueNotifyingHost(param->convertTo0to1(1));
    p.sendChangeMessage();
}
}
'''.replace('NAMESPACE',name)
        target=out/name/'RackEnvelope.cpp'
        if not target.exists() or target.read_text(encoding='utf-8')!=bridge: target.write_text(bridge,encoding='utf-8')
(root / 'upstream-lock.json').write_text(json.dumps(manifest, indent=2) + '\n')
