// Geliştirici: Fazıl Irak - Son düzenleme: 27.10.2026
// Dosyanın görevi: Envelope düzenleme ve ürün kontrol bileşeni; özgün kaynak atıfları korunur.
// Katkı kapsamı: F-Sidechain bakımı ve uyarlamaları. GATE-12/Shaper özgün telifleri korunur.
// Fazıl Irak tarafından değiştirildi: 23.10.2026; Shaper uyarlaması.
// Özgün telif ve lisans bildirimleri aşağıda korunur.
#include <JuceHeader.h>
#include <gate12_data.h>
#include "About.h"

namespace volumef {

void About::mouseDown(const juce::MouseEvent&) {};

void About::paint(Graphics& g)
{
	auto bounds = getBounds();
	g.setColour(Colour(0xdd000000));
	g.fillRect(bounds);

	bounds.reduce(50,50);
	g.setColour(Colours::white);
	g.setFont(FontOptions(30.f));
	g.drawText("Volume", bounds.removeFromTop(35), Justification::centred);
	g.setFont(FontOptions(20.f));
	g.drawText(std::string("v") + PROJECT_VERSION, bounds.removeFromTop(25), Justification::centred);
	g.setFont(FontOptions(20.0f));
	g.drawText(juce::String::fromUTF8("Fazıl Irak | Based on GATE-12 by Tilr"), bounds.removeFromTop(25), Justification::centred);
	g.setColour(Colour(COLOR_ACTIVE));
	g.drawText("Volume - sidechain volume shaper", bounds.removeFromTop(25), Justification::centred);
	g.setColour(Colours::white);
	auto w = PLUG_WIDTH - 100;
	bounds.setWidth(w);
	bounds.setX(getWidth() / 2 - w/2);
	bounds.removeFromTop(40);
	g.drawText("- Shift for fine slider adjustments.", bounds.removeFromTop(25), Justification::centredLeft);
	g.drawText("- Shift toggles snap on/off.", bounds.removeFromTop(25), Justification::centredLeft);
	g.drawText("- Mouse wheel on view changes grid size.", bounds.removeFromTop(25), Justification::centredLeft);
	g.drawText("- Right click points changes point type.", bounds.removeFromTop(25), Justification::centredLeft);
	g.drawText("- Alt + click inserts new point.", bounds.removeFromTop(25), Justification::centredLeft);
	g.drawText("- Alt + drag selection handles skews selected points.", bounds.removeFromTop(25), Justification::centredLeft);
	g.drawText("- Right click + drag in paint mode changes paint tool tension.", bounds.removeFromTop(25), Justification::centredLeft);
	g.drawText("- Shift + mouse wheel in sequencer changes step size.", bounds.removeFromTop(25), Justification::centredLeft);
};


} // namespace volumef
