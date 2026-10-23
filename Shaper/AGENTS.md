# Shaper UI standards

- All work in this project concerns Shaper. F Saturator is a separate project at `../F-Saturator`; do not edit or import its interface changes into Shaper.
- New effects must reuse the existing Shaper knob appearance and dimensions. Use `Source/Embedded/RackKnob.h`: 68 x 64 control, 16 px knob radius, 130 degree sweep each side, thin effect-coloured arc, dark circular body and white pointer. Do not introduce alternate rotary themes, large knobs or concentric controls.
- Generated legacy effect rotaries use the same `shaperui::drawKnob` renderer through `scripts/prepare_sources.py`. Edit the generator, not generated build files or upstream repositories.
- Keep the standard band graph and shared Stereo / output meter / dB / Mix panel at their established positions. Effect-specific controls belong in their own settings section.
- Keep effect parameter IDs, preset formats and audio behaviour stable for interface changes.
- Saturation and Pitch use `EnvelopeEffect` / `EnvelopeEditor` around independent native DSP. Keep the Gate-style curve editor, 12 patterns, sequencer, MIDI/audio trigger and standard controls. Pitch and formant curves are independently stored. Do not restore static transfer graphs or bespoke Pitch/Saturation editors.
