#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>

//==============================================================
// KNOB ANGLES
//==============================================================

namespace
{
constexpr float knobStartAngle =
juce::MathConstants<float>::pi * 2.0f / 3.0f;

constexpr float knobEndAngle =
    juce::MathConstants<float>::pi * 7.0f / 3.0f;

}

//==============================================================
// PEDAL KNOB
//==============================================================

RG_Precision_DriveAudioProcessorEditor::PedalKnob::PedalKnob()
{
setSliderStyle(
juce::Slider::RotaryHorizontalVerticalDrag);

setTextBoxStyle(
    juce::Slider::NoTextBox,
    false,
    0,
    0);

setRange(0.0, 1.0, 0.001);

setRotaryParameters(
    knobStartAngle,
    knobEndAngle,
    true);

setMouseDragSensitivity(180);

setColour(
    juce::Slider::rotarySliderFillColourId,
    juce::Colours::transparentBlack);

setColour(
    juce::Slider::rotarySliderOutlineColourId,
    juce::Colours::transparentBlack);

setColour(
    juce::Slider::thumbColourId,
    juce::Colours::transparentBlack);

}

//==============================================================
// PEDAL KNOB PAINT
//==============================================================

void RG_Precision_DriveAudioProcessorEditor::PedalKnob::paint(
juce::Graphics& g)
{
auto area = getLocalBounds().toFloat();

const bool smallKnob = getWidth() <= 65;

const float size =
    juce::jmin(
        area.getWidth(),
        area.getHeight());

auto knobArea =
    area.withSizeKeepingCentre(
        size,
        size).reduced(3.0f);

const auto centre = knobArea.getCentre();

const float radius =
    juce::jmin(
        knobArea.getWidth(),
        knobArea.getHeight()) * 0.5f;

//==========================================================
// CRYSTAL GATE
//==========================================================

if (smallKnob)
{
    juce::ColourGradient crystal(
        juce::Colour(0xffe8f4ff).withAlpha(0.36f),
        centre.x - radius * 0.45f,
        centre.y - radius * 0.65f,

        juce::Colour(0xff506b7c).withAlpha(0.24f),
        centre.x + radius * 0.60f,
        centre.y + radius * 0.70f,
        true);

    g.setGradientFill(crystal);
    g.fillEllipse(knobArea);

    g.setColour(
        juce::Colours::white.withAlpha(0.68f));

    g.drawEllipse(
        knobArea,
        1.4f);

    g.setColour(
        juce::Colours::white.withAlpha(0.18f));

    g.fillEllipse(
        knobArea.reduced(radius * 0.28f));

    const float value =
        (float)getNormalisableRange()
            .convertTo0to1(getValue());

    const float angle =
        knobStartAngle +
        value *
        (knobEndAngle - knobStartAngle);

    const auto p1 =
        centre +
        juce::Point<float>(
            std::cos(angle) * radius * 0.22f,
            std::sin(angle) * radius * 0.22f);

    const auto p2 =
        centre +
        juce::Point<float>(
            std::cos(angle) * radius * 0.72f,
            std::sin(angle) * radius * 0.72f);

    g.setColour(
        juce::Colours::white.withAlpha(0.95f));

    g.drawLine(
        p1.x,
        p1.y,
        p2.x,
        p2.y,
        1.8f);

    return;
}

//==========================================================
// BLACK METAL KNOB
//==========================================================

juce::ColourGradient knobGradient(
    juce::Colour(0xff363636),
    centre.x - radius * 0.55f,
    centre.y - radius * 0.70f,

    juce::Colour(0xff050505),
    centre.x + radius * 0.65f,
    centre.y + radius * 0.75f,
    true);

g.setGradientFill(knobGradient);

g.fillEllipse(knobArea);

g.setColour(
    juce::Colour(0xff929292)
        .withAlpha(0.78f));

g.drawEllipse(
    knobArea,
    1.3f);

g.setColour(
    juce::Colour(0xff111111));

g.drawEllipse(
    knobArea.reduced(radius * 0.12f),
    1.0f);

g.setColour(
    juce::Colours::white.withAlpha(0.10f));

g.drawEllipse(
    knobArea.reduced(2.0f),
    1.5f);

//==========================================================
// POINTER
//==========================================================

const float value =
    (float)getNormalisableRange()
        .convertTo0to1(getValue());

const float angle =
    knobStartAngle +
    value *
    (knobEndAngle - knobStartAngle);

const auto p1 =
    centre +
    juce::Point<float>(
        std::cos(angle) * radius * 0.25f,
        std::sin(angle) * radius * 0.25f);

const auto p2 =
    centre +
    juce::Point<float>(
        std::cos(angle) * radius * 0.72f,
        std::sin(angle) * radius * 0.72f);

g.setColour(
    juce::Colours::white.withAlpha(0.95f));

g.drawLine(
    p1.x,
    p1.y,
    p2.x,
    p2.y,
    2.3f);

g.fillEllipse(
    p2.x - 1.8f,
    p2.y - 1.8f,
    3.6f,
    3.6f);

}

