// Geliştirici: Fazıl Irak - Son düzenleme: 27.10.2026
// Dosyanın görevi: Envelope düzenleme ve ürün kontrol bileşeni; özgün kaynak atıfları korunur.
// Katkı kapsamı: F-Sidechain bakımı ve uyarlamaları. GATE-12/Shaper özgün telifleri korunur.
// Fazıl Irak tarafından değiştirildi: 23.10.2026; Shaper uyarlaması.
// Özgün telif ve lisans bildirimleri aşağıda korunur.
#include <JuceHeader.h>
#include <gate12_data.h>
#pragma once

#include <JuceHeader.h>
#include "../Globals.h"
#include "../dsp/Pattern.h"

namespace volumef {

using namespace globals;
class VOLUMEFAudioProcessor;

class PaintTool {
public:
    bool dragging = false;

    PaintTool(VOLUMEFAudioProcessor& p);
    ~PaintTool() {}

    void setViewBounds(int _x, int _y, int _w, int _h);

    void draw(Graphics& g);
    void apply();
    void mouseMove(const MouseEvent& e);
    void mouseDrag(const MouseEvent& e);
    void mouseDown(const MouseEvent& e);
    void mouseUp(const MouseEvent& e);
    void resetPatternTension();

private:
    Pattern* pat;
    int paintW = 100;
    int paintH = 150;
    int startW = 100;
    int startH = 150;
    int winx = 0;
    int winy = 0;
    int winw = 0;
    int winh = 0;
    bool invertx = false;
    bool inverty = false;
    bool snap = false;
    Point<int> mousePos;
    Point<int> lmousePos;
    VOLUMEFAudioProcessor& audioProcessor;

    Rectangle<double> getBounds();
    bool isSnapping(const MouseEvent& e);
};
} // namespace volumef
