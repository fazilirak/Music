// Geliştirici: Fazıl Irak - Son düzenleme: 27.10.2026
// Dosyanın görevi: Envelope düzenleme ve ürün kontrol bileşeni; özgün kaynak atıfları korunur.
// Katkı kapsamı: F-Sidechain bakımı ve uyarlamaları. GATE-12/Shaper özgün telifleri korunur.
// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 Fazıl Irak
// Fazıl Irak: VST3 ve bağımsız uygulama için üç ayrı Volume motorunu aynı bant işlemcisine bağlar.
#include "MultibandProcessor.h"
#include "Volume/PluginProcessor.h"
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MultibandProcessor([] { return new volumef::VOLUMEFAudioProcessor(); }, 5);
}
