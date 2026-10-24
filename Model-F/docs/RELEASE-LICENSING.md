# Yayın, lisans ve kaynak paketi

Fazıl Irak — 24.10.2026

Mevcut Rust/NIH-plug ürünü korunur; JUCE sürümü bu yayına dahil değildir. DSP ve mevcut eklenti
kimlikleri değiştirilmedi. Model F'de yalnız macOS preset/envelope dosya diyalogları etkinleştirildi.
Model F kaynak sürümü 0.1.9; F EQ 0.1.10'dur.

Ana program GPL-3.0-or-later'dır. Model F'nin Idle Foundry telifleri, NIH-plug ISC ve VST3 GPL
bildirimleri, baseview MIT/Apache koşulları, F EQ Luftikus MIT atfı korunur. URW fontları ayrı
AGPLv3 ve sınırlı belge istisnasıyla gelir; bu istisna yazılıma gömme için genişletilmez. GPLv3 ve
AGPLv3 parçalarının birleşimine ilişkin izinler kullanılır; bütün eşleşen kaynaklar sağlanır.

Her Release altında Windows ve Mac ikililerinin yanında ayrı tam kaynak ZIP'i, SHA256 manifesti
bulunur. Kaynak ZIP'i projenin kendisini, Cargo.lock, vendored Rust bağımlılıklarını, Cargo source
replacement yapılandırmasını ve değiştirilmemiş URW 20200910 kaynak arşivini içerir. Sistem
SDK'ları, Rust derleyicisi ve işletim sistemi kütüphaneleri pakete dahil değildir.

Mac paketleri Intel ve Apple Silicon için Universal VST3/CLAP/APP içerir; AU üretmez. Mac uygulama
paketleri ad-hoc imzalıdır; Apple Developer ID/noterleme yoktur. Otomatik testler build hostunun
kendi mimarisinde çalışır; diğer mimari derlenip lipo ile doğrulanır. Gerçek DAW ve her iki Mac
mimarisinde ses/donanım testi bunlardan ayrıdır.
