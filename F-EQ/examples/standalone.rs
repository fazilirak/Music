// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: standalone geliştirme örneğinin çıktı/ölçüm girişidir; ürün audio callback’i değildir.
// main alt adımı: standalone geliştirme örneğinin çıktı/ölçüm girişidir; ürün audio callback’i değildir.
fn main() {
    nih_plug::nih_export_standalone::<simple_eq::SimpleEq>();
}
