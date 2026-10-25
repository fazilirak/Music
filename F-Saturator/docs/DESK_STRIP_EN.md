# F Saturator 0.5.1 — Compact dynamics and tone strip

Developer: Fazıl Irak · Updated: 26 October 2026

Transient and Comp sit left of the model buttons; Bass and Treble sit right. All five models support them. The algorithms and graphics are independently implemented; no Klanghelm source or skin is used, and identical sound is not claimed.

| Control | What it does |
| --- | --- |
| Transient | −100% Soft to +100% Sharp: softens or emphasises attacks before saturation using a stereo-linked fast/slow envelope difference. 0% is neutral. |
| Comp | 0–100% stereo-linked feedback compression. Ratio rises from 1:1 to 6:1 while threshold falls from −12 to −30 dBFS; soft knee is 8 dB. 0% bypasses compression. No automatic makeup gain. |
| ATT LED | Normal attack 22 ms; enabled attack 1.5 ms. |
| REL LED | Normal release 220 ms; enabled release 55 ms. Fast release may produce audible modulation on low frequencies. |
| Bass | ±12 dB wide low shelf at 120 Hz. |
| Bass Tight LED | Progressively blends a 32 Hz high-pass filter; positive Bass adds a broad low-mid reduction around 290 Hz. |
| Treble | ±12 dB high shelf, normally at 6 kHz. |
| Treble Tight LED | Moves the shelf to 3.5 kHz; negative Treble adds subtle presence support near 2.3 kHz. |

Tight alone is neutral at 0 dB. Transient=0, Comp=0 and both shelves=0 dB retain the previous signal path. Drag vertically, edit the numeric value or double-click a knob to reset. Hover ATT/REL to read their full descriptions. Orange LEDs indicate enabled switches. The default window is 1040×640; minimum 960×640.

Signal path: input → Transient → existing 8× saturation/filter path → host-rate conversion → Auto Gain → Comp → Bass/Treble → Stereo Width → Mix/Output → aligned Bypass. The new strip adds no lookahead or reported latency. Mix=0 retains the dry path. Auto Gain remains the original saturation calibration; compare levels and adjust Output after compression/EQ changes.

Eight IDs are appended after the original sixteen: transient, comp, bass, treble, fast_attack, fast_release, bass_tight, treble_tight. Missing fields in old sessions load at neutral. Factory presets reset them to neutral. State version 3 uses the existing IRAK_STATE container.

Automated tests verify neutral sample identity at 44.1/48/96/192 kHz, shelf gains, Tight effects, transient shaping, compression and Fast timing, stereo linking, numerical stability, silent tails, block-size invariance, state recall and compact UI layout. These checks do not replace listening and live DAW testing.
