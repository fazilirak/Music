# Shaper Saturation

Shaper owns this native DSP snapshot, its parameters and state format. The source originated in the IRAK / F Saturator engine written for FAZIL IRAK. The separate F Saturator project is not a build dependency and is not modified here.

`SaturationDSP::processModulated` adds the Shaper curve's drive modulation. `EnvelopeEffect` supplies the Gate-derived curve, tempo/pattern, sequencer and trigger engine. `EnvelopeEditor` supplies the Shaper interface. Native parameter IDs and `IRAK_STATE` remain compatible with old Shaper presets.

The original static DSP still has its own regression tests in `Tests/SaturationDSPTests.cpp`. Curve routing, audible modulation and standard controls are covered by `Tests/EnvelopeTests.cpp` and `Tests/AudioTests.cpp`.
