// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (c) 2026 Fazıl Irak
// Geliştirici: Fazıl Irak
// Son düzenleme: 26.10.2026
// Dosyanın görevi: Oversampling, iki ADAA katı, hafızalı manyetik/dinamik modeller, filtreler, kazanç, stereo ve gecikmesi eşlenmiş karışım.
#include "SaturationDSP.h"
#include <cmath>

namespace irak
{
namespace
{
constexpr double pi = juce::MathConstants<double>::pi;
// Tek kutuplu alçak geçiren/zarf izleyici katsayısı: frekansı örnekleme hızına göre normalize eder.
double coefficient(double hz, double sr)
{
    return 1 - std::exp(-2 * pi * hz / sr);
}
// Durum değişkenini girişe yaklaştırır; amount büyüdükçe tepki hızlanır.
double follow(double input, double &state, double amount)
{
    state += amount * (input - state);
    return state;
}
} // namespace
// log(cosh(x)) ifadesini büyük genliklerde taşma üretmeden hesaplar; tanh antitürevidir.
double SaturationDSP::ADAA::logCosh(double x)
{
    const auto a = std::abs(x);
    return a + std::log1p(std::exp(-2 * a)) - std::log(2.0);
}
// Bias kaydırılmış tanh eğrisini merkezler ve küçük sinyal eğimini normalize eder.
// k kırpmanın sertliğini, b asimetriyi belirler.
double SaturationDSP::ADAA::shape(double x, double k, double b)
{
    const double offset = std::tanh(k * b), normal = k * (1 - offset * offset);
    return (std::tanh(k * (x + b)) - offset) / normal;
}
// ADAA: iki örnekteki antitürev farkını örnek farkına böler. Fark çok küçükse
// sayısal hassasiyet kaybını önlemek için orta noktadaki doğrudan eğri kullanılır.
// Fazıl Irak: Odd tarafında f(x) eğrisinden çift simetrik bileşen çıkarılır.
// Even tarafı modelin biasını artırır; bu kontrol tüm sinyalde bir harmonik ayırıcı değildir.
double SaturationDSP::ADAA::balancedShape(double x, double k, double b, double balance)
{
    const auto forward = shape(x, k, b);
    return balance > 0 ? forward - .5 * balance * (forward + shape(-x, k, b)) : forward;
}
double SaturationDSP::ADAA::process(double x, double k, double b, bool enabled, double balance)
{
    const double last = previous;
    previous = x;
    if (!enabled)
        return balancedShape(x, k, b, balance);
    // Her iki simetrik dal aynı eski/güncel örnek çiftinden hesaplanır; yeni gecikme eklenmez.
    const auto evaluate = [&](double value, double old)
    {
        const double dx = value - old;
        if (std::abs(dx) < 1e-5 * (1 + std::abs(value) + std::abs(old)))
            return shape((value + old) * .5, k, b);
        const double offset = std::tanh(k * b), normal = k * (1 - offset * offset);
        const auto primitive = [&](double v) { return (logCosh(k * (v + b)) / k - offset * v) / normal; };
        return (primitive(value) - primitive(old)) / dx;
    };
    const double forward = evaluate(x, last);
    // Nötr yol eski işlem sırasını aynen korur; ekstra dal yalnız Odd tarafında hesaplanır.
    return balance > 0 ? forward - .5 * balance * (forward + evaluate(-x, -last)) : forward;
}
// Langevin eğrisinin sıfıra yakın tekilliğini seri açılımı ile giderir.
double SaturationDSP::Magnet::langevin(double x)
{
    if (std::abs(x) < .05)
    {
        const double x2 = x * x;
        return x * (1.0 / 3 - x2 / 45 + 2 * x2 * x2 / 945);
    }
    return 1 / std::tanh(x) - 1 / x;
}
// Manyetik eğri türevi; küçük ve büyük girişler için kararlı özel yaklaşımlar içerir.
double SaturationDSP::Magnet::langevinDerivative(double x)
{
    if (std::abs(x) < .05)
    {
        const double x2 = x * x;
        return 1.0 / 3 - x2 / 15 + 2 * x2 * x2 / 189;
    }
    if (std::abs(x) > 20)
        return 1 / (x * x);
    const double s = std::sinh(x);
    return 1 / (x * x) - 1 / (s * s);
}
// Manyetik alan yönüne bağlı tersinir/tersinmez tepkiyi birleştirir.
// Payda alt sınırı ve eğim sınırı sayısal kararlılık içindir; ölçülmüş bir donanım eşlemesi değildir.
double SaturationDSP::Magnet::slope(double h, double m, double direction, double a, double k)
{
    constexpr double c = .32, alpha = .015;
    const double q = (h + alpha * m) / a, anhysteretic = langevin(q), difference = anhysteretic - m;
    const double derivative = langevinDerivative(q) / a;
    double irreversible = 0;
    if (direction * difference > 0)
    {
        double denominator = (1 - c) * direction * k - alpha * difference;
        denominator = direction * juce::jmax(.03, std::abs(denominator));
        irreversible = (1 - c) * difference / denominator;
    }
    return juce::jlimit(0.0, 4.0, (irreversible + c * derivative) / (1 - c * alpha * derivative));
}
// Alan değişimini en fazla 32 küçük adıma ayırır; orta nokta yöntemiyle hafızayı ilerletir.
// Tape ve Transformer farklı a/k katsayıları kullanır; ani otomasyonda sınırları koruyun.
double SaturationDSP::Magnet::process(double input, bool tape)
{
    const double h = juce::jlimit(-4.0, 4.0, input), delta = h - field;
    const int steps = juce::jlimit(1, 32, (int)std::ceil(std::abs(delta) / .12));
    const double increment = delta / steps, direction = delta >= 0 ? 1.0 : -1.0;
    const double a = tape ? .48 : .62, k = tape ? .11 : .18;
    // Explicit midpoint integration in field space; bounded increments prevent
    // an abrupt host automation change from destabilising the magnetic branch.
    for (int i = 0; i < steps; ++i)
    {
        // Manyetik integrasyon: başlangıç eğiminden yarım adım kestir; orta noktadaki eğimi tam adımda kullan. Bu döngü ses örneği başına maliyet taşır.
        const double first = slope(field, magnetisation, direction, a, k);
        const double middle = magnetisation + .5 * increment * first;
        magnetisation = juce::jlimit(
            -1.1, 1.1, magnetisation + increment * slope(field + .5 * increment, middle, direction, a, k));
        field += increment;
    }
    return magnetisation;
}
// Tek kanal/tek karakterin yüksek örnekleme hızındaki işleme adımı.
// Sıra: dinamik yükleme → ön şekillendirme → ilk ADAA → varsa manyetik kol → ikinci ADAA → Tape geri şekillendirme.
double SaturationDSP::Model::process(double input, int model, const std::array<double, 5> &rates,
                                     double userBias, double dynamics, double sag, bool adaa, double hardness, double harmonics)
{
    double x = input;
    // Dynamics ve ortak seviye zarfı, özellikle lamba modellerinde etkin sürüşü azaltarak sag karakteri verir.
    // Dinamik yükleme kazancı birimsizdir. Ortak stereo zarfı kullanılır; ancak model/kanalın diğer durumları birbirinden bağımsızdır.
    const double loading = 1 / (1 + dynamics * sag * (model == 4 ? .65 : model == 3 ? .42 : .13));
    x *= loading;
    double k = 1.0, b = .02, secondK = .72;
    // Transformer: kırpıcı katsayıları. Model numaraları modelNames sırasıyla eşleşir.
    if (model == 1)
    {
        k = 1.2;
        b = .065;
        secondK = .82;
    }
    // Tape: kayıt tarafında tiz ön vurgusu; çıkışta ters vurgu ile ton dengesi geri kurulur.
    if (model == 2)
    {
        const double low = follow(x, record, rates[0]);
        x = low + 1.6 * (x - low);
        k = .92;
        b = .025;
        secondK = .9;
    }
    // Triode: pozitif girişte grid yükü hafızası; farklı yüklenme/boşalma hızları asimetriyi etkiler.
    if (model == 3)
    {
        const double charge = juce::jmax(0.0, x - .25);
        follow(charge, grid, charge > grid ? rates[3] : rates[4]);
        k = 1.4;
        b = .24 + dynamics * .045 * juce::jmin(3.0, grid);
        secondK = .95;
    }
    // Pentode: daha sert ve varsayılan olarak simetrik kırpma; kullanıcı Bias ile asimetri ekleyebilir.
    if (model == 4)
    {
        k = 2.15;
        b = 0;
        secondK = 1.45;
    }
    // Warm daha yumuşak, Sizzle daha keskin kırpma eğimi sağlar; ikisi de 8×/ADAA yolundadır.
    k *= hardness;
    secondK *= hardness;
    const double evenBias = juce::jmax(0.0, -harmonics) * .20;
    // Biası sıfırın iki yanında aynı sürekli formülle ilerlet; nötrde koşullu limit koymak
    // otomasyon merkezinden geçerken eski yüksek-bias ayarlarında sıçrama yaratır.
    b += userBias * .32 + evenBias;
    // Birinci doğrusal olmayan katın sonucu ikinci kata gider. Bias/normalizasyon değişikliklerinde H2/H3 ve Auto Gain testlerini birlikte çalıştırın.
    double firstStage = first.process(x, k, b, adaa, harmonics);
    // Manyetik dal düşük seviyeli bir oranla kırpıcıya karıştırılır; yalnız iki manyetik karakterde çalışır.
    if (model == 1 || model == 2)
    {
        const double fieldInput = follow(input, bass, model == 1 ? rates[1] : rates[2]);
        const double m = magnetic.process(fieldInput, model == 2);
        const double fraction = model == 1 ? .14 : .09;
        firstStage = (1 - fraction) * firstStage + fraction * m * 1.7;
    }
    double result = second.process(firstStage, secondK, (model == 3 ? -.10 : 0) + evenBias, adaa, harmonics);
    if (model == 2)
    {
        const double lo = follow(result, playback, rates[0]);
        result = lo + (result - lo) / 1.6;
    }
    return result;
}
// Her model için 0..60 dB sürüşte 6 dB aralıklarla ölçülen telafiyi doğrusal enterpole eder.
// Drive/Heat veya model formülleri değişirse --calibrate ile tabloyu yeniden üretip test edin.
float SaturationDSP::compensationDb(int model, float db)
{
    // Calibrated offline on a -18 dBFS peak, 1 kHz sine. See measurement report.
    static constexpr float table[5][11] = {{.099f, -5.714f, -11.105f, -15.378f, -17.844f, -18.851f, -19.286f,
                                            -19.482f, -19.565f, -19.592f, -19.599f},
                                           {1.365f, -4.405f, -9.671f, -13.688f, -15.904f, -16.874f, -17.367f,
                                            -17.570f, -17.641f, -17.665f, -17.673f},
                                           {.180f, -5.766f, -11.206f, -15.321f, -17.562f, -18.444f, -18.773f,
                                            -18.891f, -18.934f, -18.947f, -18.950f},
                                           {.215f, -5.435f, -10.426f, -13.945f, -15.686f, -16.378f, -16.792f,
                                            -16.969f, -17.042f, -17.065f, -17.071f},
                                           {.420f, -4.835f, -8.918f, -11.253f, -12.231f, -12.614f, -12.896f,
                                            -13.030f, -13.081f, -13.096f, -13.100f}};
    // Tablo erişimi: sürüş 6 dB aralığına çevrilir; alt/üst komşu değer arasında interpolasyon yapılır. Bu LUFS ölçümü değildir.
    const float position = juce::jlimit(0.0f, 10.0f, db / 6);
    const int i = juce::jmin(9, (int)position);
    return juce::jmap(position - i, table[juce::jlimit(0, 4, model)][i],
                      table[juce::jlimit(0, 4, model)][i + 1]);
}
// Sadece ekranda çizilen statik yaklaşım. Buradaki katsayı değişikliği duyulan sesi değiştirmez.
float SaturationDSP::preview(float x, int model, float b, float character, float harmonics)
{
    // Yalnız statik önizleme: manyetik hafıza, sag ve zamanla değişen drift burada çizilmez.
    model = juce::jlimit(0, 4, model);
    const double k[]{1.0, 1.2, .92, 1.4, 2.15}, biases[]{.02, .065, .025, .24, 0};
    const double hardness = std::exp2(.65 * juce::jlimit(-1.0f, 1.0f, character));
    const double balance = juce::jlimit(-1.0f, 1.0f, harmonics), evenBias = juce::jmax(0.0, -balance) * .20;
    double offset = biases[model] + b * .32 + evenBias;
    const auto first = ADAA::balancedShape(x, k[model] * hardness, offset, balance);
    return (float)ADAA::balancedShape(first, (model == 4 ? 1.45 : .9) * hardness,
                                      (model == 3 ? -.1 : 0) + evenBias, balance);
}
// Gerçek zamanlı işlem başlamadan tamponları ve 8× FIR örneklemeyi kurar; burada bellek ayrılabilir.
void SaturationDSP::prepare(double sr, int n, int stages, bool useADAA)
{
    sampleRate = sr;
    capacity = juce::jmax(1, n);
    factor = 1 << stages;
    internalRate = sampleRate * factor;
    desk.prepare(sampleRate);
    antialias = useADAA;
    modelRates = {coefficient(3200, internalRate), coefficient(420, internalRate),
                  coefficient(6500, internalRate), coefficient(65, internalRate),
                  coefficient(4, internalRate)};
    // Hazırlıkta 2 iç kanal ve seçilen ikileme katları için FIR oversampler ayrılır. Callback içinde tekrar oluşturmayın.
    oversampling = std::make_unique<juce::dsp::Oversampling<float>>(
        2, (size_t)stages, juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple, true, true);
    oversampling->initProcessing((size_t)capacity);
    // İki ADAA katının toplam yaklaşık bir iç örnek gecikmesi de hesaba katılır.
    // Tam sayıya yuvarlamadan kalan fark wetFractional ile tamamlanır; kuru kol aynı latency kadar geciktirilir.
    const float extra = antialias ? 1.0f / factor : 0;
    // Gecikme tam sayıya yukarı yuvarlanır; wetFractional kalan farkı kapatır. Yalnız DAW’a bildirilen sayıyı değiştirmek dry/wet hizasını düzeltmez.
    latency = (int)std::ceil(oversampling->getLatencyInSamples() + extra);
    wetFractional.prepare({sampleRate, (juce::uint32)capacity, 2});
    wetFractional.setDelay((float)latency - oversampling->getLatencyInSamples() - extra);
    // prepare ile ayrılan kapasite içinde yeniden kullanılır. Mono giriş iki iç kanala çoğaltılır.
    // Geçersiz sayılar sıfırlanır; aşırı girişler ±64 ile sınırlandırılır.
    wet.setSize(2, capacity);
    dryDelay.setSize(2, latency + capacity + 8);
    // Kontrol rampaları 25 ms, karakter geçişleri 20 ms sürer; süreler iç örnekleme hızına göredir.
    for (auto *smoother :
         {&drive, &tone, &lowCut, &highCut, &bias, &dynamics, &mix, &output, &width, &autoLevel, &bypass, &character, &harmonics, &drift})
        smoother->reset(internalRate, .025);
    for (auto &weight : weights)
        weight.reset(internalRate, .02);
    reset();
}
// Yeni başlangıç için model/filtre/gecikme durumlarını ve sayaçları temizler; parametre tanımlarını değiştirmez.
void SaturationDSP::reset()
{
    // Reset yalnız filtre/model/gecikme hafızasını temizler. Parametre kimliklerini veya preset tanımlarını yeniden oluşturmaz.
    models = {};
    desk.reset();
    highpass = {};
    highpassInput = {};
    lowpass = {};
    toneLow = {};
    dcInput = {};
    dcOutput = {};
    envelope = 0;
    driftPhase = driftPhase2 = 0;
    previousCharacter = 2;
    characterHardness = 1;
    dryPosition = 0;
    primed = false;
    previousLow = -1;
    previousHigh = -1;
    previousTone = 2;
    if (oversampling)
        oversampling->reset();
    wetFractional.reset();
    dryDelay.clear();
    for (auto &weight : weights)
        weight.setCurrentAndTargetValue(0);
    inputPeak = 0;
    outputPeak = 0;
    activity = 0;
}
// Host blok büyüklüğü prepare kapasitesinden büyük olsa da render alt bloklarla güvenli çalışır.
void SaturationDSP::process(juce::AudioBuffer<float> &audio, const Settings &s)
{
    // Hazırlanmamış motorda çıkışa dokunmadan dönülür. Sessiz çıkış araştırırken prepareToPlay çağrısını ve kapasiteyi kontrol edin.
    if (!oversampling || capacity == 0)
        return;
    for (int offset = 0; offset < audio.getNumSamples(); offset += capacity)
        render(audio, offset, juce::jmin(capacity, audio.getNumSamples() - offset), s);
}
// Bir alt bloğun tam işleme zinciri. Yeni dosya erişimi, kilit veya GUI işlemini buraya eklemeyin.
void SaturationDSP::render(juce::AudioBuffer<float> &audio, int offset, int count, const Settings &settings)
{
    // Alt blok aşama 1: seçili modeli ve toplam sürüşü sınırla; hedef rampalar bu alt bloğun başında hazırlanır.
    const int selected = juce::jlimit(0, 4, settings.model);
    // Heat ek sürüştür; Auto Gain açık olduğunda telafi toplam Drive üzerinden hesaplanır.
    const float totalDrive = settings.drive + (settings.heat ? 20 : 0);
    const float compensation = settings.autoGain ? compensationDb(selected, totalDrive) : 0;
    // İlk blokta başlangıç sıçramasını önler; sonraki bloklarda hedefe yumuşak geçiş başlatır.
    auto target = [&](auto &v, float value)
    {
        if (!primed)
            v.setCurrentAndTargetValue(value);
        else
            v.setTargetValue(value);
    };
    target(drive, juce::Decibels::decibelsToGain(totalDrive));
    target(tone, settings.tone);
    target(lowCut, settings.lowCut);
    target(highCut, settings.highCut);
    target(bias, settings.bias);
    target(dynamics, settings.dynamics);
    target(mix, settings.mix);
    target(output, juce::Decibels::decibelsToGain(settings.output));
    target(width, settings.width);
    target(autoLevel, juce::Decibels::decibelsToGain(compensation));
    target(bypass, settings.bypass ? 1.0f : 0.0f);
    target(character, juce::jlimit(-1.0f, 1.0f, settings.character));
    target(harmonics, juce::jlimit(-1.0f, 1.0f, settings.harmonics));
    target(drift, juce::jlimit(0.0f, 1.0f, settings.drift));
    if (!primed)
    {
        weights[(size_t)selected].setCurrentAndTargetValue(1);
        lastModel = selected;
    }
    // Eski ve yeni modeller birlikte soldurulur. Önceden tamamen kapanmış modelin hafızası yeniden temizlenir.
    // Karakter geçişinde yeni modelin tamamen kapalı hafızası sıfırlanır; ağırlıklar 20 ms crossfade ile ilerler. Ani model kopyalama yapmayın.
    if (lastModel != selected)
    {
        if (weights[(size_t)selected].getCurrentValue() < 1e-5f)
            models[(size_t)selected] = {};
        for (int m = 0; m < 5; ++m)
            weights[(size_t)m].setTargetValue(m == selected ? 1.0f : 0.0f);
        lastModel = selected;
    }
    desk.configure(settings.transient, settings.comp, settings.bass, settings.treble,
                   settings.fastAttack, settings.fastRelease, settings.bassTight, settings.trebleTight);
    primed = true;
    // Alt blok aşama 2: prepare’da ayrılan belleği yeniden kullanır; avoidReallocating=true ve count<=capacity koşulu birlikte korunmalıdır.
    wet.setSize(2, count, false, false, true);
    float inputMaximum = 0;
    for (int c = 0; c < 2; ++c)
        for (int i = 0; i < count; ++i)
        {
            const float x = audio.getSample(juce::jmin(c, audio.getNumChannels() - 1), offset + i);
            // Giriş koruması: NaN/Inf sıfırlanır, sonlu giriş ±64’e sınırlandırılır. Bu koruma normal çıkışta limiter bulunduğu anlamına gelmez.
            const float clean = std::isfinite(x) ? juce::jlimit(-64.0f, 64.0f, x) : 0;
            wet.setSample(c, i, clean);
            inputMaximum = juce::jmax(inputMaximum, std::abs(clean));
        }
    // Fazıl Irak: stereo bağlı transient katı model sürüşünden önce, host hızında çalışır.
    // Ek gecikme/lookahead yoktur; eski dry/bypass hizası aynen kalır.
    for (int i = 0; i < count; ++i)
    {
        float l = wet.getSample(0, i), r = wet.getSample(1, i);
        desk.processInput(l, r);
        wet.setSample(0, i, l); wet.setSample(1, i, r);
    }
    // Önce örnekleme artırılır; aşağıdaki filtre ve model işlemleri internalRate hızında uygulanır.
    juce::dsp::AudioBlock<float> block(wet);
    auto up = oversampling->processSamplesUp(block);
    // İç hızdaki katsayılar host hızında hesaplanmamalıdır; oversampling faktörü değişince internalRate ve gecikme testleri birlikte değişir.
    const double dcPole = std::exp(-2 * pi * 10 / internalRate),
                 toneCoefficient = coefficient(900, internalRate);
    const double attackPole = coefficient(60, internalRate), releasePole = coefficient(1.8, internalRate);
    float maximumActivity = 0;
    // Mix/output control values are sampled at the host rate after downsampling;
    // skip through their equivalent high-rate ramp here to keep block invariance.
    // Örnek başına yumuşatılmış kontrolleri al; filtre katsayılarını sadece değer değiştiğinde hesapla.
    for (size_t i = 0; i < up.getNumSamples(); ++i)
    {
        const float gain = drive.getNextValue(), tilt = tone.getNextValue(), cutL = lowCut.getNextValue(),
                    cutH = highCut.getNextValue(), b = bias.getNextValue(), dyn = dynamics.getNextValue();
        const float characterValue = character.getNextValue(), balance = harmonics.getNextValue(),
                    driftAmount = drift.getNextValue();
        if (characterValue != previousCharacter)
        {
            characterHardness = std::exp2(.65 * characterValue);
            previousCharacter = characterValue;
        }
        // 0.17 ve 0.113 Hz: periyodu birbirinden farklı, stereo bağlantılı küçük tolerans değişimi.
        // Aşırı drive/pitch modülasyonu değildir; max kazanç sapması yaklaşık ±0.3 dB'dir.
        const double wander = driftAmount * (.65 * std::sin(driftPhase) + .35 * std::sin(driftPhase2));
        const double driftGain = 1 + .035 * wander;
        const double driftingBias = b + .012 * driftAmount * std::sin(driftPhase2);
        driftPhase += 2 * pi * .17 / internalRate;
        driftPhase2 += 2 * pi * .113 / internalRate;
        if (driftPhase >= 2 * pi) driftPhase -= 2 * pi;
        if (driftPhase2 >= 2 * pi) driftPhase2 -= 2 * pi;
        if (cutL != previousLow)
        {
            highpassPole = std::exp(-2 * pi * cutL / internalRate);
            previousLow = cutL;
        }
        if (cutH != previousHigh)
        {
            lowpassAmount = coefficient(cutH, internalRate);
            previousHigh = cutH;
        }
        if (tilt != previousTone)
        {
            toneBassGain = std::pow(10.0, -tilt * 4 / 20);
            toneTrebleGain = 1 / toneBassGain;
            previousTone = tilt;
        }
        // Sol/sağ kanalların en yüksek seviyesi ortak Dynamics zarfını besler; attack ve release ayrıdır.
        const double level = juce::jmin(
            4.0f, juce::jmax(std::abs(up.getSample(0, i) * gain), std::abs(up.getSample(1, i) * gain)));
        follow(level, envelope, level > envelope ? attackPole : releasePole);
        // Ağırlık rampası her iç örnekte bir kez ilerletilir; kanal döngüsüne taşımak stereo kanallara farklı model ağırlığı uygular.
        std::array<float, 5> blend{};
        for (int m = 0; m < 5; ++m)
            blend[(size_t)m] = weights[(size_t)m].getNextValue();
        for (int c = 0; c < 2; ++c)
        {
            double x = up.getSample(c, i);
            // Low Cut kırpmadan önce çalışır. 20 Hz uç değerde ses yoluna uygulanmaz fakat hafıza güncellenir.
            const double filtered = highpassPole * (highpass[(size_t)c] + x - highpassInput[(size_t)c]);
            highpass[(size_t)c] = filtered;
            highpassInput[(size_t)c] = x;
            if (cutL > 20.01f)
                x = filtered;
            double y = 0;
            for (int m = 0; m < 5; ++m)
                if (blend[(size_t)m] > 0)
                    y += blend[(size_t)m] * models[(size_t)m][(size_t)c].process(x * gain * driftGain, m, modelRates, driftingBias,
                                                                                 dyn, envelope, antialias, characterHardness, balance);
            // Tone: 900 Hz civarında ayrılan bas/tiz bileşenlerine karşılıklı kazanç uygular.
            const double lo = follow(y, toneLow[(size_t)c], toneCoefficient);
            y = lo * toneBassGain + (y - lo) * toneTrebleGain;
            // High Cut kırpmadan sonra çalışır. 20 kHz uç değerde filtre ses yoluna uygulanmaz.
            const double highFiltered = follow(y, lowpass[(size_t)c], lowpassAmount);
            if (cutH < 19999)
                y = highFiltered;
            // 10 Hz DC engelleyici asimetrik doygunluğun ürettiği sabit bileşeni azaltır.
            const double clean = y - dcInput[(size_t)c] + dcPole * dcOutput[(size_t)c];
            dcInput[(size_t)c] = y;
            dcOutput[(size_t)c] = clean;
            up.setSample(c, i, (float)clean);
        }
        maximumActivity = juce::jmax(maximumActivity, (float)envelope);
    }
    // Host örnekleme hızına geri dön; bundan sonra karışım, genişlik ve çıkış seviyesi uygulanır.
    oversampling->processSamplesDown(block);
    float peak = 0;
    for (int i = 0; i < count; ++i)
    {
        // Yüksek hızda tanımlı rampaları factor kadar ilerletir; farklı blok boylarında tutarlılık korunur.
        // Alt blok aşama 3: host hızındaki karışım kontrollerini factor iç örnek kadar ilerletir; blok boyundan bağımsız davranış buna dayanır.
        const float amount = mix.skip(factor), level = output.skip(factor), stereo = width.skip(factor),
                    makeup = autoLevel.skip(factor), pass = bypass.skip(factor);
        std::array<float, 2> delayed{}, processed{};
        for (int c = 0; c < 2; ++c)
        {
            const float original = audio.getSample(juce::jmin(c, audio.getNumChannels() - 1), offset + i);
            const float clean = std::isfinite(original) ? juce::jlimit(-64.0f, 64.0f, original) : 0;
            // Kuru sinyali dairesel tamponda latency örnek geciktirir; dry/wet karışımında faz hizası gerekir.
            dryDelay.setSample(c, dryPosition, clean);
            delayed[(size_t)c] = dryDelay.getSample(c, (dryPosition + dryDelay.getNumSamples() - latency) %
                                                           dryDelay.getNumSamples());
            wetFractional.pushSample(c, wet.getSample(c, i));
            processed[(size_t)c] = wetFractional.popSample(c) * makeup;
        }
        // Comp ve iki bant EQ yalnız işlenmiş kola uygulanır. Mix=0 ve Bypass etkilenmez.
        desk.processOutput(processed);
        // Mid/Side genişlik: 0 mono, 1 mevcut genişlik, 2 genişletilmiş wet sinyal. Kuru kol etkilenmez.
        const float mid = (processed[0] + processed[1]) * .5f,
                    side = (processed[0] - processed[1]) * .5f * stereo;
        processed = {mid + side, mid - side};
        for (int c = 0; c < audio.getNumChannels() && c < 2; ++c)
        {
            // Önce dry/wet, sonra Output uygulanır. Bypass geçişi hizalanmış kuru sese döner; Output’u da atlar.
            float value = (delayed[(size_t)c] + amount * (processed[(size_t)c] - delayed[(size_t)c])) * level;
            value += pass * (delayed[(size_t)c] - value);
            if (!std::isfinite(value))
                value = 0;
            audio.setSample(c, offset + i, value);
            peak = juce::jmax(peak, std::abs(value));
        }
        // Dairesel dry tamponu iki kanal işlendiğinde yalnız bir kez ilerletilir; kanal başına ilerletmek zaman hizasını bozar.
        dryPosition = (dryPosition + 1) % dryDelay.getNumSamples();
    }
    // Son blok tepe değerlerini atomik olarak arayüze aktarır; bunlar ses örneklerini değiştirmez.
    inputPeak.store(inputMaximum, std::memory_order_relaxed);
    outputPeak.store(peak, std::memory_order_relaxed);
    activity.store(maximumActivity, std::memory_order_relaxed);
}
} // namespace irak
