// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: model-f geliştirme örneğinin çıktı/ölçüm girişidir; ürün audio callback’i değildir.
#![cfg_attr(
    all(target_os = "windows", not(debug_assertions)),
    windows_subsystem = "windows"
)]
// main alt adımı: model-f geliştirme örneğinin çıktı/ölçüm girişidir; ürün audio callback’i değildir.
// Gövdedeki başlıca çağrılar: std::env::args → collect → cfg → iter. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
fn main() {
    let mut args: Vec<String> = std::env::args().collect();
    #[cfg(target_os = "windows")]
    if !args
        .iter()
        .any(|a| a == "-b" || a == "--backend" || a.starts_with("--backend="))
    {
        args.extend(["--backend".into(), "wasapi".into()]);
    }
    let ok =
        nih_plug::wrapper::standalone::nih_export_standalone_with_args::<model_f::ModelF, _>(args);
    #[cfg(target_os = "windows")]
    if !ok {
        rfd::MessageDialog::new().set_title("Model F").set_description("The audio device could not be opened. Close applications using the device and try again, or load Model F in your DAW.").set_level(rfd::MessageLevel::Error).show();
    }
}