//==============================================================
// CONSTRUCTOR
//==============================================================

RG_Precision_DriveAudioProcessorEditor::
RG_Precision_DriveAudioProcessorEditor(
RG_Precision_DriveAudioProcessor& p)
: AudioProcessorEditor(&p),
audioProcessor(p)
{
//==========================================================
// ACTUAL FINAL EDITOR SIZE
//==========================================================

setSize(600, 660);

setResizable(
    false,
    false);

//==========================================================
// KNOBS
//==========================================================

setupKnob(
    volumeKnob,
    juce::Slider::RotaryHorizontalVerticalDrag);

setupKnob(
    brightKnob,
    juce::Slider::RotaryHorizontalVerticalDrag);

setupKnob(
    attackKnob,
    juce::Slider::RotaryHorizontalVerticalDrag,
    true);

setupKnob(
    driveKnob,
    juce::Slider::RotaryHorizontalVerticalDrag);

setupKnob(
    gateKnob,
    juce::Slider::RotaryHorizontalVerticalDrag);

gateKnob.setComponentID("GATE");

//==========================================================
// LABELS
//==========================================================

setupLabel(volumeLabel, "VOLUME");
setupLabel(brightLabel, "BRIGHT");
setupLabel(attackLabel, "ATTACK");
setupLabel(driveLabel, "DRIVE");
setupLabel(gateLabel, "GATE");

//==========================================================
// FOOTSWITCH
//==========================================================

bypassButton.setButtonText("");

bypassButton.setClickingTogglesState(true);

bypassButton.setAlpha(0.001f);

bypassButton.setMouseCursor(
    juce::MouseCursor::PointingHandCursor);

addAndMakeVisible(bypassButton);

bypassButton.onStateChange =
    [this]()
    {
        footswitchPressed =
            bypassButton.getToggleState();

        repaint();
    };

//==========================================================
// PARAMETER ATTACHMENTS
//==========================================================

volumeAttachment =
    std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.parameters,
            "VOLUME",
            volumeKnob);

brightAttachment =
    std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.parameters,
            "BRIGHT",
            brightKnob);

attackAttachment =
    std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.parameters,
            "ATTACK",
            attackKnob);

driveAttachment =
    std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.parameters,
            "DRIVE",
            driveKnob);

gateAttachment =
    std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.parameters,
            "GATE",
            gateKnob);

bypassAttachment =
    std::make_unique<
        juce::AudioProcessorValueTreeState::ButtonAttachment>(
            audioProcessor.parameters,
            "BYPASS",
            bypassButton);

//==========================================================
// LED TIMER
//==========================================================

startTimerHz(30);

repaint();

}

//==============================================================
// DESTRUCTOR
//==============================================================

RG_Precision_DriveAudioProcessorEditor::
~RG_Precision_DriveAudioProcessorEditor()
{
stopTimer();
}

//==============================================================
// SETUP KNOB
//==============================================================

void RG_Precision_DriveAudioProcessorEditor::setupKnob(
PedalKnob& knob,
juce::Slider::SliderStyle style,
bool selector)
{
knob.setSliderStyle(style);

knob.setTextBoxStyle(
    juce::Slider::NoTextBox,
    false,
    0,
    0);

knob.setRotaryParameters(
    knobStartAngle,
    knobEndAngle,
    true);

knob.setMouseDragSensitivity(180);

if (selector)
{
    knob.setRange(
        1.0,
        6.0,
        1.0);
}
else
{
    knob.setRange(
        0.0,
        1.0,
        0.001);
}

addAndMakeVisible(knob);

}

//==============================================================
// LABEL
//==============================================================

