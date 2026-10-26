// Fazıl Irak tarafından değiştirildi: 23.10.2026; Shaper uyarlaması.
// Özgün telif ve lisans bildirimleri aşağıda korunur.
#include <JuceHeader.h>
#include <gate12_data.h>
#include "SettingsButton.h"
#include "../PluginProcessor.h"
#include "../Globals.h"

namespace volumef {

void SettingsButton::paint(Graphics&) {}
void SettingsButton::mouseDown(const juce::MouseEvent&) {}
PopupMenu SettingsButton::makeOptionsMenu() {
	PopupMenu midiTriggerChn;
	midiTriggerChn.addItem(2010, "Off", true, audioProcessor.midiTriggerChn == -1);
	for (int i = 0; i < 16; i++) {
		midiTriggerChn.addItem(2010 + i + 1, String(i + 1), true, audioProcessor.midiTriggerChn == i);
	}
	midiTriggerChn.addItem(2027, "Any", true, audioProcessor.midiTriggerChn == 16);

	PopupMenu triggerChn;
	triggerChn.addItem(10, "Off", true, audioProcessor.triggerChn == -1);
	for (int i = 0; i < 16; i++) {
		triggerChn.addItem(10 + i + 1, String(i + 1), true, audioProcessor.triggerChn == i);
	}
	triggerChn.addItem(27, "Any", true, audioProcessor.triggerChn == 16);

	PopupMenu audioTrigger;
	audioTrigger.addItem(32, "Ignore hits while playing", true, audioProcessor.audioIgnoreHitsWhilePlaying);

	PopupMenu CC;
	CC.addItem(300, "Off", true, audioProcessor.outputCC == 0);
	CC.addSeparator();
	for (int i = 1; i <= 128; ++i) {
		CC.addItem(300 + i,  String(i - 1), true, audioProcessor.outputCC == i);
	}

	PopupMenu CCChan;
	for (int i = 0; i < 16; ++i) {
		CCChan.addItem(450 + i, String(i+1), true, audioProcessor.outputCCChan == i);
	}

	auto midiNoteToName = [](int noteNumber) -> std::string {
		const std::string noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
		int octave = (noteNumber / 12) - 1;
		std::string noteName = noteNames[noteNumber % 12];
		return std::to_string(noteNumber) + " " + noteName + std::to_string(octave);
	};

	PopupMenu audioOutputMIDI;
	audioOutputMIDI.addItem(500, "Off", true, audioProcessor.outputATMIDI == 0);
	audioOutputMIDI.addSeparator();
	for (int i = 1; i < 129; ++i) {
		audioOutputMIDI.addItem(500+i, midiNoteToName(i-1), true, audioProcessor.outputATMIDI == i);
	}

	PopupMenu output;
	output.addItem(700, "CV", true, audioProcessor.outputCV);
	output.addSubMenu("CC", CC);
	output.addSubMenu("CC Channel", CCChan);
	output.addSubMenu("Audio Trig. MIDI", audioOutputMIDI);
	output.addSeparator();
	output.addItem(701, "Bipolar CC", true, audioProcessor.bipolarCC);

	PopupMenu antiClick;
	antiClick.addItem(7331, "Off", true, audioProcessor.antiClick == 0);
	antiClick.addItem(7332, "Low", true, audioProcessor.antiClick == 1);
	antiClick.addItem(7333, "High", true, audioProcessor.antiClick == 2);

	PopupMenu options;
	options.addSubMenu("Anti-click", antiClick);
	options.addSubMenu("Output", output);
	options.addSubMenu("MIDI trigger chn", midiTriggerChn);
	options.addSubMenu("Audio trigger", audioTrigger);
	options.addItem(9999, "Draw sidechain", true, audioProcessor.drawSidechain);
	options.addSeparator();
	options.addItem(30, "Dual smooth", true, audioProcessor.dualSmooth);
	options.addItem(31, "Dual tension", true, audioProcessor.dualTension);



    return options;
}
void SettingsButton::executeCommand(int result) {
    auto invokeNow = [](auto&& action) { action(); };

			if (result == 0) return;
			
			else if (result >= 2010 && result <= 2027) {
				audioProcessor.midiTriggerChn = result - 2010 - 1;
			}
			else if (result >= 10 && result <= 27) { // Trigger channel
				audioProcessor.triggerChn = result - 10 - 1;
			}
			else if (result == 30) { // Dual smooth
				audioProcessor.dualSmooth = !audioProcessor.dualSmooth;
				toggleUIComponents();
			}
			else if (result == 31) { // Dual tension
				invokeNow([this]() {
					audioProcessor.dualTension = !audioProcessor.dualTension;
					audioProcessor.onTensionChange();
					toggleUIComponents();
				});
			}
			else if (result == 32) {
				invokeNow([this]() {
					audioProcessor.audioIgnoreHitsWhilePlaying = !audioProcessor.audioIgnoreHitsWhilePlaying;
				});
			}
			else if (result == 52) {
				if (audioProcessor.uimode == UIMode::Seq) {
					auto snap = audioProcessor.sequencer->cells;
					audioProcessor.sequencer->clear();
					audioProcessor.sequencer->createUndo(snap);
					audioProcessor.sequencer->build();
				}
				else {
					auto snapshot = audioProcessor.viewPattern->points;
					audioProcessor.viewPattern->clear();
					audioProcessor.viewPattern->buildSegments();
					audioProcessor.createUndoPointFromSnapshot(snapshot);
				}
			}
			else if (result == 53) {
				audioProcessor.viewPattern->copy();
			}
			else if (result == 54) {
				auto snapshot = audioProcessor.viewPattern->points;
				audioProcessor.viewPattern->paste();
				audioProcessor.viewPattern->buildSegments();
				audioProcessor.createUndoPointFromSnapshot(snapshot);
			}
			else if (result == 55) {
				auto snapshot = audioProcessor.viewPattern->points;
				audioProcessor.viewPattern->invert();
				audioProcessor.viewPattern->buildSegments();
				audioProcessor.createUndoPointFromSnapshot(snapshot);
			}
			else if (result == 56) {
				auto snapshot = audioProcessor.viewPattern->points;
				audioProcessor.viewPattern->reverse();
				audioProcessor.viewPattern->buildSegments();
				audioProcessor.createUndoPointFromSnapshot(snapshot);
			}
			else if (result == 57) {
				if (audioProcessor.uimode == UIMode::Seq) {
					audioProcessor.sequencer->doublePattern();
				}
				else {
					auto snapshot = audioProcessor.viewPattern->points;
					audioProcessor.viewPattern->doublePattern();
					audioProcessor.viewPattern->buildSegments();
					audioProcessor.createUndoPointFromSnapshot(snapshot);
				}
			}
			
			
			// output cc channel
			else if (result >= 300 && result <= 300 + 129) {
				audioProcessor.outputCC = result - 300;
			}
			else if (result >= 450 && result <= 450 + 16) {
				audioProcessor.outputCCChan = result - 450;
			}
			// output audio trigger midi note
			else if (result >= 500 && result <= 500 + 129) {
				audioProcessor.outputATMIDI = result - 500;
			}
			// output options
			else if (result == 700) {
				audioProcessor.outputCV = !audioProcessor.outputCV;
			}
			else if (result == 701) {
				audioProcessor.bipolarCC = !audioProcessor.bipolarCC;
			}
			
			
			
			else if (result >= 7331 && result <= 7333) {
				audioProcessor.setAntiClick(result - 7331);
			}
			else if (result == 9999) {
				audioProcessor.drawSidechain = !audioProcessor.drawSidechain;
			}
    audioProcessor.sendChangeMessage();
}

} // namespace volumef
