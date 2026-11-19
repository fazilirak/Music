# F-Sidechain 1.1.4 â€” Trance Gate bank
FazÄ±l Irak Â· 19 November 2026 Â· Local Windows build

Eight new original Trance Gate presets occupy a second compact row beneath the eight existing Volume curves. Existing program indices 0â€“7 and their data are preserved. New programs 8â€“15 are Classic 16, Offbeat Eighths, Anthem Sync, Dotted 3-3-2, Triplet Roll, Swing Chops, Held Chords and Final Beat Fill.

The new presets use a one-bar tempo-synced cycle, full wet, independent band/state recall, and the existing curve editor. Short attack/release ramps and matching silent endpoints avoid abrupt gain edges and loop discontinuities. Every pulse uses only the necessary corner nodes. The fixed 912 Ã— 511 panel is retained; toolbar and graph remain separate from both rows. The selected button is also exposed as a toggled accessibility state.

CTest passed: all 16 programs, 72 rate/tempo combinations at 44.1/48/96 kHz and 90/135/180 BPM, 64/257-sample block timing equivalence, tempo changes, finite output, bounded gain slope, new-program state recall, real preset button selection and compact row geometry. Eight original stereo pad/chord audition WAVs are included. Real editor screenshots were inspected. The final build passed pluginval level 10 (pluginval-1.1.4-final.txt), the full CTest suite and button-selection screenshot checks. The original Volume preset data file is unchanged.

Source research and exact pattern designs: TRANCE-GATE-RESEARCH.md and TRANCE-GATE-DESIGNS.json. These are original patterns informed by common gating/articulation approaches, not copied factory presets. Subjective musical listening approval remains separate from automated audio checks. No macOS/Linux build or public release was made in this task.
