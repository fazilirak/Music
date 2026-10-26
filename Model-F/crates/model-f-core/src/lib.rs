// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 26.10.2026
// Dosyanın görevi: Model F devre çekirdeğinin modül/kamuya açık tür sözleşmesini bir araya getirir.
//! Real-time models of the Minimoog Model D's circuits (`docs/circuit/`).
//!
//! Each model is derived from the transcribed circuit (`circuits/boards/`) and tested
//! against the circuit lab's ngspice results (`model_f_core-lab`). Transcendental functions
//! come from `libm`, so renders are the same on every platform.

pub mod a440;
pub mod a440_table;
pub mod contour;
pub mod devices;
pub mod expo;
pub mod fast;
pub mod filter_cal;
pub mod keyboard;
pub mod linear;
pub mod mna;
pub mod modulation;
pub mod noise;
pub mod preamp;
pub mod prof;
pub mod resample;
pub mod revsaw;
pub mod shared;
pub mod supersaw;
pub mod tables;
pub mod threaded;
pub mod tuning;
pub mod ulp;
pub mod unconverged;
pub mod vca;
pub mod vcf;
pub mod vco;
pub mod voice;
// Fazıl Irak, 26.10.2026: seçilebilir özgün filtre karakterleri.
pub mod character_filter;

mod overload;
