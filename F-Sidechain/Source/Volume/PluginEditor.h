// Fazıl Irak tarafından değiştirildi: 23.10.2026; Shaper uyarlaması.
// Özgün telif ve lisans bildirimleri aşağıda korunur.
#include <JuceHeader.h>
#include <gate12_data.h>
/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ui/Rotary.h"
#include "ui/TextDial.h"
#include "ui/GridSelector.h"
#include "ui/CustomLookAndFeel.h"
#include "ui/About.h"
#include "ui/View.h"
#include "ui/SettingsButton.h"
#include <SidechainPresets.h>
#include "ui/AudioDisplay.h"
#include "ui/PaintToolWidget.h"
#include "ui/SequencerWidget.h"
#include "ui/BandsWidget.h"

namespace volumef {

using namespace globals;

class VOLUMEFAudioProcessorEditor : public juce::AudioProcessorEditor, public RackEditorCommands, private juce::AudioProcessorValueTreeState::Listener, public juce::ChangeListener
{
public:
    VOLUMEFAudioProcessorEditor (VOLUMEFAudioProcessor&);
    ~VOLUMEFAudioProcessorEditor() override;

    //==============================================================================
    void parameterChanged (const juce::String& parameterID, float newValue) override;
    void toggleUIComponents ();
    void paint (juce::Graphics&) override;
    void resized() override;
    void changeListenerCallback(ChangeBroadcaster* source) override;
    void drawGear(Graphics&g, Rectangle<int> bounds, float radius, int segs, Colour color, Colour bg);
    void drawUndoButton(Graphics& g, juce::Rectangle<float> area, bool invertx, Colour color);

    VOLUMEFAudioProcessor& audioProcessor;

    juce::PopupMenu rackOptions() override { return settingsButton->makeOptionsMenu(); }
    void rackCommand(int command) override { settingsButton->executeCommand(command); }
    bool rackCanEdit() const override { return audioProcessor.uimode != UIMode::Seq; }
    bool rackIsSequencing() const override { return audioProcessor.uimode == UIMode::Seq; }
    int rackDockLeft=0;
    int rackHeaderHeight() const override { return 158; }
    void rackApplyDock() {
        if (!init || rackDockLeft == 0) return;
        const int x=rackDockLeft, right=getWidth()-12;
        const int toolbar=119;
        auto place=[](juce::Component& c,int px,int py,int w,int h,const char* id=nullptr) {
            c.setTransform({}); c.setBounds(px,py,w,h); if(id) c.setComponentID(id);
        };
        logoLabel.setVisible(false);
        place(syncMenu,x,8,78,26,"effect-tempo");
        triggerLabel.setVisible(false); patSyncLabel.setVisible(false);
        place(triggerMenu,x+86,8,72,26,"effect-trigger");
        place(patSyncMenu,x+166,8,70,26,"effect-pattern-sync");
        syncMenu.setTooltip("Envelope length / host tempo sync");
        patSyncMenu.setTooltip("Pattern sync quantisation");
        for(auto& b:patterns) b->setVisible(false);
        for(auto* c : {rate.get(),rateSync.get(),phase.get(),smooth.get(),attack.get(),release.get(),
                      tension.get(),tensionatk.get(),tensionrel.get(),min.get(),max.get(),threshold.get(),
                      sense.get(),lowcut.get(),highcut.get(),offset.get()}) if(c) c->setVisible(false);
        algoMenu.setVisible(false); audioSettingsButton.setVisible(false);
        audioDisplay->setVisible(false); useSidechain.setVisible(false); useMonitor.setVisible(false);
        mixDial->setVisible(false); stereoSlider.setVisible(false); bandsWidget->setVisible(false); bandsBtn.setVisible(false);
        settingsButton->setVisible(false);
        for(size_t i=0;i<duckButtons.size();++i) {
            duckButtons[i]->setVisible(audioProcessor.uimode!=UIMode::Seq);
            place(*duckButtons[i],x+(int)i*48,80,43,27);
        }
        place(paintButton,12,toolbar,48,26,"effect-paint"); place(sequencerButton,66,toolbar,44,26,"effect-seq");
        place(pointLabel,116,toolbar+2,22,22); place(pointMenu,142,toolbar,84,26,"effect-curve");
        place(loopButton,232,toolbar,24,26);
        place(snapButton,right-50,toolbar,50,26,"snap-outline");
        place(*gridSelector,right-110,toolbar,54,26,"effect-grid");
        place(nudgeRightButton,right-138,toolbar,22,26); place(nudgeLeftButton,right-164,toolbar,22,26);
        place(redoButton,right-190,toolbar,22,26); place(undoButton,right-216,toolbar,22,26);
        place(*paintWidget,12,toolbar+35,getWidth()-24,40);
        int bottom=toolbar+35;
        if(paintWidget->isVisible()) bottom=paintWidget->getBottom()+6;
        place(*seqWidget,12,bottom,getWidth()-24,60);
        if(seqWidget->isVisible()) bottom=seqWidget->getBottom()+6;
        place(*view,0,bottom,getWidth(),juce::jmax(100,getHeight()-bottom),"effect-envelope-view");
        latencyWarning.setVisible(false);
        repaint();
    }
    void rackDockHeader(int left) override { rackDockLeft=left; rackApplyDock(); }

private:
    bool init = false;
    CustomLookAndFeel* customLookAndFeel = nullptr;
    std::unique_ptr<About> about;

