// SPDX-License-Identifier: GPL-3.0-only
// Fazıl Irak · 12.11.2026: A/B sırasında kapanmış editöre kuyruktan erişilmesini önler.
#pragma once
#include <JuceHeader.h>
namespace shaperui {
template<class T> void postIfAlive(T* component,std::function<void()> callback) {
    juce::Component::SafePointer<T> safe(component);
    juce::MessageManager::callAsync([safe,callback=std::move(callback)] {if(safe)callback();});
}
}
