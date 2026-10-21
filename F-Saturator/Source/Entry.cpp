// JUCE, VST3 ve bağımsız uygulama sarmalayıcılarından bu giriş noktasını çağırır.
// Her çağrı yeni bir Processor üretir; ses işleme kodu PluginProcessor ve SaturationDSP içindedir.
#include "PluginProcessor.h"
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new irak::Processor(); }
