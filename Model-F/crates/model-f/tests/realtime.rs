// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 26.10.2026
// Dosyanın görevi: realtime regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.
//! Seven-saw detune changes, ring and poly switching must not allocate in render.
#![allow(unsafe_code)]
use model_f::engine::{Controls, Engine, Event};
use std::alloc::{GlobalAlloc, Layout, System};
use std::cell::Cell;
struct Count;
thread_local! {static ARMED:Cell<bool>=const{Cell::new(false)};static ALLOCS:Cell<usize>=const{Cell::new(0)};static FREES:Cell<usize>=const{Cell::new(0)};}
// SAFETY: forwards unchanged to System, counting only the test's processing thread.
unsafe impl GlobalAlloc for Count {
    // alloc alt adımı: realtime regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.
    // Ham işaretçi/FFI sınırı var: sahiplik, ömür, null ve thread koşullarını gevşetmeyin.
    unsafe fn alloc(&self, l: Layout) -> *mut u8 {
        if ARMED.with(Cell::get) {
            ALLOCS.with(|c| c.set(c.get() + 1));
        }
        unsafe { System.alloc(l) }
    }
    // dealloc alt adımı: realtime regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.
    // Ham işaretçi/FFI sınırı var: sahiplik, ömür, null ve thread koşullarını gevşetmeyin.
    unsafe fn dealloc(&self, p: *mut u8, l: Layout) {
        if ARMED.with(Cell::get) {
            FREES.with(|c| c.set(c.get() + 1));
        }
        unsafe { System.dealloc(p, l) }
    }
}
#[global_allocator]
static A: Count = Count;
#[test]
// Regresyon senaryosu new_modes_do_not_allocate_or_free_on_the_audio_thread: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
// Gövdedeki başlıca çağrılar: Engine::new → Controls::default → engine.set → engine.prepare. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
fn new_modes_do_not_allocate_or_free_on_the_audio_thread() {
    let mut engine = Engine::new();
    let mut c = Controls::default();
    c.supersaw = Some(0.3);
    c.panel.osc[1].on = false;
    engine.set(&c);
    engine.prepare(48000.0, 7);
    let (mut left, mut right) = ([0f32; 128], [0f32; 128]);
    let mut finite = true;
    ARMED.with(|a| a.set(true));
    c.poly = true;
    c.voices = 8;
    c.mseg.enabled = true;
    c.mseg.amount = 0.25;
    c.mseg.target = 1;
    engine.set(&c);
    for key in 40..49 {
        engine.event(Event::Note { key, on: true });
    }
    engine.render(&[], &mut left, &mut right);
    engine.end_block(128);
    finite &= left.iter().chain(&right).all(|x| x.is_finite());
    engine.event(Event::AllSoundOff);
    for block in 0..240 {
        c.panel.character=(block%6) as u8;
        c.panel.character_shape=(block%100) as f64/100.0;
        c.global_cents=(block%23) as f64-11.0;
        c.feedback = if block % 3 == 0 { 0.0 } else { 1.0 };
        c.panel.ext_on = c.feedback > 0.0;
        c.panel.ext_volume = (block % 100) as f64 / 100.0;
        c.mseg.points[0].y = (block % 100) as f64 / 100.0;
        c.mseg.target = 1 + (block % 3) as usize;
        c.mseg.duration = 0.125 + block as f64 * 0.001;
        c.mseg.enabled = block < 200;
        c.supersaw = if block < 120 {
            Some(block as f64 / 120.0)
        } else {
            None
        };
        c.ring = block >= 120 && block < 180;
        c.sync = block % 3 == 0; c.fm = if block % 2 == 0 {0.45} else {0.0};
        c.poly = block >= 180;
        c.panel.osc[1].on = c.supersaw.is_none();
        c.panel.cutoff = 0.3 + block as f64 / 1000.0;
        engine.set(&c);
        if block % 30 == 0 {
            engine.event(Event::Note {
                key: 48 + (block / 30) as u8,
                on: true,
            });
        }
        if block % 30 == 24 {
            engine.event(Event::AllNotesOff);
        }
        engine.render(&[], &mut left, &mut right);
        engine.end_block(128);
        finite &= left.iter().chain(&right).all(|x| x.is_finite());
    }
    ARMED.with(|a| a.set(false));
    assert!(finite);
    assert_eq!(ALLOCS.with(Cell::get), 0);
    assert_eq!(FREES.with(Cell::get), 0);
}
