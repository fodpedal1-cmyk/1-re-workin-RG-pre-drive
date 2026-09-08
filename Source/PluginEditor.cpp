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

    const auto centre =
        knobArea.getCentre();

    const float radius =
        juce::jmin(
            knobArea.getWidth(),
            knobArea.getHeight()) * 0.5f;

    //==========================================================
    // GATE CRYSTAL KNOB
    //==========================================================

    if (smallKnob)
    {
        juce::ColourGradient crystal(
            juce::Colour(0xffe8f4ff).withAlpha(0.34f),
            centre.x - radius * 0.45f,
            centre.y - radius * 0.65f,

            juce::Colour(0xff567080).withAlpha(0.22f),
            centre.x + radius * 0.60f,
            centre.y + radius * 0.70f,
            true);

        g.setGradientFill(crystal);

        g.fillEllipse(knobArea);

        g.setColour(
            juce::Colours::white.withAlpha(0.58f));

        g.drawEllipse(
            knobArea,
            1.4f);

        g.setColour(
            juce::Colours::white.withAlpha(0.20f));

        g.fillEllipse(
            knobArea.reduced(
                radius * 0.28f));

        // Pointer
        const float value =
            (float) getNormalisableRange()
                .convertTo0to1(getValue());

        const float angle =
            knobStartAngle +
            value *
            (knobEndAngle - knobStartAngle);

        juce::Point<float> p1(
            centre.x +
                std::cos(angle) *
                radius * 0.22f,

            centre.y +
                std::sin(angle) *
                radius * 0.22f);

        juce::Point<float> p2(
            centre.x +
                std::cos(angle) *
                radius * 0.72f,

            centre.y +
                std::sin(angle) *
                radius * 0.72f);

        g.setColour(
            juce::Colours::white.withAlpha(0.90f));

        g.drawLine(
            p1.x,
            p1.y,
            p2.x,
            p2.y,
            1.8f);

        return;
    }

    //==========================================================
    // MAIN BLACK METAL KNOB
    //==========================================================

    juce::ColourGradient knobGradient(
        juce::Colour(0xff303030),
        centre.x - radius * 0.55f,
        centre.y - radius * 0.70f,

        juce::Colour(0xff050505),
        centre.x + radius * 0.65f,
        centre.y + radius * 0.75f,
        true);

    g.setGradientFill(knobGradient);

    g.fillEllipse(knobArea);

    // Outer rim
    g.setColour(
        juce::Colour(0xff8b8b8b)
            .withAlpha(0.75f));

    g.drawEllipse(
        knobArea,
        1.3f);

    // Inner ring
    g.setColour(
        juce::Colour(0xff111111));

    g.drawEllipse(
        knobArea.reduced(
            radius * 0.12f),
        1.0f);

    //==========================================================
    // HIGHLIGHT
    //==========================================================

    g.setColour(
        juce::Colours::white.withAlpha(0.10f));

    g.drawEllipse(
        knobArea.reduced(2.0f),
        1.5f);

    //==========================================================
    // POINTER
    //==========================================================

    const float value =
        (float) getNormalisableRange()
            .convertTo0to1(getValue());

    const float angle =
        knobStartAngle +
        value *
        (knobEndAngle - knobStartAngle);

    juce::Point<float> p1(
        centre.x +
            std::cos(angle) *
            radius * 0.25f,

        centre.y +
            std::sin(angle) *
            radius * 0.25f);

    juce::Point<float> p2(
        centre.x +
            std::cos(angle) *
            radius * 0.72f,

        centre.y +
            std::sin(angle) *
            radius * 0.72f);

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
    setSize(
        editorWidth,
        editorHeight);

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

    setupLabel(
        volumeLabel,
        "VOLUME");

    setupLabel(
        brightLabel,
        "BRIGHT");

    setupLabel(
        attackLabel,
        "ATTACK");

    setupLabel(
        driveLabel,
        "DRIVE");

    setupLabel(
        gateLabel,
        "GATE");

    //==========================================================
    // FOOTSWITCH BUTTON
    //==========================================================

    bypassButton.setButtonText("");

    bypassButton.setClickingTogglesState(
        true);

    bypassButton.setAlpha(0.001f);

    bypassButton.setMouseCursor(
        juce::MouseCursor::PointingHandCursor);

    addAndMakeVisible(
        bypassButton);

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
    // TIMER
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
// SETUP LABEL
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
            13.0f,
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
    const int pedalX = 20;
    const int pedalY = 25;
    const int pedalW = 460;
    const int pedalH = 610;

    //==========================================================
    // TOP
    //==========================================================

    volumeKnob.setBounds(
        pedalX + 55,
        pedalY + 72,
        115,
        115);

    brightKnob.setBounds(
        pedalX + 290,
        pedalY + 72,
        115,
        115);

    volumeLabel.setBounds(
        pedalX + 45,
        pedalY + 177,
        135,
        25);

    brightLabel.setBounds(
        pedalX + 280,
        pedalY + 177,
        135,
        25);

    //==========================================================
    // MIDDLE
    //==========================================================

    attackKnob.setBounds(
        pedalX + 58,
        pedalY + 225,
        110,
        110);

    gateKnob.setBounds(
        pedalX + 205,
        pedalY + 250,
        58,
        58);

    driveKnob.setBounds(
        pedalX + 292,
        pedalY + 225,
        110,
        110);

    attackLabel.setBounds(
        pedalX + 45,
        pedalY + 330,
        135,
        25);

    gateLabel.setBounds(
        pedalX + 195,
        pedalY + 300,
        78,
        25);

    driveLabel.setBounds(
        pedalX + 280,
        pedalY + 330,
        135,
        25);

    //==========================================================
    // FOOTSWITCH CLICK AREA
    //==========================================================

    bypassButton.setBounds(
        pedalX + 160,
        pedalY + 440,
        140,
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
// UPDATE LED
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
    // BACKGROUND
    //==========================================================

    g.fillAll(
        juce::Colour(0xff050505));

    //==========================================================
    // PEDAL
    //==========================================================

    const float pedalX = 20.0f;
    const float pedalY = 25.0f;
    const float pedalW = 460.0f;
    const float pedalH = 610.0f;

    juce::Rectangle<float> pedal(
        pedalX,
        pedalY,
        pedalW,
        pedalH);

    //==========================================================
    // BODY
    //==========================================================

    juce::ColourGradient bodyGradient(
        juce::Colour(0xff353535),
        pedal.getX(),
        pedal.getY(),

        juce::Colour(0xff151515),
        pedal.getRight(),
        pedal.getBottom(),
        true);

    g.setGradientFill(
        bodyGradient);

    g.fillRoundedRectangle(
        pedal,
        9.0f);

    //==========================================================
    // ORANGE PEEL TEXTURE
    //==========================================================

    juce::Random textureRandom(
        0x7A31C9E5);

    g.saveState();

    g.reduceClipRegion(
        pedal.reduced(4.0f)
            .toNearestInt());

    // Dark pits
    for (int i = 0; i < 900; ++i)
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
            1.15f;

        const float alpha =
            0.035f +
            textureRandom.nextFloat() *
            0.075f;

        g.setColour(
            juce::Colours::black
                .withAlpha(alpha));

        g.fillEllipse(
            x - r,
            y - r,
            r * 2.0f,
            r * 2.0f);
    }

    // Raised bumps
    for (int i = 0; i < 1100; ++i)
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
            0.65f;

        const float alpha =
            0.025f +
            textureRandom.nextFloat() *
            0.065f;

        g.setColour(
            juce::Colours::white
                .withAlpha(alpha));

        g.fillEllipse(
            x - r,
            y - r,
            r * 2.0f,
            r * 2.0f);
    }

    g.restoreState();

    //==========================================================
    // OUTER BORDER
    //==========================================================

    g.setColour(
        juce::Colour(0xff777777)
            .withAlpha(0.65f));

    g.drawRoundedRectangle(
        pedal.reduced(0.8f),
        9.0f,
        1.5f);

    //==========================================================
    // INNER BORDER
    //==========================================================

    g.setColour(
        juce::Colours::black
            .withAlpha(0.55f));

    g.drawRoundedRectangle(
        pedal.reduced(7.0f),
        5.0f,
        1.0f);

    //==========================================================
    // TITLE
    //==========================================================

    g.setColour(
        juce::Colours::white);

    g.setFont(
        juce::Font(
            22.0f,
            juce::Font::bold));

    g.drawText(
        "RG PRECISION DRIVE",
        (int) (pedalX + 25.0f),
        (int) (pedalY + 20.0f),
        (int) (pedalW - 50.0f),
        32,
        juce::Justification::centred,
        false);

    //==========================================================
    // BRAND
    //==========================================================

    g.setColour(
        juce::Colour(0xffbcbcbc));

    g.setFont(
        juce::Font(
            10.0f,
            juce::Font::bold));

    g.drawText(
        "RG ELECTRONICS",
        (int) (pedalX + 25.0f),
        (int) (pedalY + pedalH - 30.0f),
        (int) (pedalW - 50.0f),
        18,
        juce::Justification::centred,
        false);

    //==========================================================
    // IN / OUT
    //==========================================================

    g.setFont(
        juce::Font(
            9.0f,
            juce::Font::bold));

    g.setColour(
        juce::Colours::white
            .withAlpha(0.70f));

    g.drawText(
        "IN",
        (int) (pedalX + 16.0f),
        (int) (pedalY + 330.0f),
        40,
        18,
        juce::Justification::centred,
        false);

    g.drawText(
        "OUT",
        (int) (pedalX + pedalW - 56.0f),
        (int) (pedalY + 330.0f),
        40,
        18,
        juce::Justification::centred,
        false);

    //==========================================================
    // BLUE LED GLOW
    //==========================================================

    const float ledX =
        pedalX + pedalW * 0.5f;

    const float ledY =
        pedalY + 398.0f;

    const float glow =
        juce::jlimit(
            0.15f,
            1.0f,
            gateGlowLevel + 0.20f);

    for (int i = 5; i >= 1; --i)
    {
        const float r =
            8.0f +
            (float) i * 5.0f;

        g.setColour(
            juce::Colour(0xff168cff)
                .withAlpha(
                    0.025f * glow));

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
        ledX - 6.0f,
        ledY - 6.0f,
        12.0f,
        12.0f);

    g.setColour(
        juce::Colours::white
            .withAlpha(0.85f));

    g.fillEllipse(
        ledX - 2.0f,
        ledY - 3.0f,
        4.0f,
        3.0f);

    //==========================================================
    // FOOTSWITCH
    //==========================================================

    const float switchCX =
        pedalX + pedalW * 0.5f;

    const float switchCY =
        pedalY + 500.0f;

    const float switchRadius =
        footswitchPressed
            ? 37.0f
            : 40.0f;

    // Shadow
    g.setColour(
        juce::Colours::black
            .withAlpha(0.75f));

    g.fillEllipse(
        switchCX -
            switchRadius -
            4.0f,

        switchCY -
            switchRadius -
            4.0f,

        (switchRadius + 4.0f) * 2.0f,
        (switchRadius + 4.0f) * 2.0f);

    // Metal
    juce::ColourGradient metal(
        juce::Colour(0xffeeeeee),
        switchCX -
            switchRadius * 0.65f,
        switchCY -
            switchRadius * 0.80f,

        juce::Colour(0xff565656),
        switchCX +
            switchRadius * 0.70f,
        switchCY +
            switchRadius * 0.85f,
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

    // Inner ring
    g.setColour(
        juce::Colours::white
            .withAlpha(0.28f));

    g.drawEllipse(
        switchCX -
            switchRadius +
            5.0f,

        switchCY -
            switchRadius +
            5.0f,

        (switchRadius - 5.0f) * 2.0f,
        (switchRadius - 5.0f) * 2.0f,

        1.0f);

    //==========================================================
    // TOP METAL REFLECTION
    //==========================================================
    //
    // drawArc() removed because JUCE Graphics does not
    // provide drawArc(). Use a thin ellipse instead.
    //==========================================================

    g.setColour(
        juce::Colours::white
            .withAlpha(0.30f));

    g.drawEllipse(
        juce::Rectangle<float>(
            switchCX -
                switchRadius +
                4.0f,

            switchCY -
                switchRadius +
                4.0f,

            (switchRadius - 4.0f) * 2.0f,
            (switchRadius - 4.0f) * 2.0f),
        2.0f);

    //==========================================================
    // SWITCH CENTER
    //==========================================================

    g.setColour(
        juce::Colours::black
            .withAlpha(0.18f));

    g.fillEllipse(
        switchCX - 10.0f,
        switchCY - 7.0f,
        20.0f,
        14.0f);
}
