// Fazıl Irak tarafından değiştirildi: 23.10.2026; Shaper uyarlaması.
// Özgün telif ve lisans bildirimleri aşağıda korunur.
#include <JuceHeader.h>
#include <gate12_data.h>
#pragma once

#include <JuceHeader.h>
#include "../Globals.h"

namespace volumef {

using namespace globals;
class VOLUMEFAudioProcessorEditor;

class BandsWidget
	: public juce::Component
	, private juce::Timer
	, private juce::AudioProcessorValueTreeState::Listener
{
public:
	TextButton slopeBtn;

	BandsWidget(VOLUMEFAudioProcessorEditor& e);
	~BandsWidget();

	void timerCallback();
	void parameterChanged(const juce::String& parameterID, float newValue) override;
	void paint(Graphics& g) override;
	void mouseMove(const MouseEvent& e) override;
	void mouseDown(const MouseEvent& e) override;
	void mouseUp(const MouseEvent& e) override;
	void mouseDrag(const MouseEvent& e) override;
	void mouseDoubleClick(const MouseEvent& e) override;
	void resized() override;
	void drawWaveform(juce::Graphics& g);
	void recalcFFTMags();

private:
	VOLUMEFAudioProcessorEditor& editor;
	juce::dsp::FFT fft{ BANDS_FFT_ORDER };
	juce::dsp::WindowingFunction<float> window{ 1 << BANDS_FFT_ORDER, juce::dsp::WindowingFunction<float>::blackmanHarris };
	std::array<float, (1 << BANDS_FFT_ORDER) * 2> fftData{};
	std::array<float, (1 << BANDS_FFT_ORDER) / 2> fftMagnitudes{};

	Rectangle<float> lband;
	Rectangle<float> rband;

	int mouseOverBand = 0;
	bool isDragging = false;
	float cur_normed_value = 0.f;
	Point<int> last_mouse_pos;
	Point<int> start_mouse_pos;
};
} // namespace volumef