void RG_Precision_DriveAudioProcessorEditor::setupLabel(
juce::Label& label,
const juce::String& text)
{
label.setText(
text,
juce::dontSendNotification);

label.setFont(
    juce::Font(
        10.0f,
        juce::Font::bold));

label.setColour(
    juce::Label::textColourId,
    juce::Colours::white);

label.setJustificationType(
    juce::Justification::centred);

label.setInterceptsMouseClicks(
    false,
    false);

addAndMakeVisible(label);

}

//==============================================================
// RESIZED
//==============================================================

void RG_Precision_DriveAudioProcessorEditor::resized()
{
//==========================================================
// FINAL EDITOR
// 600 x 660
//==========================================================

const float editorW =
    (float)getWidth();

const float editorH =
    (float)getHeight();

//==========================================================
// PEDAL
// 350 x 590
//==========================================================

const float pedalW = 350.0f;
const float pedalH = 590.0f;

const float pedalX =
    (editorW - pedalW) * 0.5f;

const float pedalY =
    (editorH - pedalH) * 0.5f;

const float centerX =
    pedalX + pedalW * 0.5f;

const float leftX =
    pedalX + pedalW * 0.285f;

const float rightX =
    pedalX + pedalW * 0.715f;

//==========================================================
// TOP CONTROL ROW
// VOLUME / BRIGHT
//==========================================================

const int topSize = 78;

volumeKnob.setBounds(
    juce::roundToInt(leftX - topSize * 0.5f),
    juce::roundToInt(pedalY + 70.0f),
    topSize,
    topSize);

brightKnob.setBounds(
    juce::roundToInt(rightX - topSize * 0.5f),
    juce::roundToInt(pedalY + 70.0f),
    topSize,
    topSize);

volumeLabel.setBounds(
    juce::roundToInt(leftX - 48.0f),
    juce::roundToInt(pedalY + 148.0f),
    96,
    18);

brightLabel.setBounds(
    juce::roundToInt(rightX - 48.0f),
    juce::roundToInt(pedalY + 148.0f),
    96,
    18);

//==========================================================
// SECOND CONTROL ROW
// ATTACK / GATE / DRIVE
//==========================================================

const int mainSize = 72;

attackKnob.setBounds(
    juce::roundToInt(leftX - mainSize * 0.5f),
    juce::roundToInt(pedalY + 170.0f),
    mainSize,
    mainSize);

gateKnob.setBounds(
    juce::roundToInt(centerX - 25.0f),
    juce::roundToInt(pedalY + 181.0f),
    50,
    50);

driveKnob.setBounds(
    juce::roundToInt(rightX - mainSize * 0.5f),
    juce::roundToInt(pedalY + 170.0f),
    mainSize,
    mainSize);

attackLabel.setBounds(
    juce::roundToInt(leftX - 48.0f),
    juce::roundToInt(pedalY + 242.0f),
    96,
    18);

gateLabel.setBounds(
    juce::roundToInt(centerX - 30.0f),
    juce::roundToInt(pedalY + 232.0f),
    60,
    18);

driveLabel.setBounds(
    juce::roundToInt(rightX - 48.0f),
    juce::roundToInt(pedalY + 242.0f),
    96,
    18);

//==========================================================
// INVISIBLE FOOTSWITCH CLICK AREA
//==========================================================

bypassButton.setBounds(
    juce::roundToInt(centerX - 65.0f),
    juce::roundToInt(pedalY + 415.0f),
    130,
    115);

}

//==============================================================
// MOUSE DOWN
//==============================================================

void RG_Precision_DriveAudioProcessorEditor::mouseDown(
const juce::MouseEvent& event)
{
AudioProcessorEditor::mouseDown(event);

footswitchPressed = true;

repaint();

}

//==============================================================
// MOUSE UP
//==============================================================

void RG_Precision_DriveAudioProcessorEditor::mouseUp(
const juce::MouseEvent& event)
{
AudioProcessorEditor::mouseUp(event);

footswitchPressed =
    bypassButton.getToggleState();

repaint();

}

//==============================================================
// TIMER
//==============================================================

void RG_Precision_DriveAudioProcessorEditor::timerCallback()
{
updateLED();

repaint();

}

//==============================================================
// LED
//==============================================================

void RG_Precision_DriveAudioProcessorEditor::updateLED()
{
const float level =
juce::jlimit(
0.0f,
1.0f,
audioProcessor.getOutputLevel());

gateGlowLevel =
    gateGlowLevel * 0.82f +
    level * 0.18f;

}

