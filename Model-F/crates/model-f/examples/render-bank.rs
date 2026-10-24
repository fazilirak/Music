// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: render-bank geliştirme örneğinin çıktı/ölçüm girişidir; ürün audio callback’i değildir.
//! Offline audition of the actual plug-in, not a second DSP implementation.
#![allow(unsafe_code)]
use model_f::{ModelF, presets};
use nih_plug::{
    context::{PluginApi, process::TestProcessContext},
    prelude::*,
};
use std::{fs, io::Write, sync::Arc, time::Instant};
struct Init;
impl InitContext<ModelF> for Init {
    // plugin_api alt adımı: render-bank geliştirme örneğinin çıktı/ölçüm girişidir; ürün audio callback’i değildir.
    fn plugin_api(&self) -> PluginApi {
        PluginApi::Standalone
    }
    // execute alt adımı: render-bank geliştirme örneğinin çıktı/ölçüm girişidir; ürün audio callback’i değildir.
    fn execute(&self, _: ()) {}
    // set_latency_samples: render-bank geliştirme örneğinin çıktı/ölçüm girişidir; ürün audio callback’i değildir. Burada verilen ayar türetilmiş duruma/çıktıya aktarılır; çağrı sırası aşağıdaki gövdededir.
    fn set_latency_samples(&self, _: u32) {}
    // set_current_voice_capacity: render-bank geliştirme örneğinin çıktı/ölçüm girişidir; ürün audio callback’i değildir. Burada verilen ayar türetilmiş duruma/çıktıya aktarılır; çağrı sırası aşağıdaki gövdededir.
    fn set_current_voice_capacity(&self, _: u32) {}
}
// wav alt adımı: render-bank geliştirme örneğinin çıktı/ölçüm girişidir; ürün audio callback’i değildir.
// Gövdedeki başlıca çağrılar: fs::File::create → expect → samples.len → f.write_all. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
fn wav(path: &str, samples: &[f32], rate: u32) {
    let mut f = fs::File::create(path).expect("WAV file");
    let len = (samples.len() * 2) as u32;
    f.write_all(b"RIFF").unwrap();
    f.write_all(&(len + 36).to_le_bytes()).unwrap();
    f.write_all(b"WAVEfmt ").unwrap();
    f.write_all(&16u32.to_le_bytes()).unwrap();
    f.write_all(&1u16.to_le_bytes()).unwrap();
    f.write_all(&2u16.to_le_bytes()).unwrap();
    f.write_all(&rate.to_le_bytes()).unwrap();
    f.write_all(&(rate * 4).to_le_bytes()).unwrap();
    f.write_all(&4u16.to_le_bytes()).unwrap();
    f.write_all(&16u16.to_le_bytes()).unwrap();
    f.write_all(b"data").unwrap();
    f.write_all(&len.to_le_bytes()).unwrap();
    for &v in samples {
        f.write_all(&((v.clamp(-1.0, 1.0) * 32767.0) as i16).to_le_bytes())
            .unwrap();
    }
}
// main alt adımı: render-bank geliştirme örneğinin çıktı/ölçüm girişidir; ürün audio callback’i değildir.
// Gövdedeki başlıca çağrılar: std::env::args → nth → unwrap_or_else → into. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
// Ham işaretçi/FFI sınırı var: sahiplik, ömür, null ve thread koşullarını gevşetmeyin.
fn main() {
    let dir = std::env::args()
        .nth(1)
        .unwrap_or_else(|| "docs/model-f/audio".into());
    fs::create_dir_all(&dir).unwrap();
    let rate = 48000.0;
    let config = BufferConfig {
        sample_rate: rate,
        min_buffer_size: None,
        max_buffer_size: 256,
        process_mode: ProcessMode::Offline,
    };
    let mut report = Vec::new();
    let mut demo = Vec::new();
    let start = Instant::now();
    let bank = presets::factory();
    for (index, preset) in bank.iter().enumerate() {
        preset.validate().unwrap();
        let mut plugin = ModelF::default();
        let params = plugin.parameters();
        let mut context = TestProcessContext::<ModelF>::new(
            params.clone() as Arc<dyn Params>,
            rate,
            ProcessMode::Offline,
        );
        for (id, param) in params.list() {
            assert!(context.automate(param.as_ptr(), param.preview_normalized(preset.values[id])));
        }
        assert!(plugin.initialize(&ModelF::AUDIO_IO_LAYOUTS[0], &config, &mut Init));
        let notes = if preset.category == "Bass" {
            vec![45]
        } else if preset.category == "Lead" || preset.category == "FX" {
            vec![60]
        } else {
            vec![48, 55, 60, 64]
        };
        for &note in &notes {
            context.push_event(NoteEvent::NoteOn {
                timing: 0,
                voice_id: None,
                channel: 0,
                note,
                velocity: 0.8,
            });
        }
        let mut buffer = Buffer::default();
        let (mut l, mut r) = ([0f32; 256], [0f32; 256]);
        // SAFETY: the two fixed arrays outlive buffer and every process call.
        unsafe {
            buffer.set_slices(256, |s| {
                s.push(&mut l);
                s.push(&mut r);
            });
        }
        let mut aux = AuxiliaryBuffers {
            inputs: &mut [],
            outputs: &mut [],
        };
        let mut peak = 0f32;
        let mut energy = 0f64;
        let mut count = 0usize;
        let selected = [
            "Super Weight",
            "Seven Suns",
            "Copper Tines",
            "Seven Clouds",
            "Radio Ghost",
            "Seven Runner",
            "Copper Ratchet",
            "Moving Horizon",
            "Machine Garden",
        ]
        .contains(&preset.name.as_str());
        let mut rendered = Vec::new();
        let blocks = if preset.category == "Pad" { 938 } else { 563 };
        let sound_start = Instant::now();
        for block in 0..blocks {
            if block == blocks - 94 {
                for &note in &notes {
                    context.push_event(NoteEvent::NoteOff {
                        timing: 0,
                        voice_id: None,
                        channel: 0,
                        note,
                        velocity: 0.0,
                    });
                }
            }
            plugin.process(&mut buffer, &mut aux, &mut context);
            for pair in 0..256 {
                for channel in buffer.as_slice_immutable() {
                    let x = channel[pair];
                    assert!(x.is_finite(), "{} produced NaN/Inf", preset.name);
                    peak = peak.max(x.abs());
                    energy += f64::from(x) * f64::from(x);
                    count += 1;
                    if selected {
                        rendered.push(x);
                    }
                }
            }
        }
        let rms = (energy / count as f64).sqrt();
        let seconds = count as f64 / 2.0 / f64::from(rate);
        assert!(peak > 0.0001, "{} is silent", preset.name);
        assert!(peak < 8.0, "{} is unstable: {peak}", preset.name);
        let speed = seconds / sound_start.elapsed().as_secs_f64();
        report.push(serde_json::json!({"name":preset.name,"category":preset.category,"peak":peak,"rms":rms,"seconds":seconds,"offline_speed":speed}));
        if selected {
            // Demo only: peak normalised to -3 dBFS, no clipping in the WAV.
            let gain = 0.707 / peak;
            for x in &mut rendered {
                *x *= gain;
            }
            wav(
                &format!("{dir}/{}.wav", preset.name.replace(' ', "-")),
                &rendered,
                rate as u32,
            );
            demo.extend(rendered);
            demo.extend(std::iter::repeat_n(0.0, (rate * 0.4) as usize * 2));
        }
        plugin.deactivate();
        println!(
            "{:03}/160 {:5} {:20} peak={peak:.4} rms={rms:.4} {speed:.1}x",
            index + 1,
            preset.category,
            preset.name
        );
    }
    fs::write(
        format!("{dir}/validation.json"),
        serde_json::to_vec_pretty(&report).unwrap(),
    )
    .unwrap();
    wav(&format!("{dir}/Model-F-demo.wav"), &demo, rate as u32);
    println!(
        "All 160 presets produced finite audio. {:.1}s elapsed.",
        start.elapsed().as_secs_f64()
    );
}
