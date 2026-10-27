# F-Kick development standards

- F-Kick is an independent instrument. Keep all work in this folder; sibling plugins and shared JUCE sources are references, not edit targets.
- Preserve the compact 760 x 360 charcoal/yellow device panel and embedded Ubuntu font. The logo is plain text, never a framed button; clicking it opens offline About/licences. No on-screen MIDI keyboard or rotary banks. HP now uses the same editable text control as Cutoff; no rotary controls. Sound/FX/SC pages share the compact lower strip; do not enlarge the window. Use drag/scroll/editable numeric text within the sample panel and thin Body/Envelope/Filter strips.
- Put real Previous/Next preset arrows immediately left/right of the preset selector. Undo/Redo belong in the options menu. Keep parameter IDs and audio behaviour stable during layout changes.
- Sample controls live under FKickSampleDropArea, the explicit native file-drop target. Preserve this parent chain so Explorer drops over waveform/text controls reach the same async loader. Dim only the excluded Start/End waveform regions; keep flags bright and the selected interval unchanged.
- Product UI is English. Write maintenance documentation and useful ownership/DSP comments in Turkish. Preserve Fazıl Irak author credits and original third-party notices.
- Keep audio processing in Engine/PluginProcessor. Never add file/network I/O, allocation, deletion, waiting locks or GUI calls to processBlock/render. Sample ownership crosses a pending atomic pointer and a fixed-capacity retirement FIFO.
- Parameter IDs and FKICK_PRESET version 1 are session/preset contracts. Preserve existing IDs when adding controls; explicitly migrate changed state formats.
- Build VST3 and Standalone on Windows; macOS targets may be generated but must not be described as tested without a real build. Run CTest, inspect actual GUI snapshots, and retain test evidence before packaging.
- Do not import proprietary source, factory samples/presets, logos or artwork from reference products. F-Kick presets use original synthesis settings.
- Deliver locally unless the user requests publication. Keep README, docs/GELISTIRICI_REHBERI_TR.md and validation results aligned with actual behaviour and limitations.


- SC is the four-curve FX-only shaper: Mix, horizontal Shift, quarter-note host PPQ sync and MIDI fallback. Keep direct kick/sample untouched. Preserve legacy LFO/AD DSP/state only; no editor/menu. Room is numeric Mix (0-100%, legacy On=100%); FX HP/LP filter only the combined effect return.


- Room Mix heads the right column; HP/LP share the next row, Bass Lift/Q the row below. Bass EQ is a 300 Hz low shelf on the full kick + sample + FX output; 0 dB must be exact bypass. The graphic is 570 x 178; inline sample numbers stay above the graph in paint and hit-test order. Keep the 760 x 360 outer size.

