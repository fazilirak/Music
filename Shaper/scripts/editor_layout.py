# SPDX-License-Identifier: GPL-3.0-only
# Copyright (c) 2026 Fazıl Irak; Shaper eklemeleri/değişiklikleri.
# Fazıl Irak tarafından oluşturuldu / değiştirildi: 23.10.2026
# Kapsam: Shaper için eklenen kod ve uyarlamalar; özgün üçüncü taraf telif/lisansları korunur.
# Türkçe bakım açıklamaları: Fazıl Irak
# Son düzenleme: 22.10.2026
# Görev: Gömülü efekt editörlerinin Shaper standart yerleşim/çizim kaynaklarını üretir.
"""Shared rack geometry; native controls keep their original parameter bindings."""

# editor_methods: Gömülü efekt editörlerinin Shaper standart yerleşim/çizim kaynaklarını üretir.
def editor_methods(name):
    common = '''
    juce::PopupMenu rackOptions() override { return settingsButton->makeOptionsMenu(); }
    void rackCommand(int command) override { settingsButton->executeCommand(command); }
    bool rackCanEdit() const override { return audioProcessor.uimode != UIMode::Seq; }
    bool rackIsSequencing() const override { return audioProcessor.uimode == UIMode::Seq; }
    int rackDockLeft=0;
    int rackHeaderHeight() const override { return 330; }
    void rackApplyDock() {
        if(rackDockLeft==0 || !init) return;
        const int x=rackDockLeft+12, right=getWidth()-12;
        auto place=[](juce::Component& c,int px,int py,int w,int h,const char* id=nullptr) {
            c.setTransform({}); c.setBounds(px,py,w,h); if(id) c.setComponentID(id);
        };
        logoLabel.setVisible(false); logoLabel.setComponentID("effect-hidden-title");
        place(syncMenu,x,10,88,28,"effect-tempo");
        place(triggerLabel,x+96,10,48,28); place(triggerMenu,x+148,10,72,28,"effect-trigger");
        place(patSyncLabel,x+230,10,66,28); place(patSyncMenu,x+300,10,72,28,"effect-pattern-sync");
        triggerLabel.setFont(juce::FontOptions(13)); patSyncLabel.setFont(juce::FontOptions(13));
        patSyncLabel.setText("Pat. Sync",juce::dontSendNotification);
        for(size_t i=0;i<patterns.size();++i) place(*patterns[i],x+(int)i*23,48,24,25);
        place(algoMenu,x+308,48,76,25); place(audioSettingsButton,x+390,48,25,25,"effect-audio-settings");
        place(paintButton,x,300,50,26,"effect-paint"); place(sequencerButton,x+56,300,50,26,"effect-seq");
        for(auto* button:{&paintButton,&sequencerButton}) {
            button->setColour(juce::TextButton::textColourOffId,juce::Colour(COLOR_ACTIVE));
            button->setColour(juce::TextButton::textColourOnId,juce::Colour(COLOR_ACTIVE));
        }
        place(pointLabel,x+112,300,22,26); place(pointMenu,x+140,300,74,26,"effect-curve");
        place(loopButton,x+220,300,24,26);
        place(snapButton,right-54,300,54,26,"snap-outline"); place(*gridSelector,right-111,300,50,26,"effect-grid");
        place(nudgeRightButton,right-136,300,20,26); place(nudgeLeftButton,right-160,300,20,26);
        place(redoButton,right-186,300,20,26); place(undoButton,right-210,300,20,26);
        settingsButton->setVisible(false);
        // Native paint/sequence modes expand below the shared header.
        place(*paintWidget,15,342,getWidth()-30,40);
        int bottom=336;
        if(paintWidget->isVisible()) bottom=paintWidget->getBottom()+10;
        place(*seqWidget,15,bottom,getWidth()-30,60);
        if(seqWidget->isVisible()) bottom=seqWidget->getBottom()+10;
        place(*view,0,bottom,getWidth(),juce::jmax(100,getHeight()-bottom),"effect-envelope-view");
        place(latencyWarning,getWidth()/2-150,getHeight()-32,300,25);
'''
    # Common envelope slots stay fixed; hidden alternatives share the same slot.
    if name != 'time12':
        common += '''
        if(rate) place(*rate,x,94,68,64,"effect-rate");
        place(*smooth,x+216,94,68,64,"effect-smooth");
        place(*attack,x+216,94,68,64,"effect-attack"); place(*release,x+288,94,68,64,"effect-release");
        place(*tension,x+360,94,68,64,"effect-tension");
        place(*tensionatk,x+360,94,68,64); place(*tensionrel,x+432,94,68,64);
'''
    if name in ('gate12','pan12','volumef'):
        common += '''
        place(*rateSync,x,94,68,64); place(*phase,x+72,94,68,64,"effect-phase");
        place(*min,x,190,68,64); place(*max,x+76,190,68,64);
        place(*threshold,x,190,68,64); place(*sense,x+72,190,68,64);
        place(*lowcut,x+144,190,68,64); place(*highcut,x+216,190,68,64); place(*offset,x+288,190,68,64);
        place(*audioDisplay,x+364,190,juce::jmax(80,right-x-460),62);
        place(useSidechain,right-88,190,88,25); place(useMonitor,right-88,223,88,25);
        mixDial->setVisible(false); stereoSlider.setVisible(false); bandsWidget->setVisible(false); bandsBtn.setVisible(false);
'''
    elif name == 'time12':
        common += '''
        place(*rateDial,x+430,48,65,25);
        const int columns[]={72,216,216,288,360,360,432};
        const char* commonIds[]={"effect-phase","effect-smooth","effect-attack","effect-release","effect-tension","effect-tension-atk","effect-tension-rel"};
        for(size_t i=0;i<rackModulation.size();++i) if(rackModulation[i]) {
            place(*rackModulation[i],x+columns[i],94,68,64,commonIds[i]);
            bool visible=i==0 || (i==1?!audioProcessor.dualSmooth:i==2||i==3?audioProcessor.dualSmooth:i==4?!audioProcessor.dualTension:audioProcessor.dualTension);
            rackModulation[i]->setVisible(visible);
        }
        place(*threshold,x,190,68,64); place(*sense,x+72,190,68,64);
        place(*lowcut,x+144,190,68,64); place(*highcut,x+216,190,68,64); place(*offset,x+288,190,68,64);
        place(*audioDisplay,x+364,190,juce::jmax(80,right-x-460),62);
        place(useSidechain,right-88,190,88,25); place(useMonitor,right-88,223,88,25);
        mixDial->setVisible(false);
'''
    elif name == 'filtr':
        common += '''
        place(*cutoff,x,190,68,64); place(*res,x+72,190,68,64);
        place(*cutoffset,x+144,190,68,64); place(*resoffset,x+144,190,68,64);
        place(*drive,x+216,190,68,64); place(*morph,x+216,190,68,64); place(*slope,x+216,190,68,64);
        place(filterTypeMenu,x+300,194,105,26); place(filterModeMenu,x+412,194,74,26);
        place(cutEnvOnButton,x+300,232,22,26); place(resEnvOnButton,x+300,232,22,26);
        place(cutEnvButton,x+330,232,90,26); place(resEnvButton,x+330,232,90,26);
        place(linkPatsButton,x+280,48,22,25);
        for(size_t i=0;i<respatterns.size();++i) place(*respatterns[i],x+(int)i*23,48,24,25);
        place(*audioWidget,x,184,right-x,92);
        for(auto* widget:{cutenv.get(),resenv.get()}) { widget->setBounds(0,0,(int)((right-x)/.72f),150); widget->setTransform(juce::AffineTransform::scale(.72f).translated((float)x,184)); }
        mixDial->setVisible(false); stereoSlider.setVisible(false); meter->setVisible(false); bandsWidget->setVisible(false); bandsBtn.setVisible(false);
'''
    else:
        common += '''
        place(*rateDial,x+430,48,65,25);
        place(*irDisplay,x,190,160,66);
        for(auto* widget:{eqWidget.get(),decayWidget.get()}) {
            widget->setBounds(0,0,355,140);
            widget->setTransform(juce::AffineTransform::scale(.8f).translated((float)x,182));
        }
        place(eqButton,x,263,48,24,"effect-eq"); place(decayButton,x+54,263,54,24,"effect-decay"); place(trueStereoButton,x+114,263,44,24);
        for(auto* button:{&eqButton,&decayButton}) { button->setColour(juce::TextButton::textColourOffId,juce::Colour(COLOR_ACTIVE)); button->setColour(juce::TextButton::textColourOnId,juce::Colour(COLOR_ACTIVE)); }
        place(currentFile,x+308,263,150,24); place(fileInfo,x+464,263,140,24);
        place(irgainSlider,x+(audioProcessor.eqtab==1?286:162),190,16,66);
        place(*lowcut,x+184,190,58,64); place(*highcut,x+246,190,58,64);
        place(lowcutSlope,x+180,180,40,16); place(highcutSlope,x+242,180,40,16);
        place(*stretch,x+308,190,58,64); place(*predelay,x+370,190,58,64); place(*predelaysync,x+370,190,58,64);
        place(predelayUseSync,x+430,214,20,28);
        place(*reverb,x+454,190,58,64); place(*send,x+516,190,58,64);
        place(*revoffset,x+578,190,58,64); place(*sendoffset,x+578,190,58,64);
        place(revEnvOnButton,x+610,263,22,24); place(sendEnvOnButton,x+610,263,22,24);
        place(revEnvButton,x+638,263,90,24); place(sendEnvButton,x+638,263,90,24);
        place(*audioWidget,x,184,right-x,92);
        for(auto* widget:{revenv.get(),sendenv.get()}) { widget->setBounds(0,0,(int)((right-x)/.72f),150); widget->setTransform(juce::AffineTransform::scale(.72f).translated((float)x,184)); }
        place(*fileSelector,15,342,getWidth()-30,juce::jmax(150,getHeight()-342));
        drywet->setVisible(false); width->setVisible(false); toggleViewportButton.setVisible(false);
        bandsWidget->setVisible(false); bandsBtn.setVisible(false);
'''
    if name in ('time12','volumef'):
        buttons = 'quickButtons' if name == 'time12' else 'duckButtons'
        # Both banks are eight items. Place them in the dedicated effect area.
        common += f'''
        for(size_t i=0;i<{buttons}.size();++i) if({buttons}[i]) place(*{buttons}[i],x+(int)i*62,{'202' if name=='time12' else '262'},56,28);
'''
    return common + '''
        repaint();
    }
    void rackDockHeader(int left) override { rackDockLeft=left; rackApplyDock(); }
'''


# paint_sections: Gömülü efekt editörlerinin Shaper standart yerleşim/çizim kaynaklarını üretir.
def paint_sections():
    return '''
    if(rackDockLeft>0) {
        const int x=rackDockLeft;
        auto section=[&](int y,int h,const char* label) {
            g.setColour(juce::Colour(0xff1c2027)); g.fillRoundedRectangle((float)x,(float)y,(float)(getWidth()-x),(float)h,6);
            g.setColour(juce::Colour(0xff87909c)); g.setFont(juce::FontOptions(10.5f));
            g.drawText(label,x+12,y+3,180,14,juce::Justification::centredLeft);
        };
        section(80,82,"MODULATION"); section(168,126,"EFFECT SETTINGS");
    }
'''
