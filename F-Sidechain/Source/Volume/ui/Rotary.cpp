// Geliştirici: Fazıl Irak - Son düzenleme: 27.10.2026
// Dosyanın görevi: Envelope düzenleme ve ürün kontrol bileşeni; özgün kaynak atıfları korunur.
// Katkı kapsamı: F-Sidechain bakımı ve uyarlamaları. GATE-12/Shaper özgün telifleri korunur.
// Fazıl Irak tarafından değiştirildi: 23.10.2026; Shaper uyarlaması.
// Özgün telif ve lisans bildirimleri aşağıda korunur.
#include <JuceHeader.h>
#include <gate12_data.h>
#include "RackKnob.h"
#include "Rotary.h"
#include "../PluginProcessor.h"

namespace volumef {

Rotary::Rotary(VOLUMEFAudioProcessor& p, juce::String paramId, juce::String name, RotaryLabel format, bool isSymmetric, bool isAudioKnob)
    : juce::SettableTooltipClient()
    , juce::Component()
    , audioProcessor(p)
    , paramId(paramId)
    , name(name)
    , format(format)
    , isSymmetric(isSymmetric)
    , isAudioKnob(isAudioKnob)
{
    setName(name);
    audioProcessor.params.addParameterListener(paramId, this);
}

Rotary::~Rotary()
{
    audioProcessor.params.removeParameterListener(paramId, this);
}

void Rotary::parameterChanged(const juce::String& parameterID, float newValue) 
{
    (void)parameterID;
    (void)newValue;
    juce::MessageManager::callAsync([this] { repaint(); });
}

static float unmapRateSyncNormal(float norm) {
    static constexpr int numSteps = 21;
    float value = std::clamp(norm, 1.0f / numSteps, 1.0f);
    return (value - (1.0f / numSteps)) / (1.0f - (1.0f / numSteps));
}

void Rotary::paint(juce::Graphics& g) {
    auto transform=getTransform();
    if(transform.mat00>0 && transform.mat11>transform.mat00)
        g.addTransform(juce::AffineTransform::scale(1.0f,transform.mat00/transform.mat11));

    auto param = audioProcessor.params.getParameter(paramId);
    auto normValue = param->getValue();
    auto value = param->convertFrom0to1(normValue);

    if (format == rateSync) {
        normValue = unmapRateSyncNormal(normValue);
    }

    draw_rotary_slider(g, normValue); 
    draw_label_value(g, value);
    //g.drawRect(getLocalBounds());
}

void Rotary::draw_label_value(juce::Graphics& g, float slider_val)
{
    auto text = String(name);
    std::stringstream ss;

    if (mouse_down) {
        if (format == RotaryLabel::percx100) text = std::to_string((int)std::round((slider_val * 100))) + " %";
        if (format == RotaryLabel::intx100) text = std::to_string((int)std::round((slider_val * 100)));
        else if (format == RotaryLabel::hz) text = std::to_string((int)slider_val) + " Hz";
        else if (format == RotaryLabel::hzLp) text = slider_val >= 20000 ? "Off" : std::to_string((int)slider_val) + " Hz";
        else if (format == RotaryLabel::hzHp) text = slider_val <= 20 ? "Off" : std::to_string((int)slider_val) + " Hz";
        else if (format == RotaryLabel::hz1f) {
            ss << std::fixed << std::setprecision(1) << slider_val << " Hz";
            text = ss.str();
        }
        else if (format == RotaryLabel::float1) {
            ss << std::fixed << std::setprecision(1) << slider_val;
            text = ss.str();
        }
        else if (format == RotaryLabel::float2) {
            ss << std::fixed << std::setprecision(2) << slider_val;
            text = ss.str();
        }
        else if (format == RotaryLabel::float2x100) {
            ss << std::fixed << std::setprecision(2) << slider_val * 100;
            text = ss.str();
        }
        else if (format == RotaryLabel::gainTodB1f) {
            if (slider_val > 0.0f) {
                ss << std::fixed << std::setprecision(1) << 20.0f * std::log10(slider_val) << " dB";
                text = ss.str();
            }
            else {
                text = "-Inf";
            }
        }
        else if (format == RotaryLabel::audioOffset) {
            ss << std::fixed << std::setprecision(1) << slider_val * AUDIO_LATENCY_MILLIS << " ms";
            text = ss.str();
        }
        else if (format == rateSync) {
            if (slider_val == 1) text = "1/256";
            else if (slider_val == 2) text = "1/128";
            else if (slider_val == 3) text = "1/64";
            else if (slider_val == 4) text = "1/32";
            else if (slider_val == 5) text = "1/16";
            else if (slider_val == 6) text = "1/8";
            else if (slider_val == 7) text = "1/4";
            else if (slider_val == 8) text = "1/2";
            else if (slider_val == 9) text = "Bar";
            else if (slider_val == 10) text = "2 Bar";
            else if (slider_val == 11) text = "4 Bar";
            else if (slider_val == 12) text = "1/16T";
            else if (slider_val == 13) text = "1/8T";
            else if (slider_val == 14) text = "1/4T";
            else if (slider_val == 15) text = "1/2T";
            else if (slider_val == 16) text = "1/1T";
            else if (slider_val == 17) text = "1/16.";
            else if (slider_val == 18) text = "1/8.";
            else if (slider_val == 19) text = "1/4.";
            else if (slider_val == 20) text = "1/2.";
            else if (slider_val == 21) text = "1/1.";
        }
    }

    g.setColour(Colours::white);
    g.setFont(16.0f);
    g.drawText(text, 0, getHeight() - 16, getWidth(), 16, juce::Justification::centred, true);
}

void Rotary::mouseDown(const juce::MouseEvent& e) 
{
    e.source.enableUnboundedMouseMovement(true);
    mouse_down = true;
    auto param = audioProcessor.params.getParameter(paramId);
    auto cur_val = param->getValue();
    cur_normed_value = cur_val;
    last_mouse_position = e.getPosition();
    setMouseCursor(MouseCursor::NoCursor);
    start_mouse_pos = Desktop::getInstance().getMousePosition();
    repaint();
    param->beginChangeGesture();
}

void Rotary::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    if (event.mods.isLeftButtonDown() || event.mods.isRightButtonDown()) {
        return; // prevent crash, param is already mutating
    }
    auto speed = (event.mods.isShiftDown() ? 0.01f : 0.05f);
    auto slider_change = wheel.deltaY > 0 ? speed : wheel.deltaY < 0 ? -speed : 0;
    auto param = audioProcessor.params.getParameter(paramId);
    param->beginChangeGesture();
    param->setValueNotifyingHost(param->getValue() + slider_change);
    while (wheel.deltaY > 0.0f && param->getValue() == 0.0f) { // FIX wheel not working when value is zero, first step takes more than 0.05% 
        slider_change += 0.05f;
        param->setValueNotifyingHost(param->getValue() + slider_change);
    }
    param->endChangeGesture();
}

