// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: Basılı MIDI notaları, host/dahili tempo ve gate durumundan örnek zamanlı arpej olayları üretir.
//! Sample-clock arpeggiator. Fixed storage; no allocations on the audio thread.
use crate::{engine::Event, params::ModelFParams};

pub const DIVISIONS: [f64; 6] = [1.0, 0.5, 1.0 / 3.0, 0.25, 1.0 / 6.0, 0.125];
pub const DIVISION_NAMES: [&str; 6] = ["1/4", "1/8", "1/8T", "1/16", "1/16T", "1/32"];

#[derive(Debug)]
pub struct Arp {
    physical: [u16; 128],
    held: [bool; 128],
    enabled: bool,
    latch: bool,
    direction: usize,
    octaves: usize,
    period: f64,
    gate: f64,
    step: usize,
    until_step: f64,
    until_off: f64,
    sounding: Option<u8>,
    rng: u32,
}
impl Default for Arp {
    // Varsayılan değerleri kurar; parametre aralığı/preset varsayılanıyla birlikte değişmelidir.
    fn default() -> Self {
        Self {
            physical: [0; 128],
            held: [false; 128],
            enabled: false,
            latch: false,
            direction: 0,
            octaves: 1,
            period: 6000.0,
            gate: 0.55,
            step: 0,
            until_step: 0.0,
            until_off: f64::INFINITY,
            sounding: None,
            rng: 0x4d6f6466,
        }
    }
}
impl Arp {
    // configure: Basılı MIDI notaları, host/dahili tempo ve gate durumundan örnek zamanlı arpej olayları üretir. Burada verilen ayar türetilmiş duruma/çıktıya aktarılır; çağrı sırası aşağıdaki gövdededir.
    // Gövdedeki başlıca çağrılar: p.arp_on.value → p.arp_latch.value → p.arp_mode.value → round. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    pub fn configure(
        &mut self,
        p: &ModelFParams,
        rate: f64,
        host_tempo: Option<f64>,
        mut emit: impl FnMut(Event),
    ) {
        let enabled = p.arp_on.value() > 0.5;
        let latch = p.arp_latch.value() > 0.5;
        if self.latch && !latch {
            for n in 0..128 {
                self.held[n] = self.physical[n] != 0;
            }
        }
        self.latch = latch;
        self.direction = (p.arp_mode.value().round() as usize).min(3);
        self.octaves = (p.arp_octaves.value().round() as usize).clamp(1, 3);
        let tempo = if p.arp_sync.value() > 0.5 {
            host_tempo
        } else {
            None
        }
        .filter(|t| t.is_finite() && *t > 0.0)
        .unwrap_or(f64::from(p.tempo.value()));
        let period = rate * 60.0 / tempo * DIVISIONS[(p.arp_rate.value().round() as usize).min(5)];
        self.until_step *= period / self.period;
        self.period = period.max(1.0);
        // The source contours require ~13 ms of gate low to re-arm.
        self.gate = f64::from(p.arp_gate.value()).min((1.0 - 0.014 * rate / self.period).max(0.1));
        if enabled != self.enabled {
            emit(Event::AllNotesOff);
            self.sounding = None;
            self.step = 0;
            self.until_step = 0.0;
            self.until_off = f64::INFINITY;
            self.enabled = enabled;
            for n in 0..128 {
                self.held[n] = self.physical[n] != 0;
                if !enabled && self.held[n] {
                    emit(Event::Note {
                        key: n as u8,
                        on: true,
                    });
                }
            }
        }
        if self.enabled && !self.held.iter().any(|&h| h) {
            self.stop(&mut emit);
        }
    }
    // note alt adımı: Basılı MIDI notaları, host/dahili tempo ve gate durumundan örnek zamanlı arpej olayları üretir.
    // Gövdedeki başlıca çağrılar: channel.min → self.physical.iter → all → self.held.fill. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    pub fn note(&mut self, channel: u8, key: u8, on: bool, mut emit: impl FnMut(Event)) {
        if key > 127 {
            return;
        }
        let n = key as usize;
        let bit = 1u16 << channel.min(15);
        let was = self.physical[n] != 0;
        let empty = self.physical.iter().all(|&m| m == 0);
        if on {
            self.physical[n] |= bit;
        } else {
            self.physical[n] &= !bit;
        }
        if self.enabled {
            if on && empty {
                if self.latch {
                    self.held.fill(false);
                }
                self.step = 0;
                self.until_step = 0.0;
            }
            if on || !self.latch {
                self.held[n] = self.physical[n] != 0;
            }
            if !self.held.iter().any(|&h| h) {
                self.stop(&mut emit);
            }
        } else if was != (self.physical[n] != 0) {
            emit(Event::Note {
                key,
                on: self.physical[n] != 0,
            });
        }
    }
    // stop alt adımı: Basılı MIDI notaları, host/dahili tempo ve gate durumundan örnek zamanlı arpej olayları üretir.
    fn stop(&mut self, emit: &mut impl FnMut(Event)) {
        if let Some(key) = self.sounding.take() {
            emit(Event::Note { key, on: false });
        }
        self.until_off = f64::INFINITY;
        self.until_step = 0.0;
    }
    // Bu bileşenin çalışma koleksiyonunu/hafızasını temizler; canlı kaynak nesnesini silmekle karıştırmayın.
    pub fn clear(&mut self, mut emit: impl FnMut(Event)) {
        self.physical.fill(0);
        self.held.fill(false);
        self.stop(&mut emit);
        self.step = 0;
        emit(Event::AllNotesOff);
    }
    /// A voice-mode change releases the engine's allocation. Replay the held
    /// keys into its new allocation without losing the player's physical chord.
    // revoice alt adımı: Basılı MIDI notaları, host/dahili tempo ve gate durumundan örnek zamanlı arpej olayları üretir.
    pub fn revoice(&mut self, mut emit: impl FnMut(Event)) {
        emit(Event::AllNotesOff);
        self.sounding = None;
        self.until_off = f64::INFINITY;
        self.until_step = 0.0;
        self.step = 0;
        if !self.enabled {
            for n in 0..128 {
                if self.physical[n] != 0 {
                    emit(Event::Note {
                        key: n as u8,
                        on: true,
                    });
                }
            }
        }
    }
    // boundary alt adımı: Basılı MIDI notaları, host/dahili tempo ve gate durumundan örnek zamanlı arpej olayları üretir.
    // Gövdedeki başlıca çağrılar: self.sounding.take → emit → sort_unstable → self.step.wrapping_add. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    pub fn boundary(&mut self, mut emit: impl FnMut(Event)) {
        if !self.enabled {
            return;
        }
        if self.until_off <= 0.0 {
            if let Some(key) = self.sounding.take() {
                emit(Event::Note { key, on: false });
            }
            self.until_off = f64::INFINITY;
        }
        if self.until_step > 0.0 {
            return;
        }
        let mut notes = [0u8; 384];
        let mut len = 0;
        for oct in 0..self.octaves {
            for n in 0..128 {
                if self.held[n] && n + oct * 12 < 128 {
                    notes[len] = (n + oct * 12) as u8;
                    len += 1;
                }
            }
        }
        notes[..len].sort_unstable();
        // Deduplicate overlapping octave ranges without growing a collection.
        let mut count = 0;
        for n in 0..len {
            if count == 0 || notes[n] != notes[count - 1] {
                notes[count] = notes[n];
                count += 1;
            }
        }
        if count == 0 {
            return;
        }
        if let Some(key) = self.sounding.take() {
            emit(Event::Note { key, on: false });
        }
        let index = match self.direction {
            1 => count - 1 - self.step % count,
            2 if count > 1 => {
                let x = self.step % (2 * count - 2);
                if x < count { x } else { 2 * count - 2 - x }
            }
            3 => {
                self.rng ^= self.rng << 13;
                self.rng ^= self.rng >> 17;
                self.rng ^= self.rng << 5;
                self.rng as usize % count
            }
            _ => self.step % count,
        };
        let key = notes[index];
        emit(Event::Note { key, on: true });
        self.sounding = Some(key);
        self.step = self.step.wrapping_add(1);
        self.until_step += self.period;
        self.until_off = self.period * self.gate;
    }
    // next_in alt adımı: Basılı MIDI notaları, host/dahili tempo ve gate durumundan örnek zamanlı arpej olayları üretir.
    pub fn next_in(&self) -> usize {
        if !self.enabled || !self.held.iter().any(|&h| h) {
            return usize::MAX;
        }
        self.until_step.min(self.until_off).ceil().max(1.0) as usize
    }
    // advance alt adımı: Basılı MIDI notaları, host/dahili tempo ve gate durumundan örnek zamanlı arpej olayları üretir.
    pub fn advance(&mut self, n: usize) {
        if self.enabled && self.held.iter().any(|&h| h) {
            self.until_step -= n as f64;
            self.until_off -= n as f64;
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    // arp alt adımı: Basılı MIDI notaları, host/dahili tempo ve gate durumundan örnek zamanlı arpej olayları üretir.
    fn arp() -> Arp {
        Arp {
            enabled: true,
            period: 100.0,
            gate: 0.5,
            ..Arp::default()
        }
    }
    #[test]
    // Regresyon senaryosu up_down_and_sample_gates: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    // Gövdedeki başlıca çağrılar: arp → Vec::new → a.note → a.boundary. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn up_down_and_sample_gates() {
        let mut a = arp();
        let mut e = Vec::new();
        for n in [67, 60, 64] {
            a.note(0, n, true, |_| {});
        }
        for expected in [60, 64, 67, 60] {
            a.boundary(|x| e.push(x));
            assert_eq!(
                e.pop(),
                Some(Event::Note {
                    key: expected,
                    on: true
                })
            );
            assert_eq!(a.next_in(), 50);
            a.advance(50);
            a.boundary(|x| e.push(x));
            assert_eq!(
                e.pop(),
                Some(Event::Note {
                    key: expected,
                    on: false
                })
            );
            a.advance(50);
        }
        a.direction = 2;
        a.step = 0;
        for expected in [60, 64, 67, 64, 60] {
            a.boundary(|x| {
                if let Event::Note { key, on: true } = x {
                    assert_eq!(key, expected);
                }
            });
            a.advance(100);
        }
    }
    #[test]
    // Regresyon senaryosu latch_replaces_chord_and_channel_release_is_counted: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    fn latch_replaces_chord_and_channel_release_is_counted() {
        let mut a = arp();
        a.latch = true;
        a.note(0, 60, true, |_| {});
        a.note(1, 60, true, |_| {});
        a.note(0, 60, false, |_| {});
        assert!(a.held[60]);
        a.note(1, 60, false, |_| {});
        assert!(a.held[60]);
        a.note(0, 65, true, |_| {});
        assert!(!a.held[60]);
        assert!(a.held[65]);
        a.clear(|_| {});
        assert_eq!(a.next_in(), usize::MAX);
    }
    #[test]
    // Regresyon senaryosu octave_expansion_clamps_midi_and_deduplicates: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    fn octave_expansion_clamps_midi_and_deduplicates() {
        let mut a = arp();
        a.octaves = 3;
        for n in [115, 127] {
            a.note(0, n, true, |_| {});
        }
        for expected in [115, 127, 115] {
            a.boundary(|e| {
                if let Event::Note { key, on: true } = e {
                    assert_eq!(key, expected);
                }
            });
            a.advance(100);
        }
    }
    #[test]
    // Regresyon senaryosu disabling_hold_with_no_physical_keys_releases_the_gate: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    // Gövdedeki başlıca çağrılar: arp → a.note → a.boundary → nih_plug::prelude::FloatParam::new. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn disabling_hold_with_no_physical_keys_releases_the_gate() {
        let mut a = arp();
        a.latch = true;
        a.note(0, 60, true, |_| {});
        a.boundary(|_| {});
        a.note(0, 60, false, |_| {});
        let p = ModelFParams {
            arp_on: nih_plug::prelude::FloatParam::new(
                "Arp",
                1.0,
                nih_plug::prelude::FloatRange::Linear { min: 0.0, max: 1.0 },
            ),
            ..ModelFParams::default()
        };
        let mut off = false;
        a.configure(&p, 48000.0, None, |e| {
            if e == (Event::Note { key: 60, on: false }) {
                off = true;
            }
        });
        assert!(off);
        assert_eq!(a.next_in(), usize::MAX);
    }
}
