// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 Fazıl Irak; Shaper eklemeleri/değişiklikleri.
// Fazıl Irak tarafından oluşturuldu / değiştirildi: 23.10.2026
// Kapsam: Shaper için eklenen kod ve uyarlamalar; özgün üçüncü taraf telif/lisansları korunur.
// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: Efekt kimliği, adı, rengi ve üretici fonksiyonunu tek kayıt sözleşmesinde tutar.
#include "EffectPalette.h"
// EffectPalette alt adımı: Efekt kimliği, adı, rengi ve üretici fonksiyonunu tek kayıt sözleşmesinde tutar.
// Gövdedeki başlıca çağrılar: addChildComponent → addAndMakeVisible → add.setTooltip → std::find. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
EffectPalette::EffectPalette(ShaperProcessor &p) : processor(p)
{
    for (int e = 0; e < ShaperProcessor::effectCount; ++e)
    {
        cards[e] = std::make_unique<Card>(*this, e);
        addChildComponent(*cards[e]);
    }
    addAndMakeVisible(add);
    add.setTooltip("Add effect");
    add.onClick = [this]
    {
        juce::PopupMenu menu;
        for (int e = 0; e < ShaperProcessor::effectCount; ++e)
            if (std::find(installed.begin(), installed.end(), e) == installed.end())
                menu.addItem(e + 1, ShaperProcessor::effectName(e));
        juce::Component::SafePointer<EffectPalette> safe(this);
        menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&add),
                           [safe](int item)
                           {
                               if (safe && item > 0)
                               {
                                   safe->processor.addEffect(item - 1);
                                   safe->changed();
                               }
                           });
    };
    refresh();
}
// changed: Efekt kimliği, adı, rengi ve üretici fonksiyonunu tek kayıt sözleşmesinde tutar. Burada verilen ayar türetilmiş duruma/çıktıya aktarılır; çağrı sırası aşağıdaki gövdededir.
void EffectPalette::changed()
{
    refresh();
    if (onChange)
        onChange();
}
// refresh alt adımı: Efekt kimliği, adı, rengi ve üretici fonksiyonunu tek kayıt sözleşmesinde tutar.
void EffectPalette::refresh()
{
    installed = processor.palette();
    for (int e = 0; e < ShaperProcessor::effectCount; ++e)
    {
        cards[e]->setVisible(std::find(installed.begin(), installed.end(), e) != installed.end());
        cards[e]->repaint();
    }
    add.setVisible(installed.size() < ShaperProcessor::effectCount);
    resized();
    repaint();
}
// Pencere/kontrol boyutuna bağlı yerleşimi yeniden hesaplar; hit-test aynı geometriyi izlemelidir.
void EffectPalette::resized()
{
    const int cardWidth = juce::jmin(190, (getWidth() - 70) / ShaperProcessor::effectCount);
    int x = 0;
    for (int e : installed)
    {
        cards[e]->setBounds(x, 0, cardWidth, getHeight());
        x += cardWidth + 10;
    }
    add.setBounds(x, 0, 58, getHeight());
}
// Mevcut kontrol/durumun görselini çizer; ses callback’inde çağrılmamalıdır.
// Gövdedeki başlıca çağrılar: installed.empty → g.setColour → juce::Colour → g.setFont. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
void EffectPalette::paint(juce::Graphics &g)
{
    if (installed.empty())
    {
        g.setColour(juce::Colour(0xff8993a1));
        g.setFont(juce::FontOptions(14));
        g.drawText("Add an effect to start. Drag cards to change the signal order.", 78, 0, getWidth() - 85,
                   getHeight(), juce::Justification::centredLeft);
    }
    if (dropIndex >= 0)
    {
        int cardWidth = juce::jmin(190, (getWidth() - 70) / ShaperProcessor::effectCount);
        g.setColour(juce::Colours::white);
        g.fillRoundedRectangle((float)juce::jmax(0, dropIndex * (cardWidth + 10) - 6), 2, 3,
                               (float)getHeight() - 4, 1);
    }
}
// Mevcut kontrol/durumun görselini çizer; ses callback’inde çağrılmamalıdır.
// Gövdedeki başlıca çağrılar: rackEffectColour → getLocalBounds → toFloat → reduced. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
void EffectPalette::Card::paint(juce::Graphics &g)
{
    auto colour = rackEffectColour(id);
    bool selected = owner.processor.selectedEffect == id;
    auto bounds = getLocalBounds().toFloat().reduced(1);
    g.setColour(colour.withMultipliedBrightness(selected ? 0.34f : 0.16f));
    g.fillRoundedRectangle(bounds, 6);
    g.setColour(colour.withAlpha(selected ? 1.0f : 0.55f));
    g.drawRoundedRectangle(bounds, 6, selected ? 2.0f : 1.0f);
    g.setColour(colour);
    g.fillRoundedRectangle(10, 10, 4, (float)getHeight() - 20, 2);
    g.setFont(juce::FontOptions(15, juce::Font::bold));
    g.drawText(ShaperProcessor::effectName(id), 24, 8, getWidth() - 44, 28, juce::Justification::centredLeft);
    g.setColour(juce::Colours::white.withAlpha(0.8f));
    g.setFont(juce::FontOptions(20));
    g.drawText(juce::String::charToString(0x00d7), getWidth() - 28, 3, 24, 24, juce::Justification::centred);
    bool active = owner.processor.state.getRawParameterValue(ShaperProcessor::enableId(id))->load() > 0.5f;
    g.setColour(active ? colour : juce::Colour(0xff6a7380));
    g.fillEllipse(26, 46, 7, 7);
    g.setFont(juce::FontOptions(12));
    g.drawText(active ? "ON" : "BYPASSED", 39, 39, 85, 22, juce::Justification::centredLeft);
    g.setColour(juce::Colours::white.withAlpha(0.3f));
    g.drawText("::", getWidth() - 30, 38, 20, 24, juce::Justification::centred);
}
// mouseDown: kullanıcı/pencere olayını bu bileşenin parametre ve sahiplik durumuna taşır; kapanmış nesne ve boş seçim yollarını koruyun.
void EffectPalette::Card::mouseDown(const juce::MouseEvent &e)
{
    if (e.x >= getWidth() - 32 && e.y < 31)
    {
        owner.processor.removeEffect(id);
        owner.changed();
        return;
    }
    if (e.y >= 37 && e.x < 125)
    {
        auto *p = owner.processor.state.getParameter(ShaperProcessor::enableId(id));
        p->setValueNotifyingHost(p->getValue() > 0.5f ? 0.0f : 1.0f);
        repaint();
        return;
    }
    owner.processor.selectedEffect = id;
    owner.changed();
}
// mouseDrag: kullanıcı/pencere olayını bu bileşenin parametre ve sahiplik durumuna taşır; kapanmış nesne ve boş seçim yollarını koruyun.
void EffectPalette::Card::mouseDrag(const juce::MouseEvent &e)
{
    if (e.getDistanceFromDragStart() > 5 && !owner.isDragAndDropActive())
        owner.startDragging(id, this);
}
// isInterestedInDragSource alt adımı: Efekt kimliği, adı, rengi ve üretici fonksiyonunu tek kayıt sözleşmesinde tutar.
bool EffectPalette::isInterestedInDragSource(const SourceDetails &details)
{
    if (!details.description.isInt())
        return false;
    int id = (int)details.description;
    return id >= 0 && id < ShaperProcessor::effectCount && details.sourceComponent.get() == cards[id].get();
}
// insertionAt alt adımı: Efekt kimliği, adı, rengi ve üretici fonksiyonunu tek kayıt sözleşmesinde tutar.
int EffectPalette::insertionAt(int x, int moving) const
{
    int destination = 0;
    for (int e : installed)
        if (e != moving && x > cards[e]->getBounds().getCentreX())
            ++destination;
    return destination;
}
// itemDragMove alt adımı: Efekt kimliği, adı, rengi ve üretici fonksiyonunu tek kayıt sözleşmesinde tutar.
void EffectPalette::itemDragMove(const SourceDetails &details)
{
    dropIndex = insertionAt(details.localPosition.x, (int)details.description);
    repaint();
}
// itemDragExit alt adımı: Efekt kimliği, adı, rengi ve üretici fonksiyonunu tek kayıt sözleşmesinde tutar.
void EffectPalette::itemDragExit(const SourceDetails &)
{
    dropIndex = -1;
    repaint();
}
// itemDropped alt adımı: Efekt kimliği, adı, rengi ve üretici fonksiyonunu tek kayıt sözleşmesinde tutar.
void EffectPalette::itemDropped(const SourceDetails &details)
{
    if (!isInterestedInDragSource(details))
        return;
    processor.moveEffect((int)details.description,
                         insertionAt(details.localPosition.x, (int)details.description));
    dropIndex = -1;
    changed();
}
