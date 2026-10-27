// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (c) 2026 Fazıl Irak
// Geliştirici: Fazıl Irak | Oluşturma / son düzenleme: 27.10.2026
// Dosyanın görevi: Birlikte çalışan çoklu efekt makrolarını tanımlamak.
// Tarifler özgün DSP ayarlarıdır; başka bir ürünün gizli parametreleri veya preset dosyaları değildir.
#pragma once
#include <array>
namespace ft {
struct Technique { const char* name; const char* group; const char* description; };
inline constexpr std::array<Technique,7> techniques {{
 {"Air Bloom", "SPACE", "A gentle low cut opens into a wide, floating reverb bloom."},
 {"Echo Lift", "SPACE", "Low cut, dotted echoes and a growing hall lift the source together."},
 {"Skyline", "ASCENT", "A rising high-pass, echo and phaser build a bright, spacious ascent."},
 {"Heatwave", "ASCENT", "High-pass, warm drive, moving phase and reverb gather into a heated rise."},
 {"Spiral Rise", "TENSION", "Shifted echoes, sweeping phase and airy noise climb together."},
 {"Fracture", "TENSION", "Sharper filtering, driven shifted echoes and noise push the tension further."},
 {"Critical Mass", "CLIMAX", "Filter, hall, echo, phase, noise and a rising octave cloud converge, then dissolve."}
}};
inline constexpr int techniqueCount=(int)techniques.size();
inline constexpr int leftTechniqueCount=(techniqueCount+1)/2;
// Onaylı iki sütun, metin/nokta boşluğu ve harf merkezine hizalama korunur.
inline float techniqueLabelY(int i) {
 return i<leftTechniqueCount ? 246.f+(leftTechniqueCount-1-i)*56.f : 246.f+(i-leftTechniqueCount)*84.f;
}
inline constexpr const char* characters[] {"Silk", "Soft", "Balanced", "Hard", "Extreme"};
// Son HP, reverb, echo, feedback, drive, phaser, shift, noise, riser, dry azaltma, son fade.
struct Recipe { float highpass, space, echo, feedback, drive, phaser, shift, noise, riser, dissolve, finish; };
inline constexpr std::array<Recipe,techniqueCount> recipes {{
 {  220, .62f, .00f, .12f, .00f, .00f, .00f, .000f, .000f, .16f, .00f},
 // Echo Lift: ilk tekrar katmanı 0.36 -> 0.95 (+8.4 dB); feedback 0.34 -> 0.64.
 // Hall 0.70 -> 0.50: belirgin ritmik tekrarları geniş reverb kuyruğu örtmesin.
 {  480, .50f, .95f, .64f, .04f, .05f, .00f, .000f, .000f, .28f, .00f},
 { 4200, .76f, .40f, .42f, .10f, .24f, .00f, .000f, .000f, .45f, .00f},
 { 6800, .70f, .35f, .40f, .52f, .46f, .12f, .015f, .000f, .50f, .00f},
 { 8800, .72f, .48f, .54f, .20f, .48f, .70f, .110f, .030f, .62f, .00f},
 {11200, .62f, .62f, .65f, .82f, .58f, .92f, .140f, .040f, .72f, .35f},
 {14500, .80f, .65f, .72f,1.00f, .64f,1.00f, .200f, .120f, .88f,1.00f}
}};
}

