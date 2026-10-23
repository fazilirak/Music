// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (c) 2026 Fazıl Irak
// Geliştirici: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: Host sarmalayıcılarının yeni Processor oluşturmak için çağırdığı JUCE giriş noktası.
// JUCE, VST3 ve bağımsız uygulama sarmalayıcılarından bu giriş noktasını çağırır.
// Her çağrı yeni bir Processor üretir; ses işleme kodu PluginProcessor ve SaturationDSP içindedir.
#include "PluginProcessor.h"
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new irak::Processor();
}
