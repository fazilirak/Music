// Geliştirici: Fazıl Irak - Son düzenleme: 27.10.2026
// Dosyanın görevi: Ses ve durum davranışını doğrulayan testler.
// Copyright (c) 2026 Fazıl Irak - Özgün F-Kick uygulama kodu.
// SPDX-License-Identifier: AGPL-3.0-or-later
// Fazıl Irak · 27.10.2026 · DSP, MIDI zamanlaması, gömülü sample, preset ve gerçek GUI doğrulaması.
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <iostream>
#include <stdexcept>
void runTechnoTests(const juce::File&);
void runBankTests(const juce::File&);
void runImpactTests(const juce::File&);
void runFXRackTests(const juce::File&);

namespace {
void require(bool b,const char* why){if(!b)throw std::runtime_error(why);}
void set(FKickProcessor& p,const char* id,float v){p.setParameter(id,v);}
void prepare(FKickProcessor& p,double sr=48000,int block=256){p.setRateAndBufferSizeDetails(sr,block);p.prepareToPlay(sr,block);}
juce::AudioBuffer<float> render(FKickProcessor& p,int length,int block=256,int noteOff=-1,int note=60) {
    juce::AudioBuffer<float> output(2,length);output.clear();
    for(int offset=0;offset<length;offset+=block){const int n=juce::jmin(block,length-offset);juce::AudioBuffer<float> audio(2,n);juce::MidiBuffer midi;
        if(offset==0)midi.addEvent(juce::MidiMessage::noteOn(1,note,(juce::uint8)110),0);
        if(noteOff>=offset&&noteOff<offset+n)midi.addEvent(juce::MidiMessage::noteOff(1,note),noteOff-offset);
        p.processBlock(audio,midi);for(int c=0;c<2;++c)output.copyFrom(c,offset,audio,c,0,n);}
    return output;
}
float diff(const juce::AudioBuffer<float>& a,const juce::AudioBuffer<float>& b){float d=0;for(int c=0;c<2;++c)for(int i=0;i<a.getNumSamples();++i)d=juce::jmax(d,std::abs(a.getSample(c,i)-b.getSample(c,i)));return d;}
void finite(const juce::AudioBuffer<float>& a){for(int c=0;c<a.getNumChannels();++c)for(int i=0;i<a.getNumSamples();++i)require(std::isfinite(a.getSample(c,i))&&std::abs(a.getSample(c,i))<=1.00001f,"Nonfinite or over-range audio");}
void snapshot(juce::Component& c,const juce::File& file){auto image=c.createComponentSnapshot(c.getLocalBounds());auto stream=file.createOutputStream();require(stream!=nullptr,"Snapshot stream");stream->setPosition(0);stream->truncate();require(juce::PNGImageFormat{}.writeImageToStream(image,*stream),"Snapshot PNG");}
juce::Component* find(juce::Component& c,const juce::String& id){if(c.getComponentID()==id)return &c;for(auto* child:c.getChildren())if(auto* match=find(*child,id))return match;return nullptr;}
template<class T> bool containsType(juce::Component& c){if(dynamic_cast<T*>(&c))return true;for(auto* child:c.getChildren())if(containsType<T>(*child))return true;return false;}
bool pumpUntil(const std::function<bool()>& ready,int timeout=3000){const auto end=juce::Time::getMillisecondCounter()+timeout;while(!ready()&&juce::Time::getMillisecondCounter()<end)juce::MessageManager::getInstance()->runDispatchLoopUntil(10);return ready();}
}
int main(int argc,char** argv) {
    std::cout.setf(std::ios::unitbuf);std::cerr.setf(std::ios::unitbuf);
    std::cout<<"Starting F-Kick verification\n";
    juce::ScopedJuceInitialiser_GUI gui;
    try {
        auto folder=argc>1?juce::File(argv[1]):juce::File::getCurrentWorkingDirectory();folder.createDirectory();
        // Örnek konumu: MIDI offset öncesinde tam sessizlik.
        FKickProcessor timing;prepare(timing);juce::AudioBuffer<float> block(2,256);juce::MidiBuffer midi;
        midi.addEvent(juce::MidiMessage::noteOn(1,60,(juce::uint8)127),100);timing.processBlock(block,midi);
        require(block.getMagnitude(0,0,100)==0,"MIDI sample offset was ignored");require(block.getMagnitude(0,110,140)>.01f,"Kick did not start");finite(block);
        std::cout<<"PASS MIDI sample-accurate onset\n";
        // Düzensiz bloklar, aynı olaylar ve toplam örnek sayısı aynı sesi üretmeli.
        FKickProcessor a,b;prepare(a);prepare(b);auto aa=render(a,48000,127),bb=render(b,48000,512);
        require(diff(aa,bb)<2e-6f,"Block-size dependent output");require(aa.getMagnitude(0,0,24000)>.05f,"Silent default patch");require(aa.getMagnitude(0,40000,8000)<1e-5f,"Kick/filter/DC tail did not stop");
        std::cout<<"PASS block invariance and tail\n";
        FKickProcessor endpoint;auto flat=endpoint.curve(0);for(int i=0;i<flat.count;++i)flat.y[(size_t)i]=1;endpoint.setCurve(0,flat);prepare(endpoint);auto flatAudio=render(endpoint,48000);
        require(std::abs(flatAudio.getSample(0,23999)-flatAudio.getSample(0,24000))<.002f,"Non-zero Amp endpoint clicks at kick end");
        // Bütün fabrika presetleri ve örnekleme hızları.
        for(double sr:{44100.,48000.,96000.,192000.})for(int preset=0;preset<8;++preset){FKickProcessor p;p.setCurrentProgram(preset);prepare(p,sr);auto audio=render(p,int(sr*.25),255,int(sr*.1));finite(audio);}
        std::cout<<"PASS 8 patches at 44.1 / 48 / 96 / 192 kHz\n";
        // Velocity gerçekten etki etmeli; monofonik retrigger taşma üretmemeli.
        FKickProcessor velocity;prepare(velocity);juce::AudioBuffer<float> quiet(2,4000),loud(2,4000);juce::MidiBuffer vm;
        vm.addEvent(juce::MidiMessage::noteOn(1,60,(juce::uint8)32),0);velocity.processBlock(quiet,vm);prepare(velocity);
        vm.addEvent(juce::MidiMessage::noteOn(1,60,(juce::uint8)127),0);velocity.processBlock(loud,vm);
        require(loud.getMagnitude(0,0,4000)>quiet.getMagnitude(0,0,4000)*3.5f,"Velocity response missing");
        std::cout<<"PASS velocity\n";
        FKickProcessor audition;prepare(audition);audition.keyboard.noteOn(1,60,.8f);juce::MidiBuffer auditionMidi;audition.processBlock(block,auditionMidi);require(block.getMagnitude(0,0,256)>.01f,"UI keyboard queue silent");audition.requestPanic();
        for(int i=0;i<100;++i)audition.processBlock(block,auditionMidi);require(audition.voiceCount.load()==0,"Panic did not stop a one-shot");
        std::cout<<"PASS UI event queue and one-shot panic\n";
        // Gate ve sustain pedal release.
        FKickProcessor gate;set(gate,"mode",1);set(gate,"release",20);set(gate,"sustain",1);prepare(gate);
        auto held=render(gate,24000,256);prepare(gate);auto released=render(gate,24000,256,4800);
        require(held.getMagnitude(0,18000,4000)>.05f,"Sampler held body vanished");require(released.getMagnitude(0,18000,4000)<1e-5f,"Note-off release stuck");
        FKickProcessor pedal;set(pedal,"mode",1);set(pedal,"release",10);set(pedal,"sustain",1);prepare(pedal);
        juce::AudioBuffer<float> pedalAudio(2,48000);juce::MidiBuffer pedalMidi;pedalMidi.addEvent(juce::MidiMessage::noteOn(1,60,(juce::uint8)100),0);pedalMidi.addEvent(juce::MidiMessage::controllerEvent(1,64,127),1000);pedalMidi.addEvent(juce::MidiMessage::noteOff(1,60),2000);pedal.processBlock(pedalAudio,pedalMidi);
        require(pedalAudio.getMagnitude(0,8000,2000)>.01f,"CC64 did not hold released note");pedalMidi.addEvent(juce::MidiMessage::controllerEvent(1,64,0),0);pedal.processBlock(pedalAudio,pedalMidi);
        require(pedal.voiceCount.load()==0,"CC64 pedal did not release voices");require(pedalAudio.getMagnitude(0,40000,8000)<1e-5f,"Pedal release filter/DC tail did not settle");
        std::cout<<"PASS sampler sustain and note-off\n";
        // Yüklenmiş sample dış dosya taşınsa da DAW state'inden geri gelmeli.
        auto input=folder.getChildFile("test-input.wav");
        {juce::AudioBuffer<float> samples(2,6000);for(int i=0;i<6000;++i){samples.setSample(0,i,float(.3*std::sin(i*.043)));samples.setSample(1,i,float(.2*std::cos(i*.031)));}
            juce::WavAudioFormat format;auto stream=input.createOutputStream();stream->setPosition(0);stream->truncate();std::unique_ptr<juce::AudioFormatWriter> w(format.createWriterFor(stream.get(),48000,2,24,{},0));require(w!=nullptr,"Test WAV writer");stream.release();require(w->writeFromAudioSampleBuffer(samples,0,6000),"Test WAV");}
        FKickProcessor sample;require(sample.loadSample(input),"Sample import failed");set(sample,"bodyGain",-60);set(sample,"mode",1);set(sample,"loop",1);set(sample,"reverse",1);set(sample,"start",.1f);set(sample,"end",.85f);set(sample,"sampleTune",-5);prepare(sample);
        auto before=render(sample,32000,127,12000);require(before.getMagnitude(0,0,10000)>.01f,"Loaded sample is silent");finite(before);
        juce::MemoryBlock state;sample.getStateInformation(state);input.deleteFile();
        FKickProcessor restored;restored.setStateInformation(state.getData(),(int)state.getSize());prepare(restored);auto after=render(restored,32000,512,12000);
        require(restored.sampleView().loaded,"Embedded sample not restored");require(diff(before,after)<2e-6f,"Sample state round-trip changed sound");
        auto preset=folder.getChildFile("roundtrip.fkick");require(sample.savePreset(preset),"Preset save failed");FKickProcessor disk;require(disk.loadPreset(preset),"Preset load failed");require(std::abs(disk.value("sampleTune")+5)<.001f,"Preset parameter missing");
        auto bad=folder.getChildFile("invalid.fkick");bad.replaceWithText("not a preset");require(!disk.loadPreset(bad),"Invalid preset accepted");require(disk.sampleView().loaded,"Invalid preset destroyed existing state");bad.deleteFile();
        std::cout<<"PASS sample loop/reverse, portable state, preset validation\n";
        // En fazla 16 ses, CC120/123 ve boş blok.
        FKickProcessor poly;set(poly,"poly",1);set(poly,"mode",1);prepare(poly);juce::MidiBuffer pm;
        for(int note=36;note<72;++note)pm.addEvent(juce::MidiMessage::noteOn(1,note,(juce::uint8)100),0);
        poly.processBlock(block,pm);require(poly.voiceCount.load()==16,"Voice limit broken");finite(block);
        pm.addEvent(juce::MidiMessage::allSoundOff(1),0);poly.processBlock(block,pm);juce::AudioBuffer<float> empty(2,0);poly.processBlock(empty,pm);
        auto silent=render(poly,48000,256,0);finite(silent);
        std::cout<<"PASS voice stealing / panic / empty blocks\n";
        // Curve noktaları kaydedilmeli; uç zamanları sabit ve eşleme sonlu kalmalı.
        auto curve=sample.curve(1);curve.y[2]=.38f;curve.bend[1]=-.6f;sample.setCurve(1,curve);sample.getStateInformation(state);
        disk.setStateInformation(state.getData(),(int)state.getSize());require(std::abs(disk.curve(1).y[2]-.38f)<1e-5f,"Curve state missing");require(std::abs(disk.curve(1).bend[1]+.6f)<1e-5f,"Curve shape missing");
        // Worker/RT sahiplik devri: silme yalnız yükleyici/host tarafında.
        prepare(disk);for(int i=0;i<45;++i){disk.clearSample();disk.processBlock(block,pm);disk.setStateInformation(state.getData(),(int)state.getSize());disk.processBlock(block,pm);finite(block);}
        std::cout<<"PASS curve state and sample ownership stress\n";
        FKickProcessor history;history.state.copyState();history.undo.clearUndoHistory();history.undo.beginNewTransaction("Edit root");set(history,"root",83);history.state.copyState();
        require(history.undo.canUndo(),"Parameter edit missing from Undo");history.undo.undo();require(std::abs(history.value("root")-50)<.001f,"Undo did not restore parameter");history.undo.redo();require(std::abs(history.value("root")-83)<.001f,"Redo did not restore parameter");
        FKickProcessor mono,stereo;auto bus=mono.getBusesLayout();bus.outputBuses.set(0,juce::AudioChannelSet::mono());require(mono.setBusesLayout(bus),"Mono layout rejected");prepare(mono);prepare(stereo);
        juce::AudioBuffer<float> monoaudio(1,2048),stereoaudio(2,2048);juce::MidiBuffer monomidi,stereomidi;monomidi.addEvent(juce::MidiMessage::noteOn(1,60,(juce::uint8)100),0);stereomidi.addEvent(juce::MidiMessage::noteOn(1,60,(juce::uint8)100),0);mono.processBlock(monoaudio,monomidi);stereo.processBlock(stereoaudio,stereomidi);
        for(int i=0;i<2048;++i)require(std::abs(monoaudio.getSample(0,i)-.5f*(stereoaudio.getSample(0,i)+stereoaudio.getSample(1,i)))<1e-6f,"Mono fold differs from stereo");
        std::cout<<"PASS Undo / Redo and mono output\n";
        FKickProcessor demo;demo.setCurrentProgram(1);prepare(demo);require(demo.exportWav(folder.getChildFile("F-Kick-Deep-House.wav")),"WAV export failed");
        juce::WavAudioFormat format;std::unique_ptr<juce::AudioFormatReader> reader(format.createReaderFor(folder.getChildFile("F-Kick-Deep-House.wav").createInputStream().release(),true));require(reader&&reader->sampleRate==48000&&reader->numChannels==2&&reader->bitsPerSample==24,"Export format wrong");
        std::cout<<"PASS stereo 48 kHz / 24-bit export\n";
        std::unique_ptr<juce::AudioProcessorEditor> editor(demo.createEditor());require(editor->getWidth()==760&&editor->getHeight()==360,"Compact UI dimensions");
        for(auto* child:editor->getChildren())require(editor->getLocalBounds().contains(child->getBounds()),"UI component outside editor");
        require(find(*editor,"brand-logo")&&find(*editor,"envelope-editor")&&find(*editor,"preset-selector"),"Required UI components missing");
        require(!containsType<juce::MidiKeyboardComponent>(*editor)&&!containsType<juce::Slider>(*editor)&&find(*editor,"hp")!=nullptr,"HP must be an editable text control");
        auto* brand=dynamic_cast<FKickLogo*>(find(*editor,"brand-logo"));require(brand&&dynamic_cast<juce::Button*>(find(*editor,"brand-logo"))==nullptr,"Logo must be a plain component");
        auto* previous=dynamic_cast<juce::TextButton*>(find(*editor,"preset-previous"));auto* next=dynamic_cast<juce::TextButton*>(find(*editor,"preset-next"));auto* selector=dynamic_cast<juce::ComboBox*>(find(*editor,"preset-selector"));
        require(previous&&next&&selector&&previous->getRight()<=selector->getX()&&next->getX()>=selector->getRight(),"Preset arrows do not flank selector");
        next->onClick();require(demo.getCurrentProgram()==2&&std::abs(demo.value("root")-56)<.001f&&selector->getSelectedId()==3,"Next arrow did not apply next preset");
        previous->onClick();require(demo.getCurrentProgram()==1&&std::abs(demo.value("root")-48)<.001f,"Previous arrow did not apply previous preset");
        previous->onClick();previous->onClick();require(demo.getCurrentProgram()==demo.getNumPrograms()-1,"Previous preset must wrap to last");next->onClick();require(demo.getCurrentProgram()==0,"Next preset must wrap to first");selector->setSelectedId(2,juce::sendNotificationSync);
        auto* cutoffText=dynamic_cast<juce::Label*>(find(*editor,"cutoff-value"));require(cutoffText,"Editable cutoff text missing");cutoffText->showEditor();require(cutoffText->getCurrentTextEditor()!=nullptr,"Numeric text editor did not open");cutoffText->getCurrentTextEditor()->setText("2.5k");cutoffText->hideEditor(false);require(std::abs(demo.value("cutoff")-2500)<.02f,"Numeric text did not update parameter");
        auto* startText=dynamic_cast<juce::Label*>(find(*editor,"start-value"));auto* endText=dynamic_cast<juce::Label*>(find(*editor,"end-value"));require(startText&&endText,"Region text fields missing");startText->setText("25%",juce::sendNotificationSync);endText->setText("10%",juce::sendNotificationSync);require(demo.value("end")>demo.value("start"),"Typed trim bounds crossed");
        const juce::Rectangle<int> sampleStrip(76,74,570,27);for(const char* id:{"sampleGain","sampleTune","samplePan","start","end"}){auto* field=find(*editor,id);require(sampleStrip.contains(editor->getLocalArea(field,field->getLocalBounds())),"Sample parameter outside sample panel");}
        selector->setSelectedId(1,juce::sendNotificationSync);selector->setSelectedId(2,juce::sendNotificationSync);
        auto* license=dynamic_cast<juce::TextEditor*>(find(*editor,"license-text"));require(license&&license->getText().contains("F-Kick"),"Embedded licence text missing");
        snapshot(*editor,folder.getChildFile("F-Kick-main.png"));auto* about=find(*editor,"about-panel");require(about!=nullptr,"Offline About missing");brand->onClick();require(about->isVisible(),"Clicking logo did not open About");snapshot(*editor,folder.getChildFile("F-Kick-about.png"));about->setVisible(false);
        require(demo.loadSample(folder.getChildFile("F-Kick-Deep-House.wav")),"Load exported audio");auto* graph=dynamic_cast<FKickGraph*>(find(*editor,"envelope-editor"));require(graph!=nullptr,"Graph type");auto* sampleTab=dynamic_cast<juce::TextButton*>(find(*editor,"tab-sample"));sampleTab->onClick();snapshot(*editor,folder.getChildFile("F-Kick-sample.png"));
        const auto full=graph->createComponentSnapshot(graph->getLocalBounds());set(demo,"start",.25f);set(demo,"end",.75f);const auto shaded=graph->createComponentSnapshot(graph->getLocalBounds());
        const auto plot=graph->plotBounds();const int cy=juce::roundToInt(plot.getCentreY());
        for(float fraction:{.1f,.9f}){const int x=juce::roundToInt(plot.getX()+plot.getWidth()*fraction);const float normal=full.getPixelAt(x,cy).getBrightness(),dim=shaded.getPixelAt(x,cy).getBrightness();require(dim<normal*.8f&&dim>normal*.3f,"Trim exclusion must be dimmed but still visible");}
        const int cx=juce::roundToInt(plot.getCentreX());require(full.getPixelAt(cx,cy)==shaded.getPixelAt(cx,cy),"Selected sample region was dimmed");snapshot(*editor,folder.getChildFile("F-Kick-trim.png"));
        set(demo,"start",0);set(demo,"end",1);
        auto* dropArea=dynamic_cast<FKickSampleDropArea*>(find(*editor,"sample-drop-area"));require(dropArea!=nullptr,"Explicit sample drop target missing");
        const auto droppedFile=folder.getChildFile(juce::String::fromUTF8("Explorer deneme - örnek kick.wav"));require(folder.getChildFile("F-Kick-Deep-House.wav").copyFileTo(droppedFile),"Drop fixture copy");const juce::StringArray dropFiles{droppedFile.getFullPathName()};
        require(dropArea->isInterestedInFileDrag(dropFiles)&&dropArea->isInterestedInFileDrag(juce::StringArray{"C:/Sample.WAV"}),"Audio drop format rejected");
        require(!dropArea->isInterestedInFileDrag(juce::StringArray{"C:/notes.txt"})&&!dropArea->isInterestedInFileDrag(juce::StringArray{droppedFile.getFullPathName(),droppedFile.getFullPathName()}),"Unsupported or multi-file audio drop accepted");
        dropArea->fileDragEnter(dropFiles,200,100);snapshot(*editor,folder.getChildFile("F-Kick-drop-target.png"));dropArea->fileDragExit(dropFiles);
        // ComponentPeer olay yolu: Windows native drop adapter'ının çağırdığı aynı hedef araması ve async teslim.
        editor->addToDesktop(0);editor->setVisible(true);auto* peer=editor->getPeer();require(peer!=nullptr,"Native test peer missing");
        juce::ComponentPeer::DragInfo drag;drag.files=dropFiles;drag.position=editor->getLocalPoint(graph,juce::Point<int>{graph->getWidth()/2,graph->getHeight()/2});
        require(peer->handleDragMove(drag),"Native file drag did not find sample-panel ancestor through waveform");peer->handleDragExit(drag);
        auto* gainField=find(*editor,"sampleGain-value");drag.position=editor->getLocalPoint(gainField,juce::Point<int>{8,8});require(peer->handleDragMove(drag),"Native file drag did not find panel through text controls");peer->handleDragExit(drag);
        demo.clearSample();drag.position=editor->getLocalPoint(graph,juce::Point<int>{graph->getWidth()/2,graph->getHeight()/2});require(peer->handleDragDrop(drag),"Native drop dispatch rejected audio");
        require(pumpUntil([&]{return demo.sampleView().loaded&&demo.sampleView().name==droppedFile.getFileName();}),"Native drop did not load sample asynchronously");
        require(graph->tab==0,"Dropped sample must open Sample view");const auto loadedName=demo.sampleView().name;
        dropArea->filesDropped(juce::StringArray{"C:/not-audio.txt"},100,100);require(demo.sampleView().name==loadedName,"Rejected drop changed loaded sample");
        auto* sampleName=dynamic_cast<juce::TextButton*>(find(*editor,"sample-load"));require(pumpUntil([&]{return sampleName->getButtonText().contains(droppedFile.getFileName());}),"Dropped filename/waveform UI did not refresh");
        #if JUCE_WINDOWS
        demo.clearSample();require(dynamic_cast<FKickEditor*>(editor.get())->testNativeDrop(dropFiles,editor->getLocalPoint(sampleName,juce::Point<int>{20,10})),"Windows OLE registration or CF_HDROP transfer failed");
        require(pumpUntil([&]{return demo.sampleView().loaded&&demo.sampleView().name==droppedFile.getFileName();}),"Windows OLE Drop failed to load audio");
        std::cout<<"PASS Windows OLE registration / IDataObject Unicode CF_HDROP / actual native Drop callback\n";
        #endif
        editor->setVisible(false);editor->removeFromDesktop();snapshot(*editor,folder.getChildFile("F-Kick-dropped-sample.png"));droppedFile.deleteFile();
        std::cout<<"PASS dimmed trim exclusions / native file-drop target routing / async sample load / Unicode paths / invalid drop\n";
        auto* samplerMode=dynamic_cast<juce::TextButton*>(find(*editor,"mode-sampler"));samplerMode->onClick();require(demo.value("mode")==1&&!find(*editor,"tab-amp")->isEnabled(),"Sampler mode UI did not switch");snapshot(*editor,folder.getChildFile("F-Kick-sampler.png"));
        std::cout<<"PASS compact GUI / plain logo / working preset arrows / typed values / offline About / screenshots\n";
        if(argc>2) {
            juce::VST3PluginFormat format;juce::OwnedArray<juce::PluginDescription> descriptions;
            format.findAllTypesForFile(descriptions,argv[2]);require(descriptions.size()==1,"VST3 bundle scan failed");
            require(descriptions[0]->name=="F-Kick"&&descriptions[0]->isInstrument,"VST3 instrument identity wrong");
            juce::String error;auto instance=format.createInstanceFromDescription(*descriptions[0],48000,256,error);
            require(instance!=nullptr,error.toRawUTF8());instance->prepareToPlay(48000,256);
            juce::AudioBuffer<float> hostAudio(2,256);juce::MidiBuffer hostMidi;hostMidi.addEvent(juce::MidiMessage::noteOn(1,60,(juce::uint8)110),64);instance->processBlock(hostAudio,hostMidi);
            require(hostAudio.getMagnitude(0,0,64)==0&&hostAudio.getMagnitude(0,80,176)>.01f,"VST3 host audio/MIDI failure");finite(hostAudio);
            juce::MemoryBlock hostState;instance->getStateInformation(hostState);require(hostState.getSize()>100,"VST3 host state empty");instance->setStateInformation(hostState.getData(),(int)hostState.getSize());
            // AudioPluginInstance host state'i VST3PluginState zarfı taşır; ham işlemci state'i değildir.
            juce::MemoryBlock droppedState;demo.getStateInformation(droppedState);juce::XmlElement vstWrapper("VST3PluginState");vstWrapper.createNewChildElement("IComponent")->addTextElement(droppedState.toBase64Encoding());
            juce::MemoryBlock wrappedState;juce::AudioProcessor::copyXmlToBinary(vstWrapper,wrappedState);instance->setStateInformation(wrappedState.getData(),(int)wrappedState.getSize());instance->getStateInformation(hostState);
            auto returnedWrapper=juce::AudioProcessor::getXmlFromBinary(hostState.getData(),(int)hostState.getSize());require(returnedWrapper&&returnedWrapper->getChildByName("IComponent"),"VST3 component state missing");
            juce::MemoryBlock componentState;require(componentState.fromBase64Encoding(returnedWrapper->getChildByName("IComponent")->getAllSubText()),"VST3 component state decode");
            auto droppedXml=juce::AudioProcessor::getXmlFromBinary(componentState.getData(),(int)componentState.getSize());require(droppedXml&&droppedXml->getChildByName("Sample")&&droppedXml->getChildByName("Sample")->getStringAttribute("name").contains(juce::String::fromUTF8("örnek")),"VST3 lost sample loaded by native drop");
            hostMidi.clear();hostMidi.addEvent(juce::MidiMessage::noteOn(1,60,(juce::uint8)110),0);instance->processBlock(hostAudio,hostMidi);require(hostAudio.getMagnitude(0,0,256)>.01f,"VST3 embedded dropped sample was silent");finite(hostAudio);
            std::unique_ptr<juce::AudioProcessorEditor> hostEditor(instance->createEditorIfNeeded());require(hostEditor!=nullptr&&hostEditor->getWidth()==760&&hostEditor->getHeight()==360,"VST3 host editor failed");hostEditor.reset();instance->releaseResources();instance.reset();
            std::cout<<"PASS built VST3: scan / instantiate / MIDI / audio / state / editor\n";
        }
        runTechnoTests(folder);
        runImpactTests(folder);
        runBankTests(folder);
        runFXRackTests(folder);
        std::cout<<"ALL TESTS PASSED\n";
        return 0;
    }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}

