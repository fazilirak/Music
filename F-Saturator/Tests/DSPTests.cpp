// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (c) 2026 Fazıl Irak
// Geliştirici: Fazıl Irak
// Son düzenleme: 24.10.2026
// Dosyanın görevi: Sentetik sinyallerle DSP, gecikme, preset, UI hedefleme, kararlılık ve çıktı üretimi regresyonları.
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <juce_audio_utils/juce_audio_utils.h>
#include <iostream>
#include <iomanip>
#if JUCE_WINDOWS
#include <windows.h>
#endif

namespace
{
// Fazıl Irak tarafından değiştirildi: 23.10.2026.
// CI çıktısında hangi mevcut koşulun başarısız olduğunu görünür yapar; eşikleri değiştirmez.
int verificationFailure(int code, int line)
{
    std::cerr << "Verification failed: code=" << code << ", source line=" << line << std::endl;
    return code;
}

// UI testleri bileşenleri kalıcı ComponentID üzerinden bulur; görünen yazıya bağımlı değildir.
juce::Component *byId(juce::Component &c, const juce::String &id)
{
    if (c.getComponentID() == id)
        return &c;
    for (auto *child : c.getChildren())
        if (auto *result = byId(*child, id))
            return result;
    return nullptr;
}
// 8192 örneklik ölçüm penceresi FFT binlerine tam oturan sinyallerle spektral sızıntıyı azaltır.
constexpr int size = 8192;
// Gerçek JUCE editörünü PNG olarak çizer; sürüm paketindeki ekran görüntüleri buradan üretilir.
void snapshot(juce::Component &c, const juce::File &file)
{
    auto image = c.createComponentSnapshot(c.getLocalBounds());
    file.getParentDirectory().createDirectory();
    juce::FileOutputStream output(file);
    output.setPosition(0);
    if (output.truncate().failed() || !juce::PNGImageFormat().writeImageToStream(image, output))
        throw std::runtime_error("Cannot write preview");
}
// Önce yarım saniye yerleşme süresi işler, ardından ölçüm penceresini toplar.
// Parametrelerle oversampling/ADAA karşılaştırması ve farklı blok boyları sınanır.
std::vector<float> render(irak::Settings settings, int stages = 3, bool adaa = true, double sr = 48000,
                          int bin = 1711, float amplitude = .125f, int blockSize = 256)
{
    irak::SaturationDSP dsp;
    dsp.prepare(sr, blockSize, stages, adaa);
    juce::AudioBuffer<float> b(2, blockSize);
    std::vector<float> samples;
    const int settling = (int)sr / 2, total = settling + size;
    for (int i = 0; i < total; i += blockSize)
    {
        const int n = juce::jmin(blockSize, total - i);
        b.setSize(2, n, false, false, true);
        for (int c = 0; c < 2; ++c)
            for (int j = 0; j < n; ++j)
                b.setSample(c, j,
                            amplitude * std::sin((i + j) * juce::MathConstants<double>::twoPi * bin / size));
        dsp.process(b, settings);
        for (int j = 0; j < n; ++j)
        {
            float x = b.getSample(0, j);
            if (!std::isfinite(x) || std::abs(x) > 64)
                throw std::runtime_error("unstable output");
            if (i + j >= settling)
                samples.push_back(x);
        }
    }
    return samples;
}
// RMS genliği: Auto Gain referans karşılaştırmasının temel ölçümü.
double rms(const std::vector<float> &x)
{
    double v = 0;
    for (float s : x)
        v += s * s;
    return std::sqrt(v / x.size());
}
// FFT’de Nyquist altındaki beklenen harmonikleri dışarıda bırakıp artık/toplam enerji oranını ölçer.
// Sonuç bu test sinyali içindir; bütün girişlerde geçerli bir aliasing sınırı değildir.
double aliasDb(const std::vector<float> &x, int fundamental)
{
    juce::dsp::FFT fft(13);
    std::array<float, size * 2> buffer{};
    std::copy(x.begin(), x.end(), buffer.begin());
    fft.performFrequencyOnlyForwardTransform(buffer.data());
    double bad = 0, total = 0;
    for (int k = 2; k < size / 2; ++k)
    {
        double power = buffer[(size_t)k] * buffer[(size_t)k];
        total += power;
        bool harmonic = false;
        for (int h = 1; h * fundamental < size / 2; ++h)
            if (std::abs(k - h * fundamental) <= 1)
                harmonic = true;
        if (!harmonic)
            bad += power;
    }
    return 10 * std::log10(juce::jmax(1e-20, bad / total));
}
// Seçilen harmoniğin temel frekansa göre dBc düzeyini doğrudan sinüs/kosinüs izdüşümüyle ölçer.
double harmonicDb(const std::vector<float> &x, int harmonic, int fundamental = 171)
{
    auto magnitude = [&](int bin)
    {
        double re = 0, im = 0;
        for (size_t i = 0; i < x.size(); ++i)
        {
            const double phase = juce::MathConstants<double>::twoPi * bin * i / size;
            re += x[i] * std::cos(phase);
            im -= x[i] * std::sin(phase);
        }
        return std::hypot(re, im);
    };
    return 20 * std::log10(juce::jmax(1e-12, magnitude(harmonic * fundamental) / magnitude(fundamental)));
}
// Orijinal sentetik örnekleri 48 kHz stereo 24-bit WAV olarak dışarı aktarır.
void wave(const juce::File &file, const juce::AudioBuffer<float> &b)
{
    file.getParentDirectory().createDirectory();
    auto stream = file.createOutputStream();
    if (!stream || !stream->setPosition(0) || stream->truncate().failed())
        throw std::runtime_error("Cannot create WAV");
    std::unique_ptr<juce::AudioFormatWriter> writer(
        juce::WavAudioFormat().createWriterFor(stream.get(), 48000, 2, 24, {}, 0));
    if (!writer)
        throw std::runtime_error("Cannot create WAV writer");
    stream.release();
    if (!writer->writeFromAudioSampleBuffer(b, 0, b.getNumSamples()))
        throw std::runtime_error("Cannot write WAV");
}
} // namespace
// CTest giriş noktası. Çalışma klasörü build olduğunda rapor/preset/görseller ../dist içine yazılır.
// Sıfır başarı; aşağıdaki farklı dönüş kodları başarısız kontrolü ayırt etmek içindir.
// Test dönüş kodlarını yorumlamak için docs/HATA_AYIKLAMA_TR.md tablosunu kullanın. Normal testler kurulum veya tüm DAW hostlarını doğrulamaz.
int main(int argc, char **argv)
{
#if JUCE_WINDOWS
    // Windows otomasyonunda hata diyaloglarının test sürecini bekletmesini önler.
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
#endif
    // Bileşen çizimi ve font yükleme testleri için JUCE mesaj/GUI altyapısını hazırlar.
    juce::ScopedJuceInitialiser_GUI gui;
    try
    {
        // Testin çalışma dizini build olmalıdır. Doğrudan başka dizinden çalıştırmak ../dist çıktısını beklenmeyen yerde oluşturur; CTest bu dizini sağlar.
        const auto output = juce::File::getCurrentWorkingDirectory().getChildFile("../dist");
        output.createDirectory();
        // --calibrate: Auto Gain tablosunu 1 kHz’e yakın koherent sinüsle üretir; normal testlerden ayrı çalışır.
        if (argc > 1 && juce::String(argv[1]) == "--calibrate")
        {
            std::cout << std::fixed << std::setprecision(3);
            // Beş modelde 8×+ADAA ile ham motorun artık enerjisini karşılaştırır; eşikler sağlanmazsa kod 1 döner.
            for (int m = 0; m < 5; ++m)
            {
                std::cout << "{";
                for (int d = 0; d <= 60; d += 6)
                {
                    irak::Settings s;
                    s.model = m;
                    s.drive = (float)d;
                    s.autoGain = false;
                    auto samples = render(s, 3, true, 48000, 171, .12589254f);
                    std::cout << 20 * std::log10((.12589254 / std::sqrt(2.0)) / rms(samples))
                              << (d == 60 ? "" : ",");
                }
                std::cout << "},\n";
            }
            return 0;
        }
        juce::String report =
            "# IRAK DSP verification\n\nAlias test: stereo 48 kHz; coherent FFT 8192; 10025.390625 Hz input, -18.06 dBFS peak; Drive +24 dB, Heat off, Dynamics 0; allowed harmonics below Nyquist excluded from residual. Ratios are residual energy / total energy for this test signal, not universal alias limits.\n\n";
        // Aliasing geçiş koşulu iki parçalıdır: ham motordan en az 12 dB daha düşük artık enerji ve mutlak olarak -40 dB altında sonuç.
        for (int m = 0; m < 5; ++m)
        {
            irak::Settings s;
            s.model = m;
            s.drive = 24;
            s.autoGain = false;
            s.dynamics = 0;
            const auto high = render(s), raw = render(s, 0, false);
            const double cleanAlias = aliasDb(high, 1711), rawAlias = aliasDb(raw, 1711);
            report += juce::String(irak::modelNames[m]) + ": alias energy " + juce::String(cleanAlias, 2) +
                      " dB; raw " + juce::String(rawAlias, 2) + " dB.\n\n";
            std::cout << irak::modelNames[m] << ": alias " << cleanAlias << " dB vs raw " << rawAlias
                      << " dB\n";
            // Koşul başarısızsa dönüş 1: katsayı, oversampling, ADAA veya ölçüm penceresi değişikliğini araştırın; eşiği yükselterek testi susturmayın.
            if (cleanAlias > rawAlias - 12 || cleanAlias > -40)
                return verificationFailure(1, __LINE__);
        }
        for (int m = 0; m < 5; ++m)
        {
            irak::Settings s;
            s.model = m;
            s.drive = 18;
            s.autoGain = false;
            s.dynamics = 0;
            auto rendered = render(s, 3, true, 48000, 171);
            // Triode ve Pentode karakterlerini ikinci/üçüncü harmonik üzerinden kontrol eder (kod 6/7).
            const double h2 = harmonicDb(rendered, 2), h3 = harmonicDb(rendered, 3);
            report += juce::String(irak::modelNames[m]) + ": 1 kHz H2 " + juce::String(h2, 2) + " dBc; H3 " +
                      juce::String(h3, 2) + " dBc.\n\n";
            std::cout << "Harmonics model " << m << ": H2=" << h2 << ", H3=" << h3 << std::endl;
            if (m == 4 && h2 > -65)
                return verificationFailure(6, __LINE__);
            if (m == 3 && h2 < -50)
                return verificationFailure(7, __LINE__);
            // Referans girişte sürüş boyunca Auto Gain telafisinin ±0.65 dB içinde kalmasını doğrular (kod 8).
            s.autoGain = true;
            for (float d : {0.0f, 12.0f, 24.0f, 36.0f, 54.0f})
            {
                s.drive = d;
                auto calibrated = render(s, 3, true, 48000, 171, .12589254f);
                const double db = 20 * std::log10(rms(calibrated) / (.12589254 / std::sqrt(2.0)));
                if (std::abs(db) > .65)
                {
                    std::cerr << "Auto gain " << m << " / " << d << ": " << db << " dB\n";
                    return verificationFailure(8, __LINE__);
                }
            }
        }
        {
            // Aynı sinyali 256 ve 127 örneklik bloklarla işler; çıktı farkı 1e-6’yı aşmamalıdır (kod 9).
            irak::Settings s;
            s.model = 2;
            s.drive = 14;
            auto a = render(s, 3, true, 48000, 423, .2f, 256), b = render(s, 3, true, 48000, 423, .2f, 127);
            for (size_t i = 0; i < a.size(); ++i)
                if (std::abs(a[i] - b[i]) > 1e-6f)
                {
                    std::cerr << "Block difference at " << i << ": " << a[i] - b[i] << std::endl;
                    return verificationFailure(9, __LINE__);
                }
        }
        // 44.1/48/96/192 kHz’de yüksek Drive+Heat+Bias ile sonlu/kararlı çıkış kontrolü.
        for (double sr : {44100.0, 48000.0, 96000.0, 192000.0})
            for (int m = 0; m < 5; ++m)
            {
                irak::Settings s;
                s.model = m;
                s.drive = 36;
                s.heat = true;
                s.bias = .9f;
                s.dynamics = 1;
                s.autoGain = false;
                render(s, 3, true, sr, 777, .95f);
            }
        // Exact latency-aligned dry/bypass and oversized host blocks.
        // Mix=0 ve bypass yollarında darbe tam bildirilen latency kadar gecikmelidir (kod 2).
        // 1024 örneklik host bloğu 64 kapasiteyi aşarak alt blok bölmeyi de test eder.
        for (bool bypass : {false, true})
        {
            irak::SaturationDSP dsp;
            dsp.prepare(48000, 64);
            irak::Settings s;
            s.mix = 0;
            s.bypass = bypass;
            juce::AudioBuffer<float> b(2, 1024);
            b.clear();
            b.setSample(0, 0, 1);
            dsp.process(b, s);
            for (int i = 0; i < 1024; ++i)
                if (std::abs(b.getSample(0, i) - (i == dsp.getLatency() ? 1.0f : 0.0f)) > 1e-6f)
                    return verificationFailure(2, __LINE__);
            report += "Dry latency: " + juce::String(dsp.getLatency()) + " samples.\n\n";
        }
        // Fazıl Irak: yeni kontrollerin gerçekten duyulan motoru değiştirdiğini spektral ölç.
        // Even/Odd, statik katların eğimini/asimetrisini şekillendirir; tüm müziği harmoniklere ayırmaz.
        for (int model = 0; model < 5; ++model)
        {
            irak::Settings even;
            even.model = model;
            even.drive = 18;
            even.dynamics = 0;
            even.autoGain = false;
            even.harmonics = -1;
            auto odd = even;
            odd.harmonics = 1;
            const double evenH2 = harmonicDb(render(even, 3, true, 48000, 171), 2);
            const double oddH2 = harmonicDb(render(odd, 3, true, 48000, 171), 2);
            std::cout << "Balance model " << model << ": Even H2=" << evenH2 << ", Odd H2=" << oddH2 << std::endl;
            if (evenH2 < oddH2 + 15)
                return verificationFailure(18, __LINE__);
        }
        {
            irak::Settings warm;
            warm.drive = 12;
            warm.dynamics = 0;
            warm.autoGain = false;
            warm.character = -1;
            auto sizzle = warm;
            sizzle.character = 1;
            const double warmH3 = harmonicDb(render(warm, 3, true, 48000, 171), 3);
            const double sizzleH3 = harmonicDb(render(sizzle, 3, true, 48000, 171), 3);
            std::cout << "Character H3: Warm=" << warmH3 << ", Sizzle=" << sizzleH3 << std::endl;
            if (sizzleH3 < warmH3 + 3)
                return verificationFailure(19, __LINE__);
            // Drift açıkken de blok bölünmesi ve reset sonrasında deterministik sonuç gerekir.
            sizzle.drift = 1;
            auto a = render(sizzle, 3, true, 48000, 423, .2f, 256);
            auto b = render(sizzle, 3, true, 48000, 423, .2f, 127);
            auto noDrift = sizzle;
            noDrift.drift = 0;
            auto clean = render(noDrift, 3, true, 48000, 423, .2f, 256);
            double driftDifference = 0;
            for (size_t i = 0; i < a.size(); ++i)
            {
                if (std::abs(a[i] - b[i]) > 1e-6f)
                    return verificationFailure(20, __LINE__);
                driftDifference += std::abs(a[i] - clean[i]);
            }
            if (driftDifference / a.size() < 1e-5)
                return verificationFailure(20, __LINE__);
        }
        // Yeni uç ayarlar da 8×/ADAA alias azaltımını ve örnekleme hızlarında kararlılığı korumalı.
        for (int model = 0; model < 5; ++model)
            for (float balance : {-1.0f, 1.0f})
            {
                irak::Settings s;
                s.model = model;
                s.drive = 24;
                s.character = 1;
                s.harmonics = balance;
                s.drift = 1;
                s.dynamics = 0;
                s.autoGain = false;
                // Drift koherent sinüste yan bantlar üretir; alias testinde yalnız drift kapatılır.
                s.drift = 0;
                const double high = aliasDb(render(s), 1711), raw = aliasDb(render(s, 0, false), 1711);
                if (high > -40 || high > raw - 12)
                    return verificationFailure(21, __LINE__);
                s.drift = 1;
                s.heat = true;
                s.drive = 36;
                s.bias = 1;
                for (double rate : {44100.0, 96000.0, 192000.0})
                    render(s, 3, true, rate, 777, .95f);
            }
        // Drift girişsiz sinyal yaratmamalı; mono kanal stereo ile aynı soldan sonucu vermeli.
        {
            irak::SaturationDSP stereo, mono;
            stereo.prepare(48000, 256);
            mono.prepare(48000, 256);
            irak::Settings s;
            s.character = 1;
            s.harmonics = -1;
            s.drift = 1;
            s.heat = true;
            juce::AudioBuffer<float> st(2, 256), mo(1, 256);
            for (int block = 0; block < 500; ++block)
            {
                for (int i = 0; i < 256; ++i)
                {
                    const float x = block < 100 ? .2f * std::sin((block * 256 + i) * .12f) : 0;
                    st.setSample(0, i, x); st.setSample(1, i, x); mo.setSample(0, i, x);
                }
                stereo.process(st, s); mono.process(mo, s);
                for (int i = 0; i < 256; ++i)
                    if (std::abs(st.getSample(0, i) - mo.getSample(0, i)) > 1e-6f ||
                        std::abs(st.getSample(0, i) - st.getSample(1, i)) > 1e-6f)
                        return verificationFailure(22, __LINE__);
            }
            if (st.getMagnitude(0, 0, 256) > 1e-5f)
                return verificationFailure(22, __LINE__);
        }
        // Persistent state, all factory programs and a rendered editor.
        // Presetlerin dosyaya yazılıp okunmasını ve DAW durumundan geri çağrılmasını kontrol eder (kod 3/11/12).
        irak::Processor p;
        p.setRateAndBufferSizeDetails(48000, 256);
        p.prepareToPlay(48000, 256);
        const auto bank = output.getChildFile("FactoryPresets");
        bank.createDirectory();
        for (int i = 0; i < p.getNumPrograms(); ++i)
        {
            p.setCurrentProgram(i);
            juce::MemoryBlock data;
            p.getStateInformation(data);
            const auto file = bank.getChildFile(juce::String(i + 1).paddedLeft('0', 2) + " " +
                                                p.getProgramName(i) + ".irakpreset");
            if (!file.replaceWithData(data.getData(), data.getSize()))
                return verificationFailure(11, __LINE__);
            p.setCurrentProgram((i + 1) % 16);
            juce::MemoryBlock loaded;
            if (!file.loadFileAsData(loaded) || loaded != data)
                return verificationFailure(12, __LINE__);
            p.setStateInformation(loaded.getData(), (int)loaded.getSize());
            if (p.getCurrentProgram() != i)
                return verificationFailure(3, __LINE__);
        }
        // Yeni parametreler eski otomasyon indekslerinden sonra gelmelidir.
        const char *ordered[]{"model", "drive", "tone", "lowcut", "highcut", "bias", "dynamics", "mix", "output", "width", "heat", "auto_gain", "bypass", "character", "harmonics", "drift"};
        if (p.getParameters().size() != 16) return verificationFailure(23, __LINE__);
        for (int i = 0; i < 16; ++i)
            if (p.getParameters()[i] != p.state.getParameter(ordered[i]))
                return verificationFailure(23, __LINE__);
        const auto set = [&](const char *id, float v) {
            auto *parameter = p.state.getParameter(id);
            parameter->setValueNotifyingHost(parameter->convertTo0to1(v));
        };
        set("character", -.6f); set("harmonics", .7f); set("drift", .8f);
        juce::MemoryBlock currentState;
        p.getStateInformation(currentState);
        p.setCurrentProgram(0);
        p.setStateInformation(currentState.getData(), (int)currentState.getSize());
        if (std::abs(p.settings().character + .6f) > .001f || std::abs(p.settings().harmonics - .7f) > .001f || std::abs(p.settings().drift - .8f) > .001f)
            return verificationFailure(24, __LINE__);
        auto legacy = p.state.copyState();
        for (const char *id : {"character", "harmonics", "drift"})
            legacy.removeChild(legacy.getChildWithProperty("id", id), nullptr);
        auto legacyXml = legacy.createXml();
        legacyXml->setAttribute("version", 1);
        juce::MemoryBlock legacyData;
        juce::AudioProcessor::copyXmlToBinary(*legacyXml, legacyData);
        p.setStateInformation(legacyData.getData(), (int)legacyData.getSize());
        if (p.settings().character != 0 || p.settings().harmonics != 0 || p.settings().drift != 0)
            return verificationFailure(25, __LINE__);
        p.setCurrentProgram(2);
        std::unique_ptr<juce::AudioProcessorEditor> editor(p.createEditor());
        // On iki halkada hedef seçimini ve tekerleğin yalnız doğru parametreyi değiştirdiğini sınar (kod 13–15).
        const char *groupedIds[4][3] = {
            {"tone", "lowcut", "highcut"}, {"drive", "bias", "dynamics"},
            {"character", "harmonics", "drift"}, {"mix", "output", "width"}};
        for (int group = 0; group < 4; ++group)
        {
            auto *control =
                dynamic_cast<irak::ConcentricKnob *>(byId(*editor, "saturator-group-" + juce::String(group)));
            if (!control)
                return verificationFailure(13, __LINE__);
            const float x = control->getWidth() * .5f;
            for (int part = 0; part < 3; ++part)
            {
                const float radius = juce::jmin(94.0f, control->getWidth() * .34f);
                const juce::Point<float> pos{x + radius * (part == 0 ? .3f : part == 1 ? .67f : .95f), 124};
                if (control->partAt(pos) != part)
                    return verificationFailure(14, __LINE__);
                std::array<float, 3> before;
                for (int i = 0; i < 3; ++i)
                    before[(size_t)i] = p.state.getRawParameterValue(groupedIds[group][i])->load();
                const auto now = juce::Time::getCurrentTime();
                juce::MouseEvent event(juce::Desktop::getInstance().getMainMouseSource(), pos,
                                       juce::ModifierKeys(), 1, 0, 0, 0, 0, control, control, now, pos, now,
                                       1, false);
                // Sentetik olayın deltaX/isReversed/isSmooth/isInertial alanları da tanımlı olmalı.
                // Aksi halde özellikle CI'da ters yön rastgele seçilip sınırdaki değer kıpırdamayabilir.
                juce::MouseWheelDetails wheel{};
                wheel.deltaY =
                    before[(size_t)part] >=
                            p.state.getParameter(groupedIds[group][part])->getNormalisableRange().end
                        ? -.5f
                        : .5f;
                control->mouseWheelMove(event, wheel);
                for (int i = 0; i < 3; ++i)
                {
                    const float after = p.state.getRawParameterValue(groupedIds[group][i])->load();
                    if ((i == part && std::abs(after - before[(size_t)i]) < 1e-5f) ||
                        (i != part && after != before[(size_t)i]))
                        return verificationFailure(15, __LINE__);
                }
            }
        }
        // Fazıl Irak: dar, varsayılan ve geniş yerleşimde dört grup da ayrık ve pencere içinde kalmalı.
        for (const auto dimensions : {juce::Point<int>{960, 610}, {1040, 640}, {1440, 900}})
        {
            editor->setSize(dimensions.x, dimensions.y);
            for (int group = 0; group < 4; ++group)
            {
                auto *control = dynamic_cast<irak::ConcentricKnob *>(byId(*editor, "saturator-group-" + juce::String(group)));
                if (!control || !editor->getLocalBounds().contains(control->getBounds()))
                    return verificationFailure(17, __LINE__);
                if (group > 0 && control->getBounds().intersects(byId(*editor, "saturator-group-" + juce::String(group - 1))->getBounds()))
                    return verificationFailure(17, __LINE__);
                const float radius = juce::jmin(94.0f, control->getWidth() * .34f);
                for (int part = 0; part < 3; ++part)
                    if (control->partAt({control->getWidth() * .5f + radius * (part == 0 ? .3f : part == 1 ? .67f : .95f), 124}) != part)
                        return verificationFailure(17, __LINE__);
            }
        }
        editor->setSize(1040, 640);
        // Tape Glue presetini iki pencere boyunda çizerek gerçek dağıtım görsellerini üretir.
        p.setCurrentProgram(2);
        juce::Timer::callPendingTimersSynchronously();
        snapshot(*editor, output.getChildFile("F-Saturator-preview.png"));
        editor->setSize(960, 610);
        snapshot(*editor, output.getChildFile("F-Saturator-small-preview.png"));
        // Heat/Auto Gain/Bypass anahtarlarının varlığını ve dikey sırasını doğrular (kod 16).
        auto *heat = byId(*editor, "saturator-heat");
        auto *autoGain = byId(*editor, "saturator-auto-gain");
        auto *bypass = byId(*editor, "saturator-bypass");
        if (!heat || !autoGain || !bypass || heat->getX() != autoGain->getX() ||
            heat->getY() >= autoGain->getY() || autoGain->getY() >= bypass->getY())
            return verificationFailure(16, __LINE__);
        editor.reset();
        // Parameter/model changes and a silent tail must not create NaN or a stuck DC tone.
        // Preset değişimleri sırasında NaN oluşmamasını ve sessizlik kuyruğunda kalıcı DC kalmamasını sınar (kod 4/5).
        juce::AudioBuffer<float> b(2, 256);
        juce::MidiBuffer midi;
        for (int i = 0; i < 500; ++i)
        {
            if (i % 35 == 0)
                p.setCurrentProgram((i / 35) % 16);
            for (int c = 0; c < 2; ++c)
                for (int j = 0; j < 256; ++j)
                    b.setSample(c, j, i < 200 ? .2f * std::sin((i * 256 + j) * .17f) : 0);
            p.processBlock(b, midi);
            for (int c = 0; c < 2; ++c)
                for (int j = 0; j < 256; ++j)
                    if (!std::isfinite(b.getSample(c, j)))
                        return verificationFailure(4, __LINE__);
        }
        if (b.getMagnitude(0, 0, 256) > 1e-5f)
            return verificationFailure(5, __LINE__);
        // Original synthetic audition: kick/snare/hat and bass, no third-party recordings.
        // Sabit tohumlu sentetik kick/snare/hat/bass üretir; üçüncü taraf kayıt kullanılmaz.
        juce::AudioBuffer<float> source(2, 48000 * 6);
        juce::Random random(414141);
        for (int i = 0; i < source.getNumSamples(); ++i)
        {
            const double t = i / 48000.0, k = std::fmod(t, .5), sn = std::fmod(t + .25, .5),
                         hat = std::fmod(t, .25);
            const double noise = random.nextFloat() * 2 - 1;
            const double kick =
                .26 * std::exp(-k * 22) *
                std::sin(juce::MathConstants<double>::twoPi * (48 * k + 1.5 * (1 - std::exp(-k * 65))));
            const double snare = .10 * noise * std::exp(-sn * 35),
                         hihat = .035 * noise * std::exp(-hat * 180);
            const double bass = .09 * (std::sin(juce::MathConstants<double>::twoPi * 65.406 * t) +
                                       .3 * std::sin(juce::MathConstants<double>::twoPi * 130.812 * t));
            source.setSample(0, i, (float)(kick + snare + hihat + bass));
            source.setSample(1, i, (float)(kick + snare + .8 * hihat + .9 * bass));
        }
        // Bir kuru ve beş işlenmiş WAV hazırlanır. Bu duvar saati ölçümü gerçek zamanlı çalışma garantisi değildir.
        wave(output.getChildFile("Auditions/00 Dry.wav"), source);
        for (int m = 0; m < 5; ++m)
        {
            irak::SaturationDSP dsp;
            dsp.prepare(48000, 256);
            irak::Settings s;
            s.model = m;
            s.drive = 18;
            s.autoGain = true;
            juce::AudioBuffer<float> rendered;
            rendered.makeCopyOf(source);
            const double started = juce::Time::getMillisecondCounterHiRes();
            dsp.process(rendered, s);
            const double milliseconds = juce::Time::getMillisecondCounterHiRes() - started;
            report += juce::String(irak::modelNames[m]) +
                      ": 6 s stereo / 48 kHz / block capacity 256 rendered in " +
                      juce::String(milliseconds, 1) +
                      " ms (local wall clock, not a real-time scheduling guarantee).\n\n";
            wave(
                output.getChildFile("Auditions/0" + juce::String(m + 1) + " " + irak::modelNames[m] + ".wav"),
                rendered);
        }
        report +=
            "Passed: 44.1/48/96/192 kHz extreme drive, finite samples, silent tail, harmonic character, Auto Gain reference calibration within 0.65 dB, block-size invariance, sixteen preset files and recall, dry/bypass latency, oversized blocks, model changes, editor snapshot and six original audition WAVs.\n";
        // Raporu son ölçümlerle kaydeder; başarılı test mesajını yazdırır. İstisnalarda kod 10 döner.
        output.getChildFile("DSP-REPORT.md").replaceWithText(report);
        std::cout << "IRAK DSP tests passed\n";
    }
    catch (const std::exception &error)
    {
        std::cerr << error.what() << "\n";
        return verificationFailure(10, __LINE__);
    }
    return 0;
}
