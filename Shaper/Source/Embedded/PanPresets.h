// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 Fazıl Irak; Shaper eklemeleri/değişiklikleri.
// Fazıl Irak tarafından oluşturuldu / değiştirildi: 23.10.2026
// Kapsam: Shaper için eklenen kod ve uyarlamalar; özgün üçüncü taraf telif/lisansları korunur.
// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: Pan modülü için hazır desen ve parametre verilerini tanımlar.
#pragma once
#include <juce_core/juce_core.h>
#include <vector>
#include <cmath>
struct PanPresets
{
    struct Point
    {
        double x, y, tension;
        int type;
    };
    static constexpr int count = 20;
    // name alt adımı: Pan modülü için hazır desen ve parametre verilerini tanımlar.
    // Gövdedeki başlıca çağrılar: juce::jlimit. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    static const char *name(int i)
    {
        static const char *names[]{"Centre",
                                   "Hard Left",
                                   "Hard Right",
                                   "Sweep Left to Right",
                                   "Sweep Right to Left",
                                   "Sine Auto Pan",
                                   "Triangle Auto Pan",
                                   "Smooth Wide Pan",
                                   "Narrow Stereo Motion",
                                   "Ping Pong 2",
                                   "Ping Pong 4",
                                   "Ping Pong 8",
                                   "Soft Stereo Bounce",
                                   "Left Accent",
                                   "Right Accent",
                                   "Centre to Wide",
                                   "Wide to Centre",
                                   "Triplet Pan",
                                   "Swing Pan",
                                   "Subtle Drift"};
        return names[juce::jlimit(0, count - 1, i)];
    }
    // position alt adımı: Pan modülü için hazır desen ve parametre verilerini tanımlar.
    // Gövdedeki başlıca çağrılar: std::cos → std::sin → std::abs → std::floor. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    static double position(int i, double x)
    {
        const double pi = juce::MathConstants<double>::pi;
        switch (i)
        {
        case 0:
            return 0.5;
        case 1:
            return 0;
        case 2:
            return 1;
        case 3:
            return 0.5 - 0.5 * std::cos(pi * x);
        case 4:
            return 0.5 + 0.5 * std::cos(pi * x);
        case 5:
            return 0.5 + 0.5 * std::sin(2 * pi * x);
        case 6:
            return 1.0 - std::abs(2 * x - 1);
        case 7:
            return 0.5 - 0.5 * std::cos(2 * pi * x);
        case 8:
            return 0.5 + 0.2 * std::sin(2 * pi * x);
        case 9:
            return ((int)std::floor(x * 2) % 2) ? 1 : 0;
        case 10:
            return ((int)std::floor(x * 4) % 2) ? 1 : 0;
        case 11:
            return ((int)std::floor(x * 8) % 2) ? 1 : 0;
        case 12:
            return 0.5 + 0.45 * std::sin(4 * pi * x) * std::sin(2 * pi * x);
        case 13:
            return 0.5 - 0.5 * std::pow(0.5 + 0.5 * std::cos(2 * pi * x), 4);
        case 14:
            return 0.5 + 0.5 * std::pow(0.5 + 0.5 * std::cos(2 * pi * x), 4);
        case 15:
            return 0.5 + 0.5 * x * std::sin(4 * pi * x);
        case 16:
            return 0.5 + 0.5 * (1 - x) * std::sin(4 * pi * x);
        case 17:
            return ((int)std::floor(x * 6) % 2) ? 0.95 : 0.05;
        case 18:
            return std::fmod(x, 0.5) < (1.0 / 3.0) ? 0.1 : 0.9;
        default:
            return 0.5 + 0.12 * std::sin(2 * pi * x);
        }
    }
    // points alt adımı: Pan modülü için hazır desen ve parametre verilerini tanımlar.
    // Gövdedeki başlıca çağrılar: position → edges.push_back → result.push_back → juce::jlimit. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    static std::vector<Point> points(int index)
    {
        std::vector<Point> result;
        if (index <= 2)
            return {{0, 1.0 - position(index, 0), 0, 1}, {1, 1.0 - position(index, 0), 0, 1}};
        if ((index >= 9 && index <= 11) || index == 17 || index == 18)
        {
            std::vector<double> edges;
            int steps = index == 9 ? 2 : index == 10 ? 4 : index == 11 ? 8 : 6;
            if (index == 18)
                edges = {1.0 / 3.0, 0.5, 5.0 / 6.0};
            else
                for (int n = 1; n < steps; ++n)
                    edges.push_back((double)n / steps);
            result.push_back({0, 1.0 - position(index, 0), 0, 1});
            for (double edge : edges)
            {
                result.push_back({edge - 0.0025, 1.0 - position(index, edge - 0.0025), 0, 1});
                result.push_back({edge + 0.0025, 1.0 - position(index, edge + 0.0025), 0, 1});
            }
            result.push_back({1, 1.0 - position(index, 1.0 - 1e-8), 0, 1});
            return result;
        }
        for (int n = 0; n <= 24; ++n)
        {
            double x = n / 24.0;
            double pos = position(index, n == 24 ? 1.0 - 1e-8 : x);
            result.push_back({x, 1.0 - juce::jlimit(0.0, 1.0, pos), 0, 1});
        }
        return result;
    }
    // sync alt adımı: Pan modülü için hazır desen ve parametre verilerini tanımlar.
    static int sync(int index)
    {
        return index == 3 || index == 4 || index == 8 || index == 19 ? 10 : 9;
    }
};
