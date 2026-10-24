// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: Çevrimdışı/geliştirme performans ölçümlerinin zaman ve bölüm sayaçlarını taşır.
//! Where the voice's time goes, part by part, for `model_f_core-lab perf` (the `profile` feature;
//! performance). Without the feature a lap is nothing and the voice carries no timing.

/// The parts timed: the voice's stages, the contour generator's, the nodal solver's (the
/// keyboard's and the preamplifier's) and the filter's.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Part {
    Keyboard,
    Contours,
    Noise,
    Modulation,
    Preamp,
    Oscillators,
    ControlNode,
    Filter,
    Vca,
    Trigger,
    VTrig,
    Decay,
    Follow,
    SolverLoad,
    SolverSolve,
    FilterResample,
    FilterBias,
    FilterEval,
    FilterSolve,
    VcaBias,
}

/// How many parts there are.
pub const PARTS: usize = 20;

/// The parts' names, in [`Part`]'s order.
pub const NAMES: [&str; PARTS] = [
    "keyboard",
    "contours",
    "noise",
    "modulation",
    "preamp",
    "oscillators",
    "control node",
    "filter",
    "vca",
    "contour trigger",
    "contour v-trig",
    "contour decay",
    "contour follower",
    "solver load",
    "solver solve",
    "filter resampling",
    "filter bias",
    "filter evaluation",
    "filter solve",
    "vca bias",
];

#[cfg(feature = "profile")]
mod on {
    use super::{PARTS, Part};
    use std::sync::atomic::{AtomicU64, Ordering::Relaxed};
    use std::time::Instant;

    static TOTALS: [AtomicU64; PARTS] = [const { AtomicU64::new(0) }; PARTS];

    /// A stopwatch whose laps add to their parts' totals.
    #[derive(Debug)]
    pub struct Laps(Instant);

    impl Laps {
        // start alt adımı: Çevrimdışı/geliştirme performans ölçümlerinin zaman ve bölüm sayaçlarını taşır.
        pub fn start() -> Laps {
            Laps(Instant::now())
        }

        /// The time since the last lap (or the start) goes to `part`.
        // lap alt adımı: Çevrimdışı/geliştirme performans ölçümlerinin zaman ve bölüm sayaçlarını taşır.
        pub fn lap(&mut self, part: Part) {
            let now = Instant::now();
            TOTALS[part as usize].fetch_add((now - self.0).as_nanos() as u64, Relaxed);
            self.0 = now;
        }
    }

    /// Each part's total, ns, since the last call; the totals start again from zero.
    // take alt adımı: Çevrimdışı/geliştirme performans ölçümlerinin zaman ve bölüm sayaçlarını taşır.
    pub fn take() -> [u64; PARTS] {
        core::array::from_fn(|i| TOTALS[i].swap(0, Relaxed))
    }
}

#[cfg(not(feature = "profile"))]
mod on {
    use super::{PARTS, Part};

    #[derive(Debug)]
    pub struct Laps;

    impl Laps {
        #[inline(always)]
        // start alt adımı: Çevrimdışı/geliştirme performans ölçümlerinin zaman ve bölüm sayaçlarını taşır.
        pub fn start() -> Laps {
            Laps
        }

        #[inline(always)]
        // lap alt adımı: Çevrimdışı/geliştirme performans ölçümlerinin zaman ve bölüm sayaçlarını taşır.
        pub fn lap(&mut self, _part: Part) {}
    }

    /// Nothing is timed without the `profile` feature.
    // take alt adımı: Çevrimdışı/geliştirme performans ölçümlerinin zaman ve bölüm sayaçlarını taşır.
    pub fn take() -> [u64; PARTS] {
        [0; PARTS]
    }
}

pub use on::{Laps, take};
