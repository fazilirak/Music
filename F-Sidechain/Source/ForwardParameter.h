// Geliştirici: Fazıl Irak - Son düzenleme: 27.10.2026
// Dosyanın görevi: Envelope düzenleme ve ürün kontrol bileşeni; özgün kaynak atıfları korunur.
// Katkı kapsamı: F-Sidechain bakımı ve uyarlamaları. GATE-12/Shaper özgün telifleri korunur.
// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 Fazıl Irak; Shaper eklemeleri/değişiklikleri.
// Fazıl Irak tarafından oluşturuldu / değiştirildi: 23.10.2026
// Kapsam: Shaper için eklenen kod ve uyarlamalar; özgün üçüncü taraf telif/lisansları korunur.
// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: İç efekt parametresini dış host parametresine yönlendirir; host gesture ve değer ölçeklerini korur.
#pragma once
#include <JuceHeader.h>
class ForwardParameter final : public juce::RangedAudioParameter,
                               private juce::AudioProcessorParameter::Listener
{
    juce::RangedAudioParameter &child;
    bool attached = true;

  public:
    // ForwardParameter alt adımı: İç efekt parametresini dış host parametresine yönlendirir; host gesture ve değer ölçeklerini korur.
    ForwardParameter(juce::String id, juce::String name, juce::RangedAudioParameter &p)
        : RangedAudioParameter(juce::ParameterID{id, 1}, name,
                               juce::AudioProcessorParameterWithIDAttributes{}),
          child(p)
    {
        child.addListener(this);
    }
    // detach alt adımı: İç efekt parametresini dış host parametresine yönlendirir; host gesture ve değer ölçeklerini korur.
    void detach()
    {
        if (attached)
        {
            child.removeListener(this);
            attached = false;
        }
    }
    // ~ForwardParameter alt adımı: İç efekt parametresini dış host parametresine yönlendirir; host gesture ve değer ölçeklerini korur.
    ~ForwardParameter() override
    {
        detach();
    }
    // getValue: İç efekt parametresini dış host parametresine yönlendirir; host gesture ve değer ölçeklerini korur. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
    float getValue() const override
    {
        return child.getValue();
    }
    // setValue: İç efekt parametresini dış host parametresine yönlendirir; host gesture ve değer ölçeklerini korur. Burada verilen ayar türetilmiş duruma/çıktıya aktarılır; çağrı sırası aşağıdaki gövdededir.
    void setValue(float v) override
    {
        child.setValueNotifyingHost(v);
    }
    // getDefaultValue: İç efekt parametresini dış host parametresine yönlendirir; host gesture ve değer ölçeklerini korur. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
    float getDefaultValue() const override
    {
        return child.getDefaultValue();
    }
    // getText: İç efekt parametresini dış host parametresine yönlendirir; host gesture ve değer ölçeklerini korur. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
    juce::String getText(float v, int n) const override
    {
        return child.getText(v, n);
    }
    // getValueForText: İç efekt parametresini dış host parametresine yönlendirir; host gesture ve değer ölçeklerini korur. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
    float getValueForText(const juce::String &t) const override
    {
        return child.getValueForText(t);
    }
    // getNumSteps: İç efekt parametresini dış host parametresine yönlendirir; host gesture ve değer ölçeklerini korur. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
    int getNumSteps() const override
    {
        return child.getNumSteps();
    }
    // isDiscrete alt adımı: İç efekt parametresini dış host parametresine yönlendirir; host gesture ve değer ölçeklerini korur.
    bool isDiscrete() const override
    {
        return child.isDiscrete();
    }
    // isBoolean alt adımı: İç efekt parametresini dış host parametresine yönlendirir; host gesture ve değer ölçeklerini korur.
    bool isBoolean() const override
    {
        return child.isBoolean();
    }
    // &getNormalisableRange() const override alt adımı: İç efekt parametresini dış host parametresine yönlendirir; host gesture ve değer ölçeklerini korur.
    const juce::NormalisableRange<float> &getNormalisableRange() const override
    {
        return child.getNormalisableRange();
    }
    // parameterValueChanged alt adımı: İç efekt parametresini dış host parametresine yönlendirir; host gesture ve değer ölçeklerini korur.
    void parameterValueChanged(int, float v) override
    {
        sendValueChangedMessageToListeners(v);
    }
    // parameterGestureChanged alt adımı: İç efekt parametresini dış host parametresine yönlendirir; host gesture ve değer ölçeklerini korur.
    void parameterGestureChanged(int, bool begin) override
    {
        if (begin)
            beginChangeGesture();
        else
            endChangeGesture();
    }
};