    std::vector<std::unique_ptr<TextButton>> patterns;

#if defined(DEBUG)
    juce::TextButton presetExport;
#endif

    void layoutSidechainButtons();
    std::array<std::unique_ptr<SidechainPresetButton>,SidechainPresets::count> duckButtons;
    Label logoLabel;
    ComboBox syncMenu;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> syncAttachment;
    Label patSyncLabel;
    ComboBox patSyncMenu;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> patSyncAttachment;
    Slider stereoSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> stereoSliderAttachment;
    TextButton bandsBtn;
    std::unique_ptr<BandsWidget> bandsWidget;
    std::unique_ptr<SettingsButton> settingsButton;
    std::unique_ptr<TextDial> mixDial;

    std::unique_ptr<Rotary> rate;
    std::unique_ptr<Rotary> rateSync;
    std::unique_ptr<Rotary> phase;
    std::unique_ptr<Rotary> min;
    std::unique_ptr<Rotary> max;
    std::unique_ptr<Rotary> smooth;
    std::unique_ptr<Rotary> attack;
    std::unique_ptr<Rotary> release;
    std::unique_ptr<Rotary> tension;
    std::unique_ptr<Rotary> tensionatk;
    std::unique_ptr<Rotary> tensionrel;
    std::unique_ptr<Rotary> threshold;
    std::unique_ptr<Rotary> sense;
    std::unique_ptr<Rotary> lowcut;
    std::unique_ptr<Rotary> highcut;
    std::unique_ptr<Rotary> offset;

    ComboBox algoMenu;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> algoAttachment;
    TextButton useSidechain;
    TextButton useMonitor;
    TextButton nudgeRightButton;
    Label nudgeLabel;
    TextButton nudgeLeftButton;
    TextButton undoButton;
    TextButton redoButton;
    std::unique_ptr<AudioDisplay> audioDisplay;
    TextButton paintButton;
    TextButton sequencerButton;
    ComboBox pointMenu;
    Label pointLabel;
    TextButton loopButton;
    Label triggerLabel;
    ComboBox triggerMenu;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> triggerAttachment;
    TextButton audioSettingsButton;
    TextButton snapButton;
    std::unique_ptr<GridSelector> gridSelector;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> snapAttachment;
    std::unique_ptr<View> view;
    Label latencyWarning;
    std::unique_ptr<PaintToolWidget> paintWidget;
    std::unique_ptr<SequencerWidget> seqWidget;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VOLUMEFAudioProcessorEditor)
};

} // namespace volumef