//==============================================================
// PAINT
//==============================================================

void RG_Precision_DriveAudioProcessorEditor::paint(
juce::Graphics& g)
{
//==========================================================
// OUTER BACKGROUND
//==========================================================

g.fillAll(
    juce::Colour(0xff080808));

//==========================================================
// PEDAL
//==========================================================

const float pedalW = 350.0f;
const float pedalH = 590.0f;

const float pedalX =
    ((float)getWidth() - pedalW) * 0.5f;

const float pedalY =
    ((float)getHeight() - pedalH) * 0.5f;

juce::Rectangle<float> pedal(
    pedalX,
    pedalY,
    pedalW,
    pedalH);

//==========================================================
// BODY
//==========================================================

juce::ColourGradient bodyGradient(
    juce::Colour(0xff3b3b3b),
    pedal.getX(),
    pedal.getY(),

    juce::Colour(0xff151515),
    pedal.getRight(),
    pedal.getBottom(),
    true);

g.setGradientFill(bodyGradient);

g.fillRoundedRectangle(
    pedal,
    10.0f);

//==========================================================
// POWDER COAT TEXTURE
//==========================================================

juce::Random textureRandom(
    0x7A31C9E5);

g.saveState();

g.reduceClipRegion(
    pedal.reduced(4.0f).toNearestInt());

for (int i = 0; i < 1200; ++i)
{
    const float x =
        pedal.getX() +
        textureRandom.nextFloat() *
        pedal.getWidth();

    const float y =
        pedal.getY() +
        textureRandom.nextFloat() *
        pedal.getHeight();

    const float r =
        0.25f +
        textureRandom.nextFloat() *
        0.95f;

    g.setColour(
        juce::Colours::black.withAlpha(
            0.035f +
            textureRandom.nextFloat() * 0.065f));

    g.fillEllipse(
        x - r,
        y - r,
        r * 2.0f,
        r * 2.0f);
}

for (int i = 0; i < 1400; ++i)
{
    const float x =
        pedal.getX() +
        textureRandom.nextFloat() *
        pedal.getWidth();

    const float y =
        pedal.getY() +
        textureRandom.nextFloat() *
        pedal.getHeight();

    const float r =
        0.15f +
        textureRandom.nextFloat() *
        0.55f;

    g.setColour(
        juce::Colours::white.withAlpha(
            0.02f +
            textureRandom.nextFloat() * 0.055f));

    g.fillEllipse(
        x - r,
        y - r,
        r * 2.0f,
        r * 2.0f);
}

g.restoreState();

//==========================================================
// BODY BORDER
//==========================================================

g.setColour(
    juce::Colour(0xff858585)
        .withAlpha(0.65f));

g.drawRoundedRectangle(
    pedal.reduced(0.8f),
    10.0f,
    1.5f);

g.setColour(
    juce::Colours::black
        .withAlpha(0.60f));

g.drawRoundedRectangle(
    pedal.reduced(7.0f),
    6.0f,
    1.0f);

//==========================================================
// SIDE INPUT / OUTPUT
// ACTUAL HORIZON-STYLE SIDE LABEL POSITION
//==========================================================

g.setFont(
    juce::Font(
        8.0f,
        juce::Font::bold));

g.setColour(
    juce::Colours::white.withAlpha(0.78f));

// OUT — LEFT
g.saveState();

g.addTransform(
    juce::AffineTransform::rotation(
        -juce::MathConstants<float>::halfPi,
        pedalX + 17.0f,
        pedalY + 295.0f));

g.drawText(
    "OUT",
    juce::roundToInt(pedalX - 5.0f),
    juce::roundToInt(pedalY + 286.0f),
    45,
    18,
    juce::Justification::centred,
    false);

g.restoreState();

// IN — RIGHT
g.saveState();

g.addTransform(
    juce::AffineTransform::rotation(
        juce::MathConstants<float>::halfPi,
        pedalX + pedalW - 17.0f,
        pedalY + 295.0f));

g.drawText(
    "IN",
    juce::roundToInt(pedalX + pedalW - 40.0f),
    juce::roundToInt(pedalY + 286.0f),
    35,
    18,
    juce::Justification::centred,
    false);

g.restoreState();

//==========================================================
// CENTRAL BRANDING
//==========================================================

g.setColour(
    juce::Colour(0xffe7e7e7));

g.setFont(
    juce::Font(
        17.0f,
        juce::Font::bold));

g.drawText(
    "RG PRECISION",
    juce::roundToInt(pedalX + 65.0f),
    juce::roundToInt(pedalY + 278.0f),
    juce::roundToInt(pedalW - 130.0f),
    22,
    juce::Justification::centred,
    false);

g.setFont(
    juce::Font(
        18.0f,
        juce::Font::bold));

g.drawText(
    "DRIVE",
    juce::roundToInt(pedalX + 85.0f),
    juce::roundToInt(pedalY + 299.0f),
    juce::roundToInt(pedalW - 170.0f),
    24,
    juce::Justification::centred,
    false);

//==========================================================
// SMALL BLUE ACCENT LINE
//==========================================================

g.setColour(
    juce::Colour(0xff168cff)
        .withAlpha(0.75f));

g.fillRoundedRectangle(
    pedalX + 125.0f,
    pedalY + 327.0f,
    100.0f,
    2.0f,
    1.0f);

//==========================================================
// LED
//==========================================================

const float ledX =
    pedalX + pedalW * 0.5f;

const float ledY =
    pedalY + 355.0f;

const float glow =
    juce::jlimit(
        0.15f,
        1.0f,
        gateGlowLevel + 0.20f);

for (int i = 5; i >= 1; --i)
{
    const float r =
        6.0f +
        (float)i * 4.0f;

    g.setColour(
        juce::Colour(0xff168cff)
            .withAlpha(0.025f * glow));

    g.fillEllipse(
        ledX - r,
        ledY - r,
        r * 2.0f,
        r * 2.0f);
}

g.setColour(
    juce::Colour(0xff299dff)
        .withAlpha(
            0.35f +
            0.65f * glow));

g.fillEllipse(
    ledX - 5.0f,
    ledY - 5.0f,
    10.0f,
    10.0f);

g.setColour(
    juce::Colours::white.withAlpha(0.85f));

g.fillEllipse(
    ledX - 1.7f,
    ledY - 2.5f,
    3.4f,
    2.5f);

//==========================================================
// METAL FOOTSWITCH
//==========================================================

const float switchCX =
    ledX;

const float switchCY =
    pedalY + 440.0f;

const float switchRadius =
    footswitchPressed
        ? 34.0f
        : 37.0f;

// Shadow
g.setColour(
    juce::Colours::black.withAlpha(0.78f));

g.fillEllipse(
    switchCX - switchRadius - 4.0f,
    switchCY - switchRadius - 4.0f,
    (switchRadius + 4.0f) * 2.0f,
    (switchRadius + 4.0f) * 2.0f);

// Metal body
juce::ColourGradient metal(
    juce::Colour(0xffeeeeee),
    switchCX - switchRadius * 0.65f,
    switchCY - switchRadius * 0.80f,

    juce::Colour(0xff565656),
    switchCX + switchRadius * 0.70f,
    switchCY + switchRadius * 0.85f,
    true);

g.setGradientFill(metal);

g.fillEllipse(
    switchCX - switchRadius,
    switchCY - switchRadius,
    switchRadius * 2.0f,
    switchRadius * 2.0f);

// Outer ring
g.setColour(
    juce::Colour(0xff222222));

g.drawEllipse(
    switchCX - switchRadius,
    switchCY - switchRadius,
    switchRadius * 2.0f,
    switchRadius * 2.0f,
    2.0f);

// Inner reflection
g.setColour(
    juce::Colours::white.withAlpha(0.28f));

g.drawEllipse(
    switchCX - switchRadius + 5.0f,
    switchCY - switchRadius + 5.0f,
    (switchRadius - 5.0f) * 2.0f,
    (switchRadius - 5.0f) * 2.0f,
    1.0f);

// Center shadow
g.setColour(
    juce::Colours::black.withAlpha(0.18f));

g.fillEllipse(
    switchCX - 9.0f,
    switchCY - 6.0f,
    18.0f,
    12.0f);

//==========================================================
// BOTTOM BRAND
//==========================================================

g.setColour(
    juce::Colour(0xffbcbcbc));

g.setFont(
    juce::Font(
        8.0f,
        juce::Font::bold));

g.drawText(
    "RG ELECTRONICS",
    juce::roundToInt(pedalX + 25.0f),
    juce::roundToInt(pedalY + 535.0f),
    juce::roundToInt(pedalW - 50.0f),
    18,
    juce::Justification::centred,
    false);

}
