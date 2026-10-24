// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: Preset sürüm geçişi, doğrulama, capture/apply, fabrika/kullanıcı dosyaları ve arama akışıdır.
use crate::params::ModelFParams;
use nih_plug::prelude::*;
use serde::{Deserialize, Serialize};
use std::{
    collections::BTreeMap,
    fs,
    io::{self, Write},
    path::{Path, PathBuf},
};

pub const CATEGORIES: [&str; 6] = ["Bass", "Lead", "Key", "Pad", "FX", "Arp"];
#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(deny_unknown_fields)]
pub struct Preset {
    pub format: u32,
    pub name: String,
    pub category: String,
    pub description: String,
    pub values: BTreeMap<String, f32>,
}
impl Preset {
    // upgrade: kullanıcı/pencere olayını bu bileşenin parametre ve sahiplik durumuna taşır; kapanmış nesne ve boş seçim yollarını koruyun.
    // Gövdedeki başlıca çağrılar: self.values.get → target.is_finite → contains → target.fract. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    pub fn upgrade(mut self) -> Result<Self, String> {
        if self.format == 1 {
            if let Some(&target) = self.values.get("mseg_target") {
                if !target.is_finite() || !(0.0..=5.0).contains(&target) || target.fract() != 0. {
                    return Err("Invalid legacy envelope target".into());
                }
                let amount = self.values.get("mseg_amount").copied().unwrap_or(0.);
                let (target, amount) = crate::mseg::legacy_target(target, amount);
                self.values.insert("mseg_target".into(), target);
                self.values.insert("mseg_amount".into(), amount);
            }
            self.format = 2;
        }
        self.validate()?;
        Ok(self)
    }
    // capture: Preset sürüm geçişi, doğrulama, capture/apply, fabrika/kullanıcı dosyaları ve arama akışıdır. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
    pub fn capture(p: &ModelFParams, name: String, category: String) -> Self {
        Self {
            format: 2,
            name,
            category,
            description: "User sound".into(),
            values: p
                .list()
                .into_iter()
                .map(|(id, p)| (id.into(), p.unmodulated_plain_value()))
                .collect(),
        }
    }
    // validate alt adımı: Preset sürüm geçişi, doğrulama, capture/apply, fabrika/kullanıcı dosyaları ve arama akışıdır.
    // Gövdedeki başlıca çağrılar: self.clone → upgrade → map → into. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    pub fn validate(&self) -> Result<(), String> {
        if self.format == 1 {
            return self.clone().upgrade().map(|_| ());
        }
        if self.format != 2 {
            return Err("Unsupported Model F preset version".into());
        }
        if self.name.trim().is_empty()
            || self.name.chars().count() > 64
            || self.name.chars().any(char::is_control)
        {
            return Err("Use a preset name of 1–64 visible characters".into());
        }
        if !CATEGORIES.contains(&self.category.as_str()) {
            return Err("Unknown preset category".into());
        }
        let p = ModelFParams::default();
        if self.values.len() != p.list().len() && !matches!(self.values.len(), 27 | 160) {
            return Err("Preset has missing or unknown controls".into());
        }
        if p.list().iter().any(|(id, _)| {
            !self.values.contains_key(*id)
                && !(*id == "overload" && matches!(self.values.len(), 27 | 160))
                && !(id.starts_with("mseg_") && self.values.len() == 27)
        }) {
            return Err("Preset has missing controls".into());
        }
        for (id, v) in &self.values {
            let Some(param) = p.get(id) else {
                return Err(format!("Unknown control: {id}"));
            };
            let lo = param.preview_plain(0.0);
            let hi = param.preview_plain(1.0);
            if !v.is_finite() || *v < lo || *v > hi {
                return Err(format!("Invalid value for {id}"));
            }
            if (*v - param.preview_plain(param.preview_normalized(*v))).abs() > 0.011 {
                return Err(format!("Invalid selector value for {id}"));
            }
        }
        Ok(())
    }
    // apply: Preset sürüm geçişi, doğrulama, capture/apply, fabrika/kullanıcı dosyaları ve arama akışıdır. Burada verilen ayar türetilmiş duruma/çıktıya aktarılır; çağrı sırası aşağıdaki gövdededir.
    // Gövdedeki başlıca çağrılar: self.clone → upgrade → self.validate → p.list. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    pub fn apply(&self, p: &ModelFParams, setter: &ParamSetter<'_>) -> Result<(), String> {
        if self.format == 1 {
            return self.clone().upgrade()?.apply(p, setter);
        }
        self.validate()?;
        for (id, param) in p.list() {
            setter.begin_set_parameter(param);
            setter.set_parameter(
                param,
                self.values
                    .get(id)
                    .copied()
                    .unwrap_or_else(|| param.default_plain_value()),
            );
            setter.end_set_parameter(param);
        }
        if let Ok(mut name) = p.preset_name.write() {
            *name = self.name.clone();
        }
        Ok(())
    }
    // differs alt adımı: Preset sürüm geçişi, doğrulama, capture/apply, fabrika/kullanıcı dosyaları ve arama akışıdır.
    // Gövdedeki başlıca çağrılar: clone → upgrade → map_or → preset.differs. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    pub fn differs(&self, p: &ModelFParams) -> bool {
        if self.format == 1 {
            return self
                .clone()
                .upgrade()
                .map_or(true, |preset| preset.differs(p));
        }
        p.list().iter().any(|(id, param)| {
            self.values.get(*id).map_or(
                (param.default_plain_value() - param.unmodulated_plain_value()).abs() > 0.011,
                |v| (*v - param.unmodulated_plain_value()).abs() > 0.011,
            )
        })
    }
}
// factory alt adımı: Preset sürüm geçişi, doğrulama, capture/apply, fabrika/kullanıcı dosyaları ve arama akışıdır.
pub fn factory() -> Vec<Preset> {
    serde_json::from_str(include_str!("../sounds/factory.json"))
        .expect("validated built-in Model F bank")
}
// directory alt adımı: Preset sürüm geçişi, doğrulama, capture/apply, fabrika/kullanıcı dosyaları ve arama akışıdır.
pub fn directory() -> PathBuf {
    if let Some(d) = std::env::var_os("MODEL_F_PRESETS") {
        return PathBuf::from(d);
    }
    #[cfg(target_os = "windows")]
    let base = std::env::var_os("APPDATA").map(PathBuf::from);
    #[cfg(not(target_os = "windows"))]
    let base = std::env::var_os("HOME").map(|d| PathBuf::from(d).join(".local/share"));
    base.unwrap_or_else(std::env::temp_dir)
        .join("Model F")
        .join("Presets")
}
// read: Preset sürüm geçişi, doğrulama, capture/apply, fabrika/kullanıcı dosyaları ve arama akışıdır. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
pub fn read(path: &Path) -> Result<Preset, String> {
    if fs::metadata(path).map_err(|e| e.to_string())?.len() > 256 * 1024 {
        return Err("Preset file is too large".into());
    }
    let preset: Preset = serde_json::from_slice(&fs::read(path).map_err(|e| e.to_string())?)
        .map_err(|e| e.to_string())?;
    preset.upgrade()
}
// save: Preset sürüm geçişi, doğrulama, capture/apply, fabrika/kullanıcı dosyaları ve arama akışıdır. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
pub fn save(path: &Path, preset: &Preset) -> Result<(), String> {
    let preset = preset.clone().upgrade()?;
    let bytes = serde_json::to_vec_pretty(&preset).map_err(|e| e.to_string())?;
    atomic_write(path, &bytes).map_err(|e| e.to_string())
}
// atomic_write alt adımı: Preset sürüm geçişi, doğrulama, capture/apply, fabrika/kullanıcı dosyaları ve arama akışıdır.
// Gövdedeki başlıca çağrılar: path.parent → unwrap_or_else → Path::new → fs::create_dir_all. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
pub(crate) fn atomic_write(path: &Path, bytes: &[u8]) -> io::Result<()> {
    let parent = path.parent().unwrap_or_else(|| Path::new("."));
    fs::create_dir_all(parent)?;
    let stamp = std::time::SystemTime::now()
        .duration_since(std::time::UNIX_EPOCH)
        .unwrap_or_default()
        .as_nanos();
    let temp = parent.join(format!(".model-f-{}-{stamp}.tmp", std::process::id()));
    let result = (|| {
        let mut f = fs::OpenOptions::new()
            .write(true)
            .create_new(true)
            .open(&temp)?;
        f.write_all(bytes)?;
        f.sync_all()?;
        drop(f);
        fs::rename(&temp, path)
    })();
    if result.is_err() {
        let _ = fs::remove_file(temp);
    }
    result
}
// save_user: Preset sürüm geçişi, doğrulama, capture/apply, fabrika/kullanıcı dosyaları ve arama akışıdır. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
// Gövdedeki başlıca çağrılar: chars → map → c.is_alphanumeric → collect. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
pub fn save_user(preset: &Preset) -> Result<PathBuf, String> {
    let safe: String = preset
        .name
        .chars()
        .map(|c| {
            if c.is_alphanumeric() || c == ' ' || c == '-' || c == '_' {
                c
            } else {
                '_'
            }
        })
        .collect();
    let path = directory().join(format!("user-{}.modelf", safe.trim()));
    if path.exists() {
        return Err(
            "That user preset already exists; choose another name or Export to replace a file"
                .into(),
        );
    }
    save(&path, preset)?;
    Ok(path)
}
// user alt adımı: Preset sürüm geçişi, doğrulama, capture/apply, fabrika/kullanıcı dosyaları ve arama akışıdır.
// Gövdedeki başlıca çağrılar: Vec::new → fs::read_dir → directory → entries.flatten. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
pub fn user() -> (Vec<Preset>, Vec<String>) {
    let mut bank = Vec::new();
    let mut errors = Vec::new();
    if let Ok(entries) = fs::read_dir(directory()) {
        for entry in entries.flatten() {
            let path = entry.path();
            if path.extension().is_some_and(|e| e == "modelf") {
                match read(&path) {
                    Ok(p) => bank.push(p),
                    Err(e) => errors.push(format!("{}: {e}", entry.file_name().to_string_lossy())),
                }
            }
        }
    }
    bank.sort_by_key(|p| p.name.to_lowercase());
    (bank, errors)
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    // Regresyon senaryosu factory_has_160_distinct_sounds_and_40_multi_stage_sounds: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    // Gövdedeki başlıca çağrılar: factory → bank.len → bank.iter → filter. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn factory_has_160_distinct_sounds_and_40_multi_stage_sounds() {
        let bank = factory();
        assert_eq!(bank.len(), 160);
        assert_eq!(
            bank.iter()
                .filter(|p| p.values["mseg_enable"] > 0.5
                    && p.values["mseg_target"] > 0.0
                    && p.values["mseg_amount"].abs() > 0.0)
                .count(),
            40
        );
        for c in CATEGORIES {
            assert!(bank.iter().filter(|p| p.category == c).count() >= 26);
        }
        let mut names = std::collections::BTreeSet::new();
        let mut sounds = std::collections::BTreeSet::new();
        for p in bank {
            p.validate().unwrap();
            assert!(names.insert(p.name.clone()));
            assert!(sounds.insert(serde_json::to_string(&p.values).unwrap()));
            assert_eq!(p.values["arp_on"] > 0.5, p.category == "Arp");
        }
    }
    #[test]
    // Regresyon senaryosu roundtrip_and_reject_bad_file_before_applying: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    // Gövdedeki başlıca çağrılar: tempfile::tempdir → unwrap → d.path → join. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn roundtrip_and_reject_bad_file_before_applying() {
        let d = tempfile::tempdir().unwrap();
        let path = d.path().join("test.modelf");
        let p = factory().remove(0);
        save(&path, &p).unwrap();
        let q = read(&path).unwrap();
        assert_eq!(q.values, p.values);
        save(&path, &q).unwrap();
        assert_eq!(read(&path).unwrap().name, p.name);
        let mut bad = p.clone();
        bad.values.insert("cutoff".into(), 999.0);
        assert!(bad.validate().is_err());
        bad = p.clone();
        bad.values.remove("volume");
        assert!(bad.validate().is_err());
        bad = p.clone();
        bad.format = 3;
        assert!(bad.validate().is_err());
        bad = p;
        bad.values.insert("mode".into(), 1.5);
        assert!(bad.validate().is_err());
    }
    #[test]
    // Regresyon senaryosu legacy_27_control_presets_remain_compatible: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    // Gövdedeki başlıca çağrılar: factory → remove → retain → id.starts_with. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn legacy_27_control_presets_remain_compatible() {
        let mut old = factory().remove(0);
        old.format = 1;
        old.values
            .retain(|id, _| !id.starts_with("mseg_") && id != "overload");
        assert_eq!(old.values.len(), 27);
        old.validate().unwrap();
        let p = std::sync::Arc::new(ModelFParams::default());
        let context = nih_plug::context::process::TestProcessContext::<crate::ModelF>::new(
            p.clone() as std::sync::Arc<dyn Params>,
            48000.,
            ProcessMode::Offline,
        );
        for (id, v) in &old.values {
            let param = p.get(id).unwrap();
            assert!(context.automate(param.as_ptr(), param.preview_normalized(*v)));
        }
        assert!(!old.differs(&p));
        assert!(!p.controls().mseg.enabled);
        old.values.remove("mode");
        old.values.insert("mseg_enable".into(), 1.);
        assert!(old.validate().is_err());
    }
    #[test]
    // Regresyon senaryosu previous_160_control_presets_default_to_dry_feedback: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    fn previous_160_control_presets_default_to_dry_feedback() {
        let mut old = factory().remove(0);
        old.values.remove("overload");
        assert_eq!(old.values.len(), 160);
        old.validate().unwrap();
        assert_eq!(ModelFParams::default().controls().feedback, 0.0);
        old.values.remove("volume");
        assert!(old.validate().is_err());
    }
    #[test]
    // Regresyon senaryosu old_targets_upgrade_without_reinterpreting_oscillator_two_as_volume: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    // Gövdedeki başlıca çağrılar: factory → remove → p.values.insert → into. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn old_targets_upgrade_without_reinterpreting_oscillator_two_as_volume() {
        for (old, new) in [(0., 1.), (1., 1.), (2., 2.), (3., 2.), (4., 3.), (5., 3.)] {
            let mut p = factory().remove(120);
            p.format = 1;
            p.values.insert("mseg_target".into(), old);
            p.values.insert("mseg_amount".into(), -0.4);
            let p = p.upgrade().unwrap();
            assert_eq!(p.format, 2);
            assert_eq!(p.values["mseg_target"], new);
            assert_eq!(
                p.values["mseg_amount"],
                if old == 0. {
                    0.
                } else if old >= 4. {
                    0.4
                } else {
                    -0.4
                }
            );
            p.validate().unwrap();
        }
    }
}
