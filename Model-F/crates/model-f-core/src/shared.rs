// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: Devre/voice tarafından paylaşılan salt okunur veya açık sahiplikli parametre verilerini taşır.
//! Tables every voice would otherwise make a copy of, made once a process and shared
//! (performance: a voice carried about 400 KB of tables that every voice has the same of,
//! which crowded the caches when many play).

use std::sync::{Arc, Mutex, PoisonError};

/// A process's tables of one kind by the key they were made for.
#[derive(Debug)]
pub struct Shared<K, T> {
    made: Mutex<Vec<(K, Arc<T>)>>,
}

impl<K: PartialEq, T> Shared<K, T> {
    // Nesneyi ve sahip olduğu başlangıç durumunu oluşturur; tampon/bağımlılık ömrü bu kurulumla başlar.
    // Eşzamanlılık sınırı: bu kilidin hangi thread’de alındığını kontrol edin; audio callback’e yeni bekleyen kilit taşımayın.
    pub const fn new() -> Self {
        Shared {
            made: Mutex::new(Vec::new()),
        }
    }

    /// The table made for an equal key, or `make`'s, kept for the next (not on the audio
    /// thread: it can make one, and it locks).
    // get: Devre/voice tarafından paylaşılan salt okunur veya açık sahiplikli parametre verilerini taşır. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
    // Eşzamanlılık sınırı: bu kilidin hangi thread’de alındığını kontrol edin; audio callback’e yeni bekleyen kilit taşımayın.
    pub fn get(&self, key: K, make: impl FnOnce() -> T) -> Arc<T> {
        let mut made = self.made.lock().unwrap_or_else(PoisonError::into_inner);
        if let Some((_, t)) = made.iter().find(|(k, _)| *k == key) {
            return t.clone();
        }
        let t = Arc::new(make());
        made.push((key, t.clone()));
        t
    }
}

impl<K: PartialEq, T> Default for Shared<K, T> {
    // Varsayılan değerleri kurar; parametre aralığı/preset varsayılanıyla birlikte değişmelidir.
    fn default() -> Self {
        Self::new()
    }
}
