# F EQ dependency notes

The NIH-plug, baseview and URW Gothic sources were copied from the local Model F project. Their licenses and patch histories remain in each directory.

Pinned git dependencies have been copied into third_party for a self-contained source package: clap-sys 25d7f53, vst3-sys b3ff4d7 and assert-no-alloc a6fb4f6. NIH-plug uses the same patched baseview as the EQ editor.

Additional NIH-plug correction: CLAP handle_in_events_until now splits before a future automation event that is the first event in the buffer. Previously only subsequent events triggered the split, so a single event could be applied too early. Covered by tests/clap_host.rs with an event at sample 96 and unchanged audio before sample 96.

F EQ 0.1.1 ports the Analog mode's gain error threshold/range and noise scale from Luftikus, copyright 2019 lkjb (MIT), as used by the local Machine project in Source/Effects/Equalizer/EqualizerCore.{h,cpp}. The implementation is in src/analog.rs and the noise injection/shelf gain application is in src/dsp.rs. The topology is adapted to F EQ's three serial shelves. Root Luftikus-LICENSE.txt preserves its copyright and permission notice. Saturation in src/saturation.rs is an independent implementation; the original Luftikus Analog mode has no saturation.

F EQ 0.1.10: NIH-plug Editor has an optional host_key_down callback, forwarded by VST3 IPlugView::on_key_down with the required handled/unhandled result. F EQ consumes only unmodified Escape, Return or keypad Enter while About is open, and queues closing to its GUI frame. Constants follow Steinberg pluginterfaces/base/keycodes.h. Native keyboard events also recognize physical Escape/Enter/NumpadEnter and control-character variants.
