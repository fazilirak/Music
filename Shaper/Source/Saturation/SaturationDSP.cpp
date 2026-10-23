// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 Fazıl Irak; Shaper eklemeleri/değişiklikleri.
// Fazıl Irak tarafından oluşturuldu / değiştirildi: 23.10.2026
// Kapsam: Shaper için eklenen kod ve uyarlamalar; özgün üçüncü taraf telif/lisansları korunur.
// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: Doygunluk karakterleri, oversampling, ADAA, dinamik hafıza, filtre ve hizalı dry/wet zincirini uygular.
#include "SaturationDSP.h"
#include <cmath>

namespace irak
{
namespace
{
constexpr double pi = juce::MathConstants<double>::pi;
// Örnekleme hızı ve frekanstan filtre/zarf katsayısını hesaplar; Hz ile örnek başına katsayı farklı birimdir.
double coefficient(double hz, double sr)
{
    return 1 - std::exp(-2 * pi * hz / sr);
}
// follow alt adımı: Doygunluk karakterleri, oversampling, ADAA, dinamik hafıza, filtre ve hizalı dry/wet zincirini uygular.
double follow(double input, double &state, double amount)
{
    state += amount * (input - state);
    return state;
}
} // namespace
// logCosh alt adımı: Doygunluk karakterleri, oversampling, ADAA, dinamik hafıza, filtre ve hizalı dry/wet zincirini uygular.
double SaturationDSP::ADAA::logCosh(double x)
{
    const auto a = std::abs(x);
    return a + std::log1p(std::exp(-2 * a)) - std::log(2.0);
}
// shape alt adımı: Doygunluk karakterleri, oversampling, ADAA, dinamik hafıza, filtre ve hizalı dry/wet zincirini uygular.
double SaturationDSP::ADAA::shape(double x, double k, double b)
{
    const double offset = std::tanh(k * b), normal = k * (1 - offset * offset);
    return (std::tanh(k * (x + b)) - offset) / normal;
}
// İlgili giriş verisini bu bileşenin mevcut durumuyla işler; alt çağrılardaki birim ve kanal sınırlarını koruyun.
// Gövdedeki başlıca çağrılar: shape → std::abs → std::tanh → logCosh. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
double SaturationDSP::ADAA::process(double x, double k, double b, bool enabled)
{
    const double last = previous;
    previous = x;
    if (!enabled)
        return shape(x, k, b);
    const double dx = x - last;
    if (std::abs(dx) < 1e-5 * (1 + std::abs(x) + std::abs(last)))
        return shape((x + last) * .5, k, b);
    const double offset = std::tanh(k * b), normal = k * (1 - offset * offset);
    const auto primitive = [&](double v) { return (logCosh(k * (v + b)) / k - offset * v) / normal; };
    return (primitive(x) - primitive(last)) / dx;
}
// langevin alt adımı: Doygunluk karakterleri, oversampling, ADAA, dinamik hafıza, filtre ve hizalı dry/wet zincirini uygular.
double SaturationDSP::Magnet::langevin(double x)
{
    if (std::abs(x) < .05)
    {
        const double x2 = x * x;
        return x * (1.0 / 3 - x2 / 45 + 2 * x2 * x2 / 945);
    }
    return 1 / std::tanh(x) - 1 / x;
}
// langevinDerivative alt adımı: Doygunluk karakterleri, oversampling, ADAA, dinamik hafıza, filtre ve hizalı dry/wet zincirini uygular.
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
// slope alt adımı: Doygunluk karakterleri, oversampling, ADAA, dinamik hafıza, filtre ve hizalı dry/wet zincirini uygular.
// Gövdedeki başlıca çağrılar: langevin → langevinDerivative → juce::jmax → std::abs. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
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
// İlgili giriş verisini bu bileşenin mevcut durumuyla işler; alt çağrılardaki birim ve kanal sınırlarını koruyun.
// Gövdedeki başlıca çağrılar: juce::jlimit → std::ceil → std::abs → slope. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
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
        const double first = slope(field, magnetisation, direction, a, k);
        const double middle = magnetisation + .5 * increment * first;
        magnetisation = juce::jlimit(
            -1.1, 1.1, magnetisation + increment * slope(field + .5 * increment, middle, direction, a, k));
        field += increment;
    }
    return magnetisation;
}
// İlgili giriş verisini bu bileşenin mevcut durumuyla işler; alt çağrılardaki birim ve kanal sınırlarını koruyun.
// Gövdedeki başlıca çağrılar: follow → juce::jmax → juce::jmin → first.process. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
double SaturationDSP::Model::process(double input, int model, const std::array<double, 5> &rates,
                                     double userBias, double dynamics, double sag, bool adaa)
{
    double x = input;
    const double loading = 1 / (1 + dynamics * sag * (model == 4 ? .65 : model == 3 ? .42 : .13));
    x *= loading;
    double k = 1.0, b = .02, secondK = .72;
    if (model == 1)
    {
        k = 1.2;
        b = .065;
        secondK = .82;
    }
    if (model == 2)
    {
        const double low = follow(x, record, rates[0]);
        x = low + 1.6 * (x - low);
        k = .92;
        b = .025;
        secondK = .9;
    }
    if (model == 3)
    {
        const double charge = juce::jmax(0.0, x - .25);
        follow(charge, grid, charge > grid ? rates[3] : rates[4]);
        k = 1.4;
        b = .24 + dynamics * .045 * juce::jmin(3.0, grid);
        secondK = .95;
    }
    if (model == 4)
    {
        k = 2.15;
        b = 0;
        secondK = 1.45;
    }
    b += userBias * .32;
    double firstStage = first.process(x, k, b, adaa);
    if (model == 1 || model == 2)
    {
        const double fieldInput = follow(input, bass, model == 1 ? rates[1] : rates[2]);
        const double m = magnetic.process(fieldInput, model == 2);
        const double fraction = model == 1 ? .14 : .09;
        firstStage = (1 - fraction) * firstStage + fraction * m * 1.7;
    }
    double result = second.process(firstStage, secondK, model == 3 ? -.10 : 0, adaa);
    if (model == 2)
    {
        const double lo = follow(result, playback, rates[0]);
        result = lo + (result - lo) / 1.6;
    }
    return result;
}
// compensationDb alt adımı: Doygunluk karakterleri, oversampling, ADAA, dinamik hafıza, filtre ve hizalı dry/wet zincirini uygular.
// Gövdedeki başlıca çağrılar: juce::jlimit → juce::jmin → juce::jmap. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
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
    const float position = juce::jlimit(0.0f, 10.0f, db / 6);
    const int i = juce::jmin(9, (int)position);
    return juce::jmap(position - i, table[juce::jlimit(0, 4, model)][i],
                      table[juce::jlimit(0, 4, model)][i + 1]);
}
// preview alt adımı: Doygunluk karakterleri, oversampling, ADAA, dinamik hafıza, filtre ve hizalı dry/wet zincirini uygular.
float SaturationDSP::preview(float x, int model, float b)
{
    const double k[]{1.0, 1.2, .92, 1.4, 2.15}, biases[]{.02, .065, .025, .24, 0};
    return (float)ADAA::shape(ADAA::shape(x, k[model], biases[model] + b * .32), model == 4 ? 1.45 : .9,
                              model == 3 ? -.1 : 0);
}
// Örnekleme hızı ve blok kapasitesine bağlı tampon/işleme durumunu ses başlamadan hazırlar.
// Gövdedeki başlıca çağrılar: juce::jmax → coefficient → oversampling->initProcessing → std::ceil. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
void SaturationDSP::prepare(double sr, int n, int stages, bool useADAA)
{
    sampleRate = sr;
    capacity = juce::jmax(1, n);
    factor = 1 << stages;
    internalRate = sampleRate * factor;
    antialias = useADAA;
    modelRates = {coefficient(3200, internalRate), coefficient(420, internalRate),
                  coefficient(6500, internalRate), coefficient(65, internalRate),
                  coefficient(4, internalRate)};
    oversampling = std::make_unique<juce::dsp::Oversampling<float>>(
        2, (size_t)stages, juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple, true, true);
    oversampling->initProcessing((size_t)capacity);
    const float extra = antialias ? 1.0f / factor : 0;
    latency = (int)std::ceil(oversampling->getLatencyInSamples() + extra);
    wetFractional.prepare({sampleRate, (juce::uint32)capacity, 2});
    wetFractional.setDelay((float)latency - oversampling->getLatencyInSamples() - extra);
    wet.setSize(2, capacity);
    dryDelay.setSize(2, latency + capacity + 8);
    for (auto *smoother :
         {&drive, &tone, &lowCut, &highCut, &bias, &dynamics, &mix, &output, &width, &autoLevel, &bypass})
        smoother->reset(internalRate, .025);
    for (auto &weight : weights)
        weight.reset(internalRate, .02);
    reset();
}
// Örnek/filtre/faz gibi çalışma hafızasını yeniden başlangıca alır; parametre tanımıyla aynı işlem değildir.
// Gövdedeki başlıca çağrılar: oversampling->reset → wetFractional.reset → dryDelay.clear → weight.setCurrentAndTargetValue. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
void SaturationDSP::reset()
{
    models = {};
    highpass = {};
    highpassInput = {};
    lowpass = {};
    toneLow = {};
    dcInput = {};
    dcOutput = {};
    envelope = 0;
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
// İlgili giriş verisini bu bileşenin mevcut durumuyla işler; alt çağrılardaki birim ve kanal sınırlarını koruyun.
void SaturationDSP::process(juce::AudioBuffer<float> &audio, const Settings &s)
{
    if (!oversampling || capacity == 0)
        return;
    for (int offset = 0; offset < audio.getNumSamples(); offset += capacity)
        render(audio, offset, juce::jmin(capacity, audio.getNumSamples() - offset), s);
}
// processModulated alt adımı: Doygunluk karakterleri, oversampling, ADAA, dinamik hafıza, filtre ve hizalı dry/wet zincirini uygular.
void SaturationDSP::processModulated(juce::AudioBuffer<float> &audio, const Settings &s,
                                     const float *envelope, float depth)
{
    if (!oversampling || capacity == 0)
        return;
    for (int offset = 0; offset < audio.getNumSamples(); offset += capacity)
        render(audio, offset, juce::jmin(capacity, audio.getNumSamples() - offset), s, envelope, depth);
}
// Hazırlanmış durumdan ses veya panel çıktısını üretir; bu dosyanın DSP/GUI bağlamını aşağıdaki gövdeden ayırın.
// Gövdedeki başlıca çağrılar: juce::jlimit → compensationDb → v.setCurrentAndTargetValue → v.setTargetValue. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
void SaturationDSP::render(juce::AudioBuffer<float> &audio, int offset, int count, const Settings &settings,
                           const float *modulation, float depth)
{
    const int selected = juce::jlimit(0, 4, settings.model);
    const float totalDrive = settings.drive + (settings.heat ? 20 : 0);
    const float compensation = settings.autoGain ? compensationDb(selected, totalDrive) : 0;
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
    if (!primed)
    {
        weights[(size_t)selected].setCurrentAndTargetValue(1);
        lastModel = selected;
    }
    if (lastModel != selected)
    {
        if (weights[(size_t)selected].getCurrentValue() < 1e-5f)
            models[(size_t)selected] = {};
        for (int m = 0; m < 5; ++m)
            weights[(size_t)m].setTargetValue(m == selected ? 1.0f : 0.0f);
        lastModel = selected;
    }
    primed = true;
    wet.setSize(2, count, false, false, true);
    float inputMaximum = 0;
    for (int c = 0; c < 2; ++c)
        for (int i = 0; i < count; ++i)
        {
            const float x = audio.getSample(juce::jmin(c, audio.getNumChannels() - 1), offset + i);
            const float clean = std::isfinite(x) ? juce::jlimit(-64.0f, 64.0f, x) : 0;
            wet.setSample(c, i, clean);
            inputMaximum = juce::jmax(inputMaximum, std::abs(clean));
        }
    juce::dsp::AudioBlock<float> block(wet);
    auto up = oversampling->processSamplesUp(block);
    const double dcPole = std::exp(-2 * pi * 10 / internalRate),
                 toneCoefficient = coefficient(900, internalRate);
    const double attackPole = coefficient(60, internalRate), releasePole = coefficient(1.8, internalRate);
    float maximumActivity = 0;
    // Mix/output control values are sampled at the host rate after downsampling;
    // skip through their equivalent high-rate ramp here to keep block invariance.
    for (size_t i = 0; i < up.getNumSamples(); ++i)
    {
        const float baseGain = drive.getNextValue(), tilt = tone.getNextValue(), cutL = lowCut.getNextValue(),
                    cutH = highCut.getNextValue(), b = bias.getNextValue(), dyn = dynamics.getNextValue();
        const float cv = modulation ? juce::jlimit(0.0f, 1.0f, modulation[offset + (int)i / factor]) : 1.0f;
        const float modDrive = totalDrive * (1 - depth + depth * cv);
        const float gain = modulation ? juce::Decibels::decibelsToGain(modDrive) : baseGain;
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
        const double level = juce::jmin(
            4.0f, juce::jmax(std::abs(up.getSample(0, i) * gain), std::abs(up.getSample(1, i) * gain)));
        follow(level, envelope, level > envelope ? attackPole : releasePole);
        std::array<float, 5> blend{};
        for (int m = 0; m < 5; ++m)
            blend[(size_t)m] = weights[(size_t)m].getNextValue();
        for (int c = 0; c < 2; ++c)
        {
            double x = up.getSample(c, i);
            const double filtered = highpassPole * (highpass[(size_t)c] + x - highpassInput[(size_t)c]);
            highpass[(size_t)c] = filtered;
            highpassInput[(size_t)c] = x;
            if (cutL > 20.01f)
                x = filtered;
            double y = 0;
            for (int m = 0; m < 5; ++m)
                if (blend[(size_t)m] > 0)
                    y += blend[(size_t)m] * models[(size_t)m][(size_t)c].process(x * gain, m, modelRates, b,
                                                                                 dyn, envelope, antialias);
            const double lo = follow(y, toneLow[(size_t)c], toneCoefficient);
            y = lo * toneBassGain + (y - lo) * toneTrebleGain;
            const double highFiltered = follow(y, lowpass[(size_t)c], lowpassAmount);
            if (cutH < 19999)
                y = highFiltered;
            const double clean = y - dcInput[(size_t)c] + dcPole * dcOutput[(size_t)c];
            dcInput[(size_t)c] = y;
            dcOutput[(size_t)c] = clean;
            up.setSample(c, i, (float)clean);
        }
        maximumActivity = juce::jmax(maximumActivity, (float)envelope);
    }
    oversampling->processSamplesDown(block);
    float peak = 0;
    for (int i = 0; i < count; ++i)
    {
        const float amount = mix.skip(factor), level = output.skip(factor), stereo = width.skip(factor),
                    makeup = autoLevel.skip(factor), pass = bypass.skip(factor);
        std::array<float, 2> delayed{}, processed{};
        for (int c = 0; c < 2; ++c)
        {
            const float original = audio.getSample(juce::jmin(c, audio.getNumChannels() - 1), offset + i);
            const float clean = std::isfinite(original) ? juce::jlimit(-64.0f, 64.0f, original) : 0;
            dryDelay.setSample(c, dryPosition, clean);
            delayed[(size_t)c] = dryDelay.getSample(c, (dryPosition + dryDelay.getNumSamples() - latency) %
                                                           dryDelay.getNumSamples());
            wetFractional.pushSample(c, wet.getSample(c, i));
            const float cv = modulation ? juce::jlimit(0.0f, 1.0f, modulation[offset + i]) : 1.0f;
            const float modMakeup = modulation && settings.autoGain
                                        ? juce::Decibels::decibelsToGain(
                                              compensationDb(selected, totalDrive * (1 - depth + depth * cv)))
                                        : makeup;
            processed[(size_t)c] = wetFractional.popSample(c) * modMakeup;
        }
        const float mid = (processed[0] + processed[1]) * .5f,
                    side = (processed[0] - processed[1]) * .5f * stereo;
        processed = {mid + side, mid - side};
        for (int c = 0; c < audio.getNumChannels() && c < 2; ++c)
        {
            float value = (delayed[(size_t)c] + amount * (processed[(size_t)c] - delayed[(size_t)c])) * level;
            value += pass * (delayed[(size_t)c] - value);
            if (!std::isfinite(value))
                value = 0;
            audio.setSample(c, offset + i, value);
            peak = juce::jmax(peak, std::abs(value));
        }
        dryPosition = (dryPosition + 1) % dryDelay.getNumSamples();
    }
    inputPeak.store(inputMaximum, std::memory_order_relaxed);
    outputPeak.store(peak, std::memory_order_relaxed);
    activity.store(maximumActivity, std::memory_order_relaxed);
}
} // namespace irak
