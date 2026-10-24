// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: one_cpu regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.
//! The voice on its threads, every one of them at real-time priority on the same CPU (as
//! when there are more of them than CPUs: several voices at once). A part waiting for
//! another gives it the CPU ([`model_f_core::threaded::Backoff`]), so the voice goes on,
//! slower; before, the waiting part spun and the two held each other and the CPU for good
//! (the machine stalled, 2026-09-30). The run must end well before the real-time watchdog
//! would step in ([`model_f_rt::STARVED`]) and without it. Linux; skipped where real-time
//! priority is not allowed.

#![cfg(target_os = "linux")]
#![allow(clippy::unwrap_used)]

use model_f_core::threaded::Threaded;
use model_f_core::voice::{Jacks, Quality, Voice};
use std::sync::atomic::{AtomicUsize, Ordering};
use std::time::{Duration, Instant};

/// The CPU every thread is pinned to.
static CPU: AtomicUsize = AtomicUsize::new(0);

// start alt adımı: one_cpu regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.
fn start() {
    let _ = model_f_rt::promote(Duration::from_millis(5), Duration::from_millis(2));
    let _ = model_f_rt::pin(0, CPU.load(Ordering::Relaxed));
}

#[test]
// Regresyon senaryosu the_voice_goes_on_with_all_its_threads_on_one_cpu: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
// Gövdedeki başlıca çağrılar: std::thread::available_parallelism → map_or → n.get → CPU.store. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
fn the_voice_goes_on_with_all_its_threads_on_one_cpu() {
    let cpus = std::thread::available_parallelism().map_or(1, |n| n.get());
    CPU.store(cpus - 1, Ordering::Relaxed);
    let caller = std::thread::spawn(|| {
        // Built first, at ordinary priority (its calibration is long work), as a plug-in does.
        let mut v = Threaded::new(Voice::prototype(48_000.0), start).unwrap();
        v.panel.quality = Quality::HighFidelity;
        v.note(57, true);
        model_f_rt::promote(Duration::from_millis(5), Duration::from_millis(2))?;
        model_f_rt::pin(0, CPU.load(Ordering::Relaxed)).unwrap();
        let t = Instant::now();
        let mut sink = 0.0;
        // 16 passes of 32 samples (a debug build is slow; the parts' waiting on
        // each other shows from the first).
        for _ in 0..16 {
            v.process(32, |_| Jacks::default(), |_, y| sink += y);
        }
        std::hint::black_box(sink);
        Ok::<_, String>(t.elapsed())
    });
    let took = match caller.join().unwrap() {
        Ok(t) => t,
        Err(e) => {
            eprintln!("real-time priority is not allowed here ({e}): skipped");
            return;
        }
    };
    eprintln!(
        "512 samples of the voice with six threads on one CPU: {took:?}; demotions {}",
        model_f_rt::demotions()
    );
    assert_eq!(model_f_rt::demotions(), 0, "the watchdog had to step in");
    assert!(took < model_f_rt::STARVED / 2, "{took:?}");
}