void Rotary::mouseUp(const juce::MouseEvent& e) {
    mouse_down = false;
    setMouseCursor(MouseCursor::NormalCursor);
    e.source.enableUnboundedMouseMovement(false);
    Desktop::getInstance().setMousePosition(start_mouse_pos);
    repaint();
    auto param = audioProcessor.params.getParameter(paramId);
    param->endChangeGesture();
}

void Rotary::mouseDoubleClick(const juce::MouseEvent& e) {
    (void)e;
    auto param = audioProcessor.params.getParameter(paramId);
    param->setValueNotifyingHost(param->getDefaultValue());
}

void Rotary::mouseDrag(const juce::MouseEvent& e) {
    auto change = e.getPosition() - last_mouse_position;
    last_mouse_position = e.getPosition();
    auto speed = (e.mods.isShiftDown() ? 40.0f : 4.0f) * pixels_per_percent;
    auto slider_change = float(change.getX() - change.getY()) / speed;
    cur_normed_value += slider_change;
    auto param = audioProcessor.params.getParameter(paramId);
    if (format == rateSync && cur_normed_value < 1.f/22) {
        cur_normed_value = 1.f / 22;
    }
    param->setValueNotifyingHost(cur_normed_value);
}

void Rotary::draw_rotary_slider(juce::Graphics& g, float slider_pos) {
    shaperui::drawKnob(g,getLocalBounds(),slider_pos,juce::Colour(isAudioKnob?COLOR_AUDIO:COLOR_ACTIVE),isSymmetric,juce::Colour(COLOR_KNOB));
}
} // namespace volumef
