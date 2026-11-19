# F-Saturator 0.5.6 — UI preferences and CPU optimisation

Local Windows x64 build, 19 November 2026. Not yet published.

The compact **UI** button offers 50/75/100/125/150/175/200% scaling. The bottom-right corner resizes proportionally. The selected scale and last window size are saved per user, independently of sound presets and host audio state. Initial default is 100% (912 × 511). Opening size and display changes are limited to the available logical desktop area with room for host chrome.

ADAA caches unchanged coefficients and the preceding antiderivative evaluation without altering the formulas. Drift skips inaudible sine evaluations at zero depth while maintaining phase. The editor repaints meter regions and invalidates its transfer graph only when required. Boolean host readback now reports a canonical two-step value, fixing a state-restoration failure detected by pluginval.

8x FIR oversampling, ADAA, latency, model crossfades and sound quality are retained. The paired 0.5.5 reference comparison checked 2,621,440 samples across 44.1/48/96/192 kHz, model/parameter changes, desk processing and bypass, with a maximum difference of zero.

CTest passed 3/3: irak_dsp, irak_view, irak_reference. pluginval level 10 (seed 0x19112026) passed, including editor lifecycle, state restoration, automation, varied rates/buffers and fuzzing. See pluginval-0.5.6.txt, CPU-0.5.6.csv and AUDIO-COMPARISON-0.5.6.txt.

CPU measurements interleave old and new engines in the same process, pin the benchmark thread to one available CPU core, and report medians of three runs. Intel Core i9-13900H; stereo; 44.1/48/96 kHz; 64/256 sample blocks; drive 18 dB, bias .12, character .2, harmonics .3. These are DSP processing-time measurements, not DAW CPU-meter readings or guarantees for other presets/machines. UI invalidation was audited separately; no separate GUI/GPU timing series was recorded.

The original reporter's CPU/buffer/preset details have not arrived. Installed Ableton is 12.4.16, not the reported 12.4.3. A full Live session restart and physical multi-monitor/DPI test remain external acceptance checks. The live Standalone inspection was stopped with Escape; automated editor checks passed. No new macOS/Linux binaries, website/GitHub publication or newsletter was performed.

Measured DSP time reduction: 6.78% to 33.20%; median 21.87%.

Dependency audit: the cached JUCE build tree matches every bundled pinned source file after CRLF/LF normalization (1,359 line-ending-only differences; zero content differences). Bundled upstream SHA-256 verification is also required by the packager.
