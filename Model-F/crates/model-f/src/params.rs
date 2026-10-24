// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 24.10.2026
// Dosyanın görevi: Host parametre kimliği, aralığı, varsayılanı ve DSP kontrol modeline dönüşüm sözleşmesidir.
use crate::engine::Controls;
use model_f_core::{
    tuning::Range,
    voice::{ContourKnobs, Quality, Waveform},
};
use nih_plug::prelude::*;
use std::sync::{Arc, RwLock};

// All controls use stable IDs and are saved/automated by the host. Switches and
// selectors are stepped floats so the small editor can share one gesture path.
macro_rules! parameters {
    ($( $id:ident: $key:literal => ($name:literal, $min:expr, $max:expr, $default:expr, $step:expr) ),* $(,)?) => {
        #[derive(Params, Debug)]
        pub struct ModelFParams {
            #[persist = "preset_name"]
            pub preset_name: Arc<RwLock<String>>,
            $(#[id = $key] pub $id: FloatParam,)*
        }
        impl Default for ModelFParams {
            fn default() -> Self {
                Self {
                    preset_name: Arc::new(RwLock::new("Init — Model F".into())),
                    $($id: FloatParam::new($name, $default, FloatRange::Linear {min: $min, max: $max})
                        .with_step_size($step),)*
                }
            }
        }
        impl ModelFParams {
            pub fn list(&self) -> Vec<(&'static str, &FloatParam)> { vec![$((stringify!($id), &self.$id)),*] }
            pub fn get(&self, id: &str) -> Option<&FloatParam> { match id { $(stringify!($id) => Some(&self.$id),)* _ => None } }
        }
    }
}
parameters! {
    // Append Mono so existing preset/session plain values retain their meaning.
    mode: "mode" => ("Voice Mode", 0.0, 6.0, 6.0, 1.0),
    octave: "octave" => ("Octave", 0.0, 5.0, 2.0, 1.0),
    waveform: "waveform" => ("Waveform", 0.0, 5.0, 2.0, 1.0),
    detune: "detune" => ("Detune", 0.0, 100.0, 12.0, 0.1),
    noise: "noise" => ("Noise", 0.0, 10.0, 0.0, 0.01),
    noise_color: "noise_color" => ("Noise Color", 0.0, 1.0, 0.0, 1.0),
    cutoff: "cutoff" => ("Cutoff", -5.0, 5.0, 0.2, 0.01),
    emphasis: "emphasis" => ("Emphasis", 0.0, 10.0, 2.0, 0.01),
    contour: "contour" => ("Contour Amount", 0.0, 10.0, 4.5, 0.01),
    attack: "attack" => ("Attack", 0.0, 10.0, 0.0, 0.01),
    decay: "decay" => ("Decay", 0.0, 10.0, 3.6, 0.01),
    sustain: "sustain" => ("Sustain", 0.0, 10.0, 5.0, 0.01),
    release: "release" => ("Contour Decay", 0.0, 1.0, 1.0, 1.0),
    lfo_rate: "lfo_rate" => ("LFO Rate", 0.0, 10.0, 3.0, 0.01),
    mod_depth: "mod_depth" => ("Mod Depth", 0.0, 10.0, 0.0, 0.01),
    osc_mod: "osc_mod" => ("Osc Mod", 0.0, 1.0, 0.0, 1.0),
    filter_mod: "filter_mod" => ("Filter Mod", 0.0, 1.0, 0.0, 1.0),
    glide: "glide" => ("Glide", 0.0, 10.0, 0.0, 0.01),
    volume: "volume" => ("Volume", 0.0, 10.0, 7.0, 0.01),
    arp_on: "arp_on" => ("Arpeggiator", 0.0, 1.0, 0.0, 1.0),
    arp_mode: "arp_mode" => ("Arp Direction", 0.0, 3.0, 0.0, 1.0),
    arp_rate: "arp_rate" => ("Arp Division", 0.0, 5.0, 3.0, 1.0),
    arp_octaves: "arp_octaves" => ("Arp Octaves", 1.0, 3.0, 1.0, 1.0),
    arp_gate: "arp_gate" => ("Arp Gate", 0.1, 0.9, 0.55, 0.01),
    arp_latch: "arp_latch" => ("Arp Hold", 0.0, 1.0, 0.0, 1.0),
    arp_sync: "arp_sync" => ("Arp Host Sync", 0.0, 1.0, 1.0, 1.0),
    tempo: "tempo" => ("Tempo", 40.0, 240.0, 120.0, 0.1),
    overload: "overload" => ("Overload", 0.0, 10.0, 0.0, 0.01),
    mseg_enable: "mseg_enable" => ("Multi Stage", 0.000, 1.0, 0.0, 1.0),
    mseg_duration: "mseg_duration" => ("Envelope Duration", 0.050, 10.0, 1.0, 0.001),
    mseg_curve: "mseg_curve" => ("Envelope Curve", -1.000, 1.0, 0.0, 0.001),
    mseg_mode: "mseg_mode" => ("Envelope Playback", 0.000, 2.0, 0.0, 1.0),
    mseg_attack: "mseg_attack" => ("Envelope AD Attack", 0.000, 1.0, 0.0, 0.001),
    mseg_decay: "mseg_decay" => ("Envelope AD Decay", 0.000, 1.0, 0.0, 0.001),
    mseg_amount: "mseg_amount" => ("Envelope Amount", -1.000, 1.0, 0.0, 0.001),
    mseg_target: "mseg_target" => ("Envelope Target", 1.000, 3.0, 1.0, 1.0),
    mseg_gridx: "mseg_gridx" => ("Envelope X Grid", 1.000, 4.0, 3.0, 1.0),
    mseg_gridy: "mseg_gridy" => ("Envelope Y Grid", 1.000, 6.0, 2.0, 1.0),
    mseg_snap: "mseg_snap" => ("Envelope Snap", 0.000, 1.0, 0.0, 1.0),
    mseg_brush: "mseg_brush" => ("Envelope Wave Brush", 1.000, 8.0, 1.0, 1.0),
    mseg_start: "mseg_start" => ("Envelope Linked Ends", 0.000, 1.0, 0.0, 0.001),
    mseg_p0_x: "mseg_p0_x" => ("Envelope Point 1 x", 0.0, 1.0, 0.1200, 0.0001),
    mseg_p0_y: "mseg_p0_y" => ("Envelope Point 1 y", 0.0, 1.0, 1.0000, 0.001),
    mseg_p0_on: "mseg_p0_on" => ("Envelope Point 1 on", 0.0, 1.0, 1.0000, 1.0),
    mseg_p1_x: "mseg_p1_x" => ("Envelope Point 2 x", 0.0, 1.0, 0.2800, 0.0001),
    mseg_p1_y: "mseg_p1_y" => ("Envelope Point 2 y", 0.0, 1.0, 0.2000, 0.001),
    mseg_p1_on: "mseg_p1_on" => ("Envelope Point 2 on", 0.0, 1.0, 1.0000, 1.0),
    mseg_p2_x: "mseg_p2_x" => ("Envelope Point 3 x", 0.0, 1.0, 0.4500, 0.0001),
    mseg_p2_y: "mseg_p2_y" => ("Envelope Point 3 y", 0.0, 1.0, 0.7500, 0.001),
    mseg_p2_on: "mseg_p2_on" => ("Envelope Point 3 on", 0.0, 1.0, 1.0000, 1.0),
    mseg_p3_x: "mseg_p3_x" => ("Envelope Point 4 x", 0.0, 1.0, 0.6200, 0.0001),
    mseg_p3_y: "mseg_p3_y" => ("Envelope Point 4 y", 0.0, 1.0, 0.1500, 0.001),
    mseg_p3_on: "mseg_p3_on" => ("Envelope Point 4 on", 0.0, 1.0, 1.0000, 1.0),
    mseg_p4_x: "mseg_p4_x" => ("Envelope Point 5 x", 0.0, 1.0, 0.7800, 0.0001),
    mseg_p4_y: "mseg_p4_y" => ("Envelope Point 5 y", 0.0, 1.0, 0.5000, 0.001),
    mseg_p4_on: "mseg_p4_on" => ("Envelope Point 5 on", 0.0, 1.0, 1.0000, 1.0),
    mseg_p5_x: "mseg_p5_x" => ("Envelope Point 6 x", 0.0, 1.0, 0.9000, 0.0001),
    mseg_p5_y: "mseg_p5_y" => ("Envelope Point 6 y", 0.0, 1.0, 0.3000, 0.001),
    mseg_p5_on: "mseg_p5_on" => ("Envelope Point 6 on", 0.0, 1.0, 1.0000, 1.0),
    mseg_p6_x: "mseg_p6_x" => ("Envelope Point 7 x", 0.0, 1.0, 0.3889, 0.0001),
    mseg_p6_y: "mseg_p6_y" => ("Envelope Point 7 y", 0.0, 1.0, 0.0000, 0.001),
    mseg_p6_on: "mseg_p6_on" => ("Envelope Point 7 on", 0.0, 1.0, 0.0000, 1.0),
    mseg_p7_x: "mseg_p7_x" => ("Envelope Point 8 x", 0.0, 1.0, 0.4444, 0.0001),
    mseg_p7_y: "mseg_p7_y" => ("Envelope Point 8 y", 0.0, 1.0, 0.0000, 0.001),
    mseg_p7_on: "mseg_p7_on" => ("Envelope Point 8 on", 0.0, 1.0, 0.0000, 1.0),
    mseg_p8_x: "mseg_p8_x" => ("Envelope Point 9 x", 0.0, 1.0, 0.5000, 0.0001),
    mseg_p8_y: "mseg_p8_y" => ("Envelope Point 9 y", 0.0, 1.0, 0.0000, 0.001),
    mseg_p8_on: "mseg_p8_on" => ("Envelope Point 9 on", 0.0, 1.0, 0.0000, 1.0),
    mseg_p9_x: "mseg_p9_x" => ("Envelope Point 10 x", 0.0, 1.0, 0.5556, 0.0001),
    mseg_p9_y: "mseg_p9_y" => ("Envelope Point 10 y", 0.0, 1.0, 0.0000, 0.001),
    mseg_p9_on: "mseg_p9_on" => ("Envelope Point 10 on", 0.0, 1.0, 0.0000, 1.0),
    mseg_p10_x: "mseg_p10_x" => ("Envelope Point 11 x", 0.0, 1.0, 0.6111, 0.0001),
    mseg_p10_y: "mseg_p10_y" => ("Envelope Point 11 y", 0.0, 1.0, 0.0000, 0.001),
    mseg_p10_on: "mseg_p10_on" => ("Envelope Point 11 on", 0.0, 1.0, 0.0000, 1.0),
    mseg_p11_x: "mseg_p11_x" => ("Envelope Point 12 x", 0.0, 1.0, 0.6667, 0.0001),
    mseg_p11_y: "mseg_p11_y" => ("Envelope Point 12 y", 0.0, 1.0, 0.0000, 0.001),
    mseg_p11_on: "mseg_p11_on" => ("Envelope Point 12 on", 0.0, 1.0, 0.0000, 1.0),
    mseg_p12_x: "mseg_p12_x" => ("Envelope Point 13 x", 0.0, 1.0, 0.7222, 0.0001),
    mseg_p12_y: "mseg_p12_y" => ("Envelope Point 13 y", 0.0, 1.0, 0.0000, 0.001),
    mseg_p12_on: "mseg_p12_on" => ("Envelope Point 13 on", 0.0, 1.0, 0.0000, 1.0),
    mseg_p13_x: "mseg_p13_x" => ("Envelope Point 14 x", 0.0, 1.0, 0.7778, 0.0001),
    mseg_p13_y: "mseg_p13_y" => ("Envelope Point 14 y", 0.0, 1.0, 0.0000, 0.001),
    mseg_p13_on: "mseg_p13_on" => ("Envelope Point 14 on", 0.0, 1.0, 0.0000, 1.0),
    mseg_p14_x: "mseg_p14_x" => ("Envelope Point 15 x", 0.0, 1.0, 0.8333, 0.0001),
    mseg_p14_y: "mseg_p14_y" => ("Envelope Point 15 y", 0.0, 1.0, 0.0000, 0.001),
    mseg_p14_on: "mseg_p14_on" => ("Envelope Point 15 on", 0.0, 1.0, 0.0000, 1.0),
    mseg_p15_x: "mseg_p15_x" => ("Envelope Point 16 x", 0.0, 1.0, 0.8889, 0.0001),
    mseg_p15_y: "mseg_p15_y" => ("Envelope Point 16 y", 0.0, 1.0, 0.0000, 0.001),
    mseg_p15_on: "mseg_p15_on" => ("Envelope Point 16 on", 0.0, 1.0, 0.0000, 1.0),
    mseg_n0_in: "mseg_n0_in" => ("Envelope Node 0 in", -1.0, 1.0, 0.0, 0.001),
    mseg_n0_out: "mseg_n0_out" => ("Envelope Node 0 out", -1.0, 1.0, 0.0, 0.001),
    mseg_n0_wave: "mseg_n0_wave" => ("Envelope Node 0 wave", 0.0, 8.0, 0.0, 1.0),
    mseg_n0_depth: "mseg_n0_depth" => ("Envelope Node 0 depth", 0.0, 1.0, 1.0, 0.001),
    mseg_n1_in: "mseg_n1_in" => ("Envelope Node 1 in", -1.0, 1.0, 0.0, 0.001),
    mseg_n1_out: "mseg_n1_out" => ("Envelope Node 1 out", -1.0, 1.0, 0.0, 0.001),
    mseg_n1_wave: "mseg_n1_wave" => ("Envelope Node 1 wave", 0.0, 8.0, 0.0, 1.0),
    mseg_n1_depth: "mseg_n1_depth" => ("Envelope Node 1 depth", 0.0, 1.0, 1.0, 0.001),
    mseg_n2_in: "mseg_n2_in" => ("Envelope Node 2 in", -1.0, 1.0, 0.0, 0.001),
    mseg_n2_out: "mseg_n2_out" => ("Envelope Node 2 out", -1.0, 1.0, 0.0, 0.001),
    mseg_n2_wave: "mseg_n2_wave" => ("Envelope Node 2 wave", 0.0, 8.0, 0.0, 1.0),
    mseg_n2_depth: "mseg_n2_depth" => ("Envelope Node 2 depth", 0.0, 1.0, 1.0, 0.001),
    mseg_n3_in: "mseg_n3_in" => ("Envelope Node 3 in", -1.0, 1.0, 0.0, 0.001),
    mseg_n3_out: "mseg_n3_out" => ("Envelope Node 3 out", -1.0, 1.0, 0.0, 0.001),
    mseg_n3_wave: "mseg_n3_wave" => ("Envelope Node 3 wave", 0.0, 8.0, 0.0, 1.0),
    mseg_n3_depth: "mseg_n3_depth" => ("Envelope Node 3 depth", 0.0, 1.0, 1.0, 0.001),
    mseg_n4_in: "mseg_n4_in" => ("Envelope Node 4 in", -1.0, 1.0, 0.0, 0.001),
    mseg_n4_out: "mseg_n4_out" => ("Envelope Node 4 out", -1.0, 1.0, 0.0, 0.001),
    mseg_n4_wave: "mseg_n4_wave" => ("Envelope Node 4 wave", 0.0, 8.0, 0.0, 1.0),
    mseg_n4_depth: "mseg_n4_depth" => ("Envelope Node 4 depth", 0.0, 1.0, 1.0, 0.001),
    mseg_n5_in: "mseg_n5_in" => ("Envelope Node 5 in", -1.0, 1.0, 0.0, 0.001),
    mseg_n5_out: "mseg_n5_out" => ("Envelope Node 5 out", -1.0, 1.0, 0.0, 0.001),
    mseg_n5_wave: "mseg_n5_wave" => ("Envelope Node 5 wave", 0.0, 8.0, 0.0, 1.0),
    mseg_n5_depth: "mseg_n5_depth" => ("Envelope Node 5 depth", 0.0, 1.0, 1.0, 0.001),
    mseg_n6_in: "mseg_n6_in" => ("Envelope Node 6 in", -1.0, 1.0, 0.0, 0.001),
    mseg_n6_out: "mseg_n6_out" => ("Envelope Node 6 out", -1.0, 1.0, 0.0, 0.001),
    mseg_n6_wave: "mseg_n6_wave" => ("Envelope Node 6 wave", 0.0, 8.0, 0.0, 1.0),
    mseg_n6_depth: "mseg_n6_depth" => ("Envelope Node 6 depth", 0.0, 1.0, 1.0, 0.001),
    mseg_n7_in: "mseg_n7_in" => ("Envelope Node 7 in", -1.0, 1.0, 0.0, 0.001),
    mseg_n7_out: "mseg_n7_out" => ("Envelope Node 7 out", -1.0, 1.0, 0.0, 0.001),
    mseg_n7_wave: "mseg_n7_wave" => ("Envelope Node 7 wave", 0.0, 8.0, 0.0, 1.0),
    mseg_n7_depth: "mseg_n7_depth" => ("Envelope Node 7 depth", 0.0, 1.0, 1.0, 0.001),
    mseg_n8_in: "mseg_n8_in" => ("Envelope Node 8 in", -1.0, 1.0, 0.0, 0.001),
    mseg_n8_out: "mseg_n8_out" => ("Envelope Node 8 out", -1.0, 1.0, 0.0, 0.001),
    mseg_n8_wave: "mseg_n8_wave" => ("Envelope Node 8 wave", 0.0, 8.0, 0.0, 1.0),
    mseg_n8_depth: "mseg_n8_depth" => ("Envelope Node 8 depth", 0.0, 1.0, 1.0, 0.001),
    mseg_n9_in: "mseg_n9_in" => ("Envelope Node 9 in", -1.0, 1.0, 0.0, 0.001),
    mseg_n9_out: "mseg_n9_out" => ("Envelope Node 9 out", -1.0, 1.0, 0.0, 0.001),
    mseg_n9_wave: "mseg_n9_wave" => ("Envelope Node 9 wave", 0.0, 8.0, 0.0, 1.0),
    mseg_n9_depth: "mseg_n9_depth" => ("Envelope Node 9 depth", 0.0, 1.0, 1.0, 0.001),
    mseg_n10_in: "mseg_n10_in" => ("Envelope Node 10 in", -1.0, 1.0, 0.0, 0.001),
    mseg_n10_out: "mseg_n10_out" => ("Envelope Node 10 out", -1.0, 1.0, 0.0, 0.001),
    mseg_n10_wave: "mseg_n10_wave" => ("Envelope Node 10 wave", 0.0, 8.0, 0.0, 1.0),
    mseg_n10_depth: "mseg_n10_depth" => ("Envelope Node 10 depth", 0.0, 1.0, 1.0, 0.001),
    mseg_n11_in: "mseg_n11_in" => ("Envelope Node 11 in", -1.0, 1.0, 0.0, 0.001),
    mseg_n11_out: "mseg_n11_out" => ("Envelope Node 11 out", -1.0, 1.0, 0.0, 0.001),
    mseg_n11_wave: "mseg_n11_wave" => ("Envelope Node 11 wave", 0.0, 8.0, 0.0, 1.0),
    mseg_n11_depth: "mseg_n11_depth" => ("Envelope Node 11 depth", 0.0, 1.0, 1.0, 0.001),
    mseg_n12_in: "mseg_n12_in" => ("Envelope Node 12 in", -1.0, 1.0, 0.0, 0.001),
    mseg_n12_out: "mseg_n12_out" => ("Envelope Node 12 out", -1.0, 1.0, 0.0, 0.001),
    mseg_n12_wave: "mseg_n12_wave" => ("Envelope Node 12 wave", 0.0, 8.0, 0.0, 1.0),
    mseg_n12_depth: "mseg_n12_depth" => ("Envelope Node 12 depth", 0.0, 1.0, 1.0, 0.001),
    mseg_n13_in: "mseg_n13_in" => ("Envelope Node 13 in", -1.0, 1.0, 0.0, 0.001),
    mseg_n13_out: "mseg_n13_out" => ("Envelope Node 13 out", -1.0, 1.0, 0.0, 0.001),
    mseg_n13_wave: "mseg_n13_wave" => ("Envelope Node 13 wave", 0.0, 8.0, 0.0, 1.0),
    mseg_n13_depth: "mseg_n13_depth" => ("Envelope Node 13 depth", 0.0, 1.0, 1.0, 0.001),
    mseg_n14_in: "mseg_n14_in" => ("Envelope Node 14 in", -1.0, 1.0, 0.0, 0.001),
    mseg_n14_out: "mseg_n14_out" => ("Envelope Node 14 out", -1.0, 1.0, 0.0, 0.001),
    mseg_n14_wave: "mseg_n14_wave" => ("Envelope Node 14 wave", 0.0, 8.0, 0.0, 1.0),
    mseg_n14_depth: "mseg_n14_depth" => ("Envelope Node 14 depth", 0.0, 1.0, 1.0, 0.001),
    mseg_n15_in: "mseg_n15_in" => ("Envelope Node 15 in", -1.0, 1.0, 0.0, 0.001),
    mseg_n15_out: "mseg_n15_out" => ("Envelope Node 15 out", -1.0, 1.0, 0.0, 0.001),
    mseg_n15_wave: "mseg_n15_wave" => ("Envelope Node 15 wave", 0.0, 8.0, 0.0, 1.0),
    mseg_n15_depth: "mseg_n15_depth" => ("Envelope Node 15 depth", 0.0, 1.0, 1.0, 0.001),
    mseg_n16_in: "mseg_n16_in" => ("Envelope Node 16 in", -1.0, 1.0, 0.0, 0.001),
    mseg_n16_out: "mseg_n16_out" => ("Envelope Node 16 out", -1.0, 1.0, 0.0, 0.001),
    mseg_n16_wave: "mseg_n16_wave" => ("Envelope Node 16 wave", 0.0, 8.0, 0.0, 1.0),
    mseg_n16_depth: "mseg_n16_depth" => ("Envelope Node 16 depth", 0.0, 1.0, 1.0, 0.001),
    mseg_n17_in: "mseg_n17_in" => ("Envelope Node 17 in", -1.0, 1.0, 0.0, 0.001),
    mseg_n17_out: "mseg_n17_out" => ("Envelope Node 17 out", -1.0, 1.0, 0.0, 0.001),
    mseg_n17_wave: "mseg_n17_wave" => ("Envelope Node 17 wave", 0.0, 8.0, 0.0, 1.0),
    mseg_n17_depth: "mseg_n17_depth" => ("Envelope Node 17 depth", 0.0, 1.0, 1.0, 0.001),
    // Fazıl Irak: eski kimliklere dokunmadan yeni kontroller sona eklenir.
    // -1 eski presetin ortak değerini izler; yeni kayıtta/ilk GUI düzenlemesinde çözülür.
    osc2_waveform: "osc2_waveform" => ("OSC 2 Waveform", -1.0, 5.0, -1.0, 1.0),
    osc2_octave: "osc2_octave" => ("OSC 2 Range", -1.0, 5.0, -1.0, 1.0),
    osc1_level: "osc1_level" => ("OSC 1 Level", 0.0, 10.0, 7.6, 0.01),
    osc2_level: "osc2_level" => ("OSC 2 Level", 0.0, 10.0, 7.6, 0.01),
    osc1_tune: "osc1_tune" => ("OSC 1 Tune Semitones", -12.0, 12.0, 0.0, 0.01),
    audio_attack: "audio_attack" => ("Audio Env Attack", -1.0, 10.0, -1.0, 0.01),
    audio_decay: "audio_decay" => ("Audio Env Decay", -1.0, 10.0, -1.0, 0.01),
    audio_sustain: "audio_sustain" => ("Audio Env Sustain", -1.0, 10.0, -1.0, 0.01),
    osc_sync: "osc_sync" => ("OSC 1 to 2 Hard Sync", 0.0, 1.0, 0.0, 1.0),
    fm_on: "fm_on" => ("OSC 2 to 1 FM", 0.0, 1.0, 0.0, 1.0),
    fm_amount: "fm_amount" => ("FM Depth", 0.0, 1.0, 0.2, 0.001),
}

impl ModelFParams {
    // Kalıtılmış eski değerleri çizim, preset ve DSP için aynı yerde çöz.
    pub fn effective(&self, id: &str) -> f32 {
        let v = self.get(id).expect("known parameter").value();
        if v >= 0.0 { return v; }
        match id {
            "osc2_waveform" => self.waveform.value(), "osc2_octave" => self.octave.value(),
            "audio_attack" => self.attack.value(), "audio_decay" => self.decay.value(),
            "audio_sustain" => self.sustain.value(), _ => v,
        }
    }
    pub fn display_normalized(&self, id: &str) -> f32 {
        if matches!(id,"audio_attack" | "audio_decay" | "audio_sustain") { self.effective(id)/10.0 } else { self.get(id).unwrap().preview_normalized(self.effective(id)) }
    }
    // envelope_slots alt adımı: Host parametre kimliği, aralığı, varsayılanı ve DSP kontrol modeline dönüşüm sözleşmesidir.
    // Gövdedeki başlıca çağrılar: f64::from → self.mseg_p0_x.value → self.mseg_p0_y.value → self.mseg_p0_on.value. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    pub fn envelope_slots(&self) -> ([crate::mseg::Point; 16], [crate::mseg::Bend; 18]) {
        (
            [
                crate::mseg::Point {
                    x: f64::from(self.mseg_p0_x.value()),
                    y: f64::from(self.mseg_p0_y.value()),
                    on: self.mseg_p0_on.value() > 0.5,
                },
                crate::mseg::Point {
                    x: f64::from(self.mseg_p1_x.value()),
                    y: f64::from(self.mseg_p1_y.value()),
                    on: self.mseg_p1_on.value() > 0.5,
                },
                crate::mseg::Point {
                    x: f64::from(self.mseg_p2_x.value()),
                    y: f64::from(self.mseg_p2_y.value()),
                    on: self.mseg_p2_on.value() > 0.5,
                },
                crate::mseg::Point {
                    x: f64::from(self.mseg_p3_x.value()),
                    y: f64::from(self.mseg_p3_y.value()),
                    on: self.mseg_p3_on.value() > 0.5,
                },
                crate::mseg::Point {
                    x: f64::from(self.mseg_p4_x.value()),
                    y: f64::from(self.mseg_p4_y.value()),
                    on: self.mseg_p4_on.value() > 0.5,
                },
                crate::mseg::Point {
                    x: f64::from(self.mseg_p5_x.value()),
                    y: f64::from(self.mseg_p5_y.value()),
                    on: self.mseg_p5_on.value() > 0.5,
                },
                crate::mseg::Point {
                    x: f64::from(self.mseg_p6_x.value()),
                    y: f64::from(self.mseg_p6_y.value()),
                    on: self.mseg_p6_on.value() > 0.5,
                },
                crate::mseg::Point {
                    x: f64::from(self.mseg_p7_x.value()),
                    y: f64::from(self.mseg_p7_y.value()),
                    on: self.mseg_p7_on.value() > 0.5,
                },
                crate::mseg::Point {
                    x: f64::from(self.mseg_p8_x.value()),
                    y: f64::from(self.mseg_p8_y.value()),
                    on: self.mseg_p8_on.value() > 0.5,
                },
                crate::mseg::Point {
                    x: f64::from(self.mseg_p9_x.value()),
                    y: f64::from(self.mseg_p9_y.value()),
                    on: self.mseg_p9_on.value() > 0.5,
                },
                crate::mseg::Point {
                    x: f64::from(self.mseg_p10_x.value()),
                    y: f64::from(self.mseg_p10_y.value()),
                    on: self.mseg_p10_on.value() > 0.5,
                },
                crate::mseg::Point {
                    x: f64::from(self.mseg_p11_x.value()),
                    y: f64::from(self.mseg_p11_y.value()),
                    on: self.mseg_p11_on.value() > 0.5,
                },
                crate::mseg::Point {
                    x: f64::from(self.mseg_p12_x.value()),
                    y: f64::from(self.mseg_p12_y.value()),
                    on: self.mseg_p12_on.value() > 0.5,
                },
                crate::mseg::Point {
                    x: f64::from(self.mseg_p13_x.value()),
                    y: f64::from(self.mseg_p13_y.value()),
                    on: self.mseg_p13_on.value() > 0.5,
                },
                crate::mseg::Point {
                    x: f64::from(self.mseg_p14_x.value()),
                    y: f64::from(self.mseg_p14_y.value()),
                    on: self.mseg_p14_on.value() > 0.5,
                },
                crate::mseg::Point {
                    x: f64::from(self.mseg_p15_x.value()),
                    y: f64::from(self.mseg_p15_y.value()),
                    on: self.mseg_p15_on.value() > 0.5,
                },
            ],
            [
                crate::mseg::Bend {
                    incoming: f64::from(self.mseg_n0_in.value()),
                    outgoing: f64::from(self.mseg_n0_out.value()),
                    wave: self.mseg_n0_wave.value() as usize,
                    depth: f64::from(self.mseg_n0_depth.value()),
                },
                crate::mseg::Bend {
                    incoming: f64::from(self.mseg_n1_in.value()),
                    outgoing: f64::from(self.mseg_n1_out.value()),
                    wave: self.mseg_n1_wave.value() as usize,
                    depth: f64::from(self.mseg_n1_depth.value()),
                },
                crate::mseg::Bend {
                    incoming: f64::from(self.mseg_n2_in.value()),
                    outgoing: f64::from(self.mseg_n2_out.value()),
                    wave: self.mseg_n2_wave.value() as usize,
                    depth: f64::from(self.mseg_n2_depth.value()),
                },
                crate::mseg::Bend {
                    incoming: f64::from(self.mseg_n3_in.value()),
                    outgoing: f64::from(self.mseg_n3_out.value()),
                    wave: self.mseg_n3_wave.value() as usize,
                    depth: f64::from(self.mseg_n3_depth.value()),
                },
                crate::mseg::Bend {
                    incoming: f64::from(self.mseg_n4_in.value()),
                    outgoing: f64::from(self.mseg_n4_out.value()),
                    wave: self.mseg_n4_wave.value() as usize,
                    depth: f64::from(self.mseg_n4_depth.value()),
                },
                crate::mseg::Bend {
                    incoming: f64::from(self.mseg_n5_in.value()),
                    outgoing: f64::from(self.mseg_n5_out.value()),
                    wave: self.mseg_n5_wave.value() as usize,
                    depth: f64::from(self.mseg_n5_depth.value()),
                },
                crate::mseg::Bend {
                    incoming: f64::from(self.mseg_n6_in.value()),
                    outgoing: f64::from(self.mseg_n6_out.value()),
                    wave: self.mseg_n6_wave.value() as usize,
                    depth: f64::from(self.mseg_n6_depth.value()),
                },
                crate::mseg::Bend {
                    incoming: f64::from(self.mseg_n7_in.value()),
                    outgoing: f64::from(self.mseg_n7_out.value()),
                    wave: self.mseg_n7_wave.value() as usize,
                    depth: f64::from(self.mseg_n7_depth.value()),
                },
                crate::mseg::Bend {
                    incoming: f64::from(self.mseg_n8_in.value()),
                    outgoing: f64::from(self.mseg_n8_out.value()),
                    wave: self.mseg_n8_wave.value() as usize,
                    depth: f64::from(self.mseg_n8_depth.value()),
                },
                crate::mseg::Bend {
                    incoming: f64::from(self.mseg_n9_in.value()),
                    outgoing: f64::from(self.mseg_n9_out.value()),
                    wave: self.mseg_n9_wave.value() as usize,
                    depth: f64::from(self.mseg_n9_depth.value()),
                },
                crate::mseg::Bend {
                    incoming: f64::from(self.mseg_n10_in.value()),
                    outgoing: f64::from(self.mseg_n10_out.value()),
                    wave: self.mseg_n10_wave.value() as usize,
                    depth: f64::from(self.mseg_n10_depth.value()),
                },
                crate::mseg::Bend {
                    incoming: f64::from(self.mseg_n11_in.value()),
                    outgoing: f64::from(self.mseg_n11_out.value()),
                    wave: self.mseg_n11_wave.value() as usize,
                    depth: f64::from(self.mseg_n11_depth.value()),
                },
                crate::mseg::Bend {
                    incoming: f64::from(self.mseg_n12_in.value()),
                    outgoing: f64::from(self.mseg_n12_out.value()),
                    wave: self.mseg_n12_wave.value() as usize,
                    depth: f64::from(self.mseg_n12_depth.value()),
                },
                crate::mseg::Bend {
                    incoming: f64::from(self.mseg_n13_in.value()),
                    outgoing: f64::from(self.mseg_n13_out.value()),
                    wave: self.mseg_n13_wave.value() as usize,
                    depth: f64::from(self.mseg_n13_depth.value()),
                },
                crate::mseg::Bend {
                    incoming: f64::from(self.mseg_n14_in.value()),
                    outgoing: f64::from(self.mseg_n14_out.value()),
                    wave: self.mseg_n14_wave.value() as usize,
                    depth: f64::from(self.mseg_n14_depth.value()),
                },
                crate::mseg::Bend {
                    incoming: f64::from(self.mseg_n15_in.value()),
                    outgoing: f64::from(self.mseg_n15_out.value()),
                    wave: self.mseg_n15_wave.value() as usize,
                    depth: f64::from(self.mseg_n15_depth.value()),
                },
                crate::mseg::Bend {
                    incoming: f64::from(self.mseg_n16_in.value()),
                    outgoing: f64::from(self.mseg_n16_out.value()),
                    wave: self.mseg_n16_wave.value() as usize,
                    depth: f64::from(self.mseg_n16_depth.value()),
                },
                crate::mseg::Bend {
                    incoming: f64::from(self.mseg_n17_in.value()),
                    outgoing: f64::from(self.mseg_n17_out.value()),
                    wave: self.mseg_n17_wave.value() as usize,
                    depth: f64::from(self.mseg_n17_depth.value()),
                },
            ],
        )
    }
    // Host/panel ayarlarından DSP kontrol görünümünü hesaplar; normalize ve fiziksel değerleri ayırın.
    // Gövdedeki başlıca çağrılar: Controls::default → self.mode.value → round → self.octave.value. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    pub fn controls(&self) -> Controls {
        let mut c = Controls::default();
        let p = &mut c.panel;
        let mode = self.mode.value().round() as usize;
        let octave = self.octave.value().round() as usize;
        // 1' is the circuit's 2' range transposed one octave with its tuning bus.
        let range = [
            Range::R32,
            Range::R16,
            Range::R8,
            Range::R4,
            Range::R2,
            Range::R2,
        ][octave.min(5)];
        let wave = [
            Waveform::Triangle,
            Waveform::SharkTooth,
            Waveform::Sawtooth,
            Waveform::Square,
            Waveform::WideRectangle,
            Waveform::NarrowRectangle,
        ][(self.waveform.value().round() as usize).min(5)];
        for o in &mut p.osc[..2] {
            o.range = range;
            o.waveform = wave;
            o.on = true;
            o.volume = 0.76;
        }
        let second_octave = self.effective("osc2_octave").round() as usize;
        p.osc[1].range = [Range::R32, Range::R16, Range::R8, Range::R4, Range::R2, Range::R2][second_octave.min(5)];
        p.osc[1].waveform = [Waveform::Triangle, Waveform::SharkTooth, Waveform::Sawtooth, Waveform::Square, Waveform::WideRectangle, Waveform::NarrowRectangle][(self.effective("osc2_waveform").round() as usize).min(5)];
        p.osc[0].volume = if self.osc1_level.value() == 7.6 { 0.76 } else { f64::from(self.osc1_level.value()) / 10.0 };
        p.osc[1].volume = if self.osc2_level.value() == 7.6 { 0.76 } else { f64::from(self.osc2_level.value()) / 10.0 };
        p.osc[0].on = p.osc[0].volume > 0.0;
        p.osc[1].on = p.osc[1].volume > 0.0;
        if mode == 1 {
            p.osc[0].waveform = Waveform::Sawtooth;
            p.osc[1].on = false;
            c.supersaw = Some(f64::from(self.detune.value()) / 100.0);
        }
        // Oscillator 3 is the original circuit's free-running LFO, disconnected
        // from the audio mixer and keyboard, still feeding the modulation circuit.
        p.osc[2].on = false;
        p.osc[2].range = Range::Lo;
        p.osc[2].freq = f64::from(self.lfo_rate.value()) / 10.0;
        p.osc[2].waveform = Waveform::Triangle;
        p.osc3_control = false;
        p.osc[1].freq = 0.5 + f64::from(self.detune.value()) / 1500.0;
        p.cutoff = f64::from(self.cutoff.value() + 5.0) / 10.0;
        p.emphasis = f64::from(self.emphasis.value()) / 10.0;
        p.contour_amount = f64::from(self.contour.value()) / 10.0;
        let contour = ContourKnobs {
            attack: f64::from(self.attack.value()) / 10.0,
            decay: f64::from(self.decay.value()) / 10.0,
            sustain: f64::from(self.sustain.value()) / 10.0,
        };
        p.filter_contour = contour;
        p.loudness_contour = ContourKnobs {
            attack: f64::from(self.effective("audio_attack")) / 10.0,
            decay: f64::from(self.effective("audio_decay")) / 10.0,
            sustain: f64::from(self.effective("audio_sustain")) / 10.0,
        };
        p.decay = self.release.value() > 0.5;
        p.noise_on = self.noise.value() > 0.0;
        p.noise_volume = f64::from(self.noise.value()) / 10.0;
        p.noise_pink = self.noise_color.value() > 0.5;
        p.osc_mod = self.osc_mod.value() > 0.5;
        p.filter_mod = self.filter_mod.value() > 0.5;
        p.mod_wheel = f64::from(self.mod_depth.value()) / 10.0;
        p.glide = f64::from(self.glide.value()) / 10.0;
        p.glide_on = p.glide > 0.0;
        // Return the voice output through the external-input preamp into the ladder.
        // Zero disconnects the loop, preserving the original dry circuit.
        let drive = f64::from(self.overload.value()) / 10.0;
        p.ext_on = drive > 0.0;
        p.ext_volume = drive;
        c.feedback = overload_feedback(p.emphasis, drive > 0.0);
        p.quality = Quality::Potato;
        c.mseg = crate::mseg::Config::from_params(self);
        c.volume = f64::from(self.volume.value()) / 10.0;
        c.poly = matches!(mode, 0 | 1 | 5);
        c.voices = if mode == 1 { 8 } else { 4 };
        c.ring = matches!(mode, 4 | 5);
        c.transpose = if octave == 5 { 12.0 } else { 0.0 };
        c.interval = match mode {
            2 => 12.0,
            3 => 7.0,
            _ => 0.0,
        };
        c.transpose += f64::from(self.osc1_tune.value());
        c.interval += if second_octave == 5 { 12.0 } else { 0.0 }
            - if octave == 5 { 12.0 } else { 0.0 } - f64::from(self.osc1_tune.value());
        // SuperSaw tek 7'li banktır; ikinci audio osilatör yerine FM/Sync bankı açılmaz.
        c.sync = mode != 1 && self.osc_sync.value() > 0.5;
        c.fm = if mode != 1 && self.fm_on.value() > 0.5 { f64::from(self.fm_amount.value()) } else { 0.0 };
        c
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    // Regresyon senaryosu mono_is_default_and_uses_normal_monophonic_oscillators: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    fn mono_is_default_and_uses_normal_monophonic_oscillators() {
        let p = ModelFParams::default();
        assert_eq!(p.mode.value(), 6.0);
        let c = p.controls();
        assert!(!c.poly && !c.ring && c.supersaw.is_none());
        assert!(c.panel.osc[0].on && c.panel.osc[1].on);
    }
    #[test]
    // Regresyon senaryosu existing_mode_numbers_keep_their_dsp_behavior: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    // Gövdedeki başlıca çağrılar: FloatParam::new → Default::default → p.controls → c.supersaw.is_some. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn existing_mode_numbers_keep_their_dsp_behavior() {
        for mode in 0..=6 {
            let p = ModelFParams {
                mode: FloatParam::new(
                    "Voice Mode",
                    mode as f32,
                    FloatRange::Linear { min: 0.0, max: 6.0 },
                ),
                ..Default::default()
            };
            let c = p.controls();
            assert_eq!(c.poly, matches!(mode, 0 | 1 | 5));
            assert_eq!(c.voices, if mode == 1 { 8 } else { 4 });
            assert_eq!(c.ring, matches!(mode, 4 | 5));
            assert_eq!(c.supersaw.is_some(), mode == 1);
            assert_eq!(
                c.interval,
                match mode {
                    2 => 12.0,
                    3 => 7.0,
                    _ => 0.0,
                }
            );
        }
    }
}

/// Restrain the loop as ladder resonance rises, preserving the original amp attack.
// overload_feedback alt adımı: Host parametre kimliği, aralığı, varsayılanı ve DSP kontrol modeline dönüşüm sözleşmesidir.
pub fn overload_feedback(emphasis: f64, enabled: bool) -> f64 {
    if enabled {
        0.18 * (1.0 - 0.7 * emphasis.clamp(0., 1.).powi(2))
    } else {
        0.0
    }
}
