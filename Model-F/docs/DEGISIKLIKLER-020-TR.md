# Model F 0.2.0 bakım notları
Fazıl Irak · 24.10.2026

## Değiştirilen katmanlar
- params.rs: 11 yeni parametre sona eklendi; OSC2 ve Audio ADS için eski değerleri izleyen -1 geçiş değeri.
- editor.rs: fiziksel keyboard code, ESC/key-up ve odak yönetimi; kalıtılmış değerlerin ilk düzenlemede bağımsızlaştırılması.
- panel.rs / mseg_ui.rs: ortak çizim/hit-test geometrisi, AD/Multi karşılıklı giriş kilidi ve beyaz/turuncu durum göstergesi.
- voice.rs / vco.rs: 2→1 exponential FM (en fazla ±2 oktav, 5 ms derinlik geçişi), 1→2 BLEP hard-sync. Bir önceki OSC2 örneği FM kaynağıdır. Oversampling reset alt-örnek ofseti korunur.
- engine.rs: kararlı mono blokta kilit/gain/gate sorguları örnek başına tekrarlanmaz. Core mixer katsayıları aynı işlem sırası korunarak önbelleğe alınır. Kalite/oversampling/voice sınırı düşürülmez.
- presets.rs / lib.rs: eski 27/160/161 parametreli dosya ve oturumlar için geçiş; yeni banka tam 172 değer kaydeder.
- generate-factory.py: 160 sesin düzenlenebilir tarifi. python generate-factory.py bankayı deterministik üretir; mevcut factory.json dosyasını değiştirir. Eski cjs tarifinin çalıştırılması engellendi.

## Müzikal kullanım
80 AD sesinde transient pitch/filter konturları, 80 Multi sesinde 16 iç noktalı döngüler vardır. Nota başına perde cümleleri 0/3/5/7/10/12 yarım ses etrafında, amplitude/filtre cümleleri farklı vurgu ve boşluklarla kurulmuştur. Aynı tuşu basılı tutarak tam cümleyi dinleyin. Bazı sesler bilerek yumuşak veya dar frekans aralığındadır; ses kartı/DAW kazancı dinleme seviyesini etkiler.

Multi Arp seslerinde arp_on=0 bırakılır: klasik arpeggiator her adımda MSEG'yi yeniden başlatır ve uzun cümlenin ilerlemesini engeller. İlk yarı Arp presetleri gerçek arpeggiator kullanır. Pad süreleri uzun ve döngülüdür. Süreleri tempo için elle 60/BPM × beat formülüyle ayarlayabilirsiniz.

## Tekrar doğrulama
cargo test -p model-f --release --locked
cargo test -p model-f-core --lib --release --locked
cargo run -p model-f --example render-bank --release -- docs/audio-020
cargo run -p model-f --example performance-check --release -- docs/performance-020.json
build.ps1 -Offline -PackageSource

Yeni testler: fiziksel tuşlar/modifier altında key-up, bağımsız OSC/ADS, AD-Multi kilidi, sessiz OSC2 ile FM, Sync ses farkı/alt örnek zamanlaması, her kategoride AD/Multi ve bütün 160 sesin finite/sessiz olmayan çıktısı. Windows host testleri genel CLAP automation/state round-trip'ini de kapsar. Native DAW'ların bütün odak davranışları ve macOS bu yerel sürümde ayrıca doğrulanmalıdır.
