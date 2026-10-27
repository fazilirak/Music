// Geliştirici: Fazıl Irak - 28.10.2026
// SPDX-License-Identifier: AGPL-3.0-or-later
// Görev: Eski Windows referans WAV'larını platformlar arası örnek düzeyinde doğrulamak.
#pragma once
#include <juce_audio_formats/juce_audio_formats.h>
#include <iostream>
#include <iomanip>
namespace fkickTest {
inline bool sameLegacyWav(const juce::File& reference, const juce::File& actual) {
#if JUCE_WINDOWS
    // Mevcut Windows regresyon sözleşmesi byte-byte eşit kalır.
    juce::MemoryBlock a,b;
    return reference.loadFileAsData(a)&&actual.loadFileAsData(b)&&a==b;
#else
    juce::AudioFormatManager formats;formats.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> a(formats.createReaderFor(reference)),b(formats.createReaderFor(actual));
    if(!a||!b||a->sampleRate!=b->sampleRate||a->numChannels!=b->numChannels||a->bitsPerSample!=24||b->bitsPerSample!=24||a->lengthInSamples!=b->lengthInSamples||a->lengthInSamples>2147483647) {
        std::cerr<<"Legacy WAV metadata mismatch: "<<reference.getFileName()<<'\n';return false;
    }
    const int count=int(a->lengthInSamples),channels=int(a->numChannels);
    juce::AudioBuffer<float> left(channels,count),right(channels,count);
    if(!a->read(&left,0,count,0,true,true)||!b->read(&right,0,count,0,true,true))return false;
    double worst=0,energy=0;int changed=0;
    for(int c=0;c<channels;++c)for(int i=0;i<count;++i){const double error=std::abs(double(left.getSample(c,i))-right.getSample(c,i));worst=juce::jmax(worst,error);energy+=error*error;changed+=error!=0;}
    // İki 24-bit PCM basamağı: libm/derleyici ve son PCM yuvarlaması için dar sınır.
    // Seviye, zamanlama ve bütün örnekler karşılaştırılır; ses testi atlanmaz.
    constexpr double pcmStep=1.0/8388608.0;
    std::cout<<std::setprecision(12)<<"Legacy PCM "<<reference.getFileName()<<": max="<<worst/pcmStep<<" LSB, RMS="<<std::sqrt(energy/(channels*count))/pcmStep<<" LSB, differing="<<changed<<'\n';
    return worst<=2*pcmStep;
#endif
}
}
