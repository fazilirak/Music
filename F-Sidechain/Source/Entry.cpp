// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 Fazıl Irak
// Fazıl Irak: VST3 ve bağımsız uygulama için üç ayrı Volume motorunu aynı bant işlemcisine bağlar.
#include "MultibandProcessor.h"
#include "Volume/PluginProcessor.h"
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MultibandProcessor([] { return new volumef::VOLUMEFAudioProcessor(); }, 5);
}
