// Geliştirici: Fazıl Irak - Son düzenleme: 27.10.2026
// Dosyanın görevi: Windows dosya bırakma olaylarını asenkron sample yükleyicisine taşımak.
// Copyright (c) 2026 Fazıl Irak - Özgün F-Kick uygulama kodu.
// SPDX-License-Identifier: AGPL-3.0-or-later
// Fazıl Irak · Windows OLE dosya bırakma köprüsü; host'un HWND'si değiştirilmez.
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
class NativeFileDrop {
public:
    NativeFileDrop();
    ~NativeFileDrop();
    void attach(juce::Component&);
    std::function<bool(const juce::StringArray&)> accepts;
    std::function<void(const juce::StringArray&,juce::Point<int>,bool)> hover;
    std::function<void(const juce::StringArray&,juce::Point<int>)> drop;
    bool registered() const noexcept;
    bool testNativeTransfer(const juce::StringArray&,juce::Point<int>);
private:
    struct Impl;std::unique_ptr<Impl> impl;
};
