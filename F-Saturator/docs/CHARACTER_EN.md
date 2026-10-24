# F Saturator 0.4.0 — Character controls

Developer: Fazıl Irak · Updated: 24 October 2026

The fourth concentric group sits between Saturation and Output. Inner cap: **Character**, middle ring: **Harmonic Balance**, outer ring: **Analog Drift**.

| Control | Range | Meaning |
|---|---|---|
| Character | −100% Warm to +100% Sizzle; 0% neutral | Changes the hardness of both nonlinear stages. Warm softens their transfer curves; Sizzle produces more pronounced upper harmonics. This does not change the separate Tone setting. |
| Harmonic Balance | −100% Even to +100% Odd; 0% original balance | Even increases nonlinear-stage asymmetry; Odd reduces the even-symmetric component of the static curves. This is not a spectral filter isolating every even/odd harmonic already present in music. |
| Analog Drift | 0% off to 100% subtle variation | Two slow, stereo-linked variations at 0.17 and 0.113 Hz slightly move drive and bias. Maximum gain deviation is about ±0.3 dB, with bias deviation up to ±0.012. No hiss, pitch modulation or tape wow/flutter is added. |

Drag a ring vertically or scroll over it. Hold Shift for fine adjustment. Ctrl/Alt-click or double-click a ring to restore its default. Double-click its readout for numeric entry; use signed percentages for Character and Harmonic Balance.

All three defaults are zero. Old sessions missing the new fields explicitly load them at neutral; selecting a factory preset also resets them. The original thirteen DAW parameter IDs and indices remain in order, and three new parameters are appended. New state files use version 2 of the same IRAK_STATE container. Neutral controls preserve the previous sound.

Parameters are smoothed over 25 ms. Existing model transitions and reported latency are unchanged. Auto Gain remains a reference-tone calibration at neutral Character settings, not a loudness normaliser for every new shaping setting. Check the output level and adjust Output as needed.

The static transfer preview includes Character and Harmonic Balance. Dynamic loading, magnetic history and time-varying Drift are not represented by that static graph.

Windows and macOS Universal builds run the DSP/preset/UI regression suite. Real-DAW and Intel Mac hardware validation are separate. Mac bundles are ad-hoc signed, not Developer ID signed or Apple notarised; Windows binaries are unsigned. Existing third-party licences are preserved.
