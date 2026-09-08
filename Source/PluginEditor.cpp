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
    setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);

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
// KNOB PAINT
//==============================================================

void RG_Precision_DriveAudioProcessorEditor::PedalKnob::paint(
    juce::Graphics& g)
{
    const auto bounds =
        getLocalBounds().toFloat();

    const float cx = bounds.getCentreX();
    const float cy = bounds.getCentreY();

    const bool smallKnob =
        getWidth() <= 55;

    const float radius =
        juce::jmin(bounds.getWidth(),
                   bounds.getHeight()) * 0.5f
        - 2.0f;

    if (smallKnob)
    {
        juce::ColourGradient crystal(
            juce::Colours::white.withAlpha(0.30f),
            cx - radius * 0.7f,
            cy - radius * 0.8f,

            juce::Colours::lightgrey.withAlpha(0.07f),
            cx + radius,
            cy + radius,
            true);

        g.setGradientFill(crystal);

        g.fillEllipse(
            cx - radius,
            cy - radius,
            radius * 2.0f,
            radius * 2.0f);

        g.setColour(
            juce::Colours::white.withAlpha(0.75f));

        g.drawEllipse(
            cx - radius,
            cy - radius,
            radius * 2.0f,
            radius * 2.0f,
            1.4f);

        g.setColour(
            juce::Colours::white.withAlpha(0.22f));

        g.fillEllipse(
            cx - radius * 0.58f,
            cy - radius * 0.58f,
            radius * 1.16f,
            radius * 1.16f);

        const float value =
            (float)getValue();

        const float angle =
            knobStartAngle
            + value * (knobEndAngle - knobStartAngle);

        const float pointerLength =
            radius * 0.72f;

        g.setColour(
            juce::Colours::white.withAlpha(0.95f));

        g.drawLine(
            cx,
            cy,
            cx + std::cos(angle) * pointerLength,
            cy + std::sin(angle) * pointerLength,
            2.0f);

        return;
    }

    juce::ColourGradient metal(
        juce::Colours::grey.brighter(0.55f),
        cx - radius,
        cy - radius,

        juce::Colours::black,
        cx + radius,
        cy + radius,
        true);

    g.setGradientFill(metal);

    g.fillEllipse(
        cx - radius,
        cy - radius,
        radius * 2.0f,
        radius * 2.0f);

    g.setColour(
        juce::Colours::black);

    g.drawEllipse(
        cx - radius,
        cy - radius,
        radius * 2.0f,
        radius * 2.0f,
        2.0f);

    const float innerRadius =
        radius * 0.78f;

    g.setColour(
        juce::Colours::white.withAlpha(0.10f));

    g.drawEllipse(
        cx - innerRadius,
        cy - innerRadius,
        innerRadius * 2.0f,
        innerRadius * 2.0f,
        1.2f);

    const float value =
        (float)getValue();

    const float angle =
        knobStartAngle
        + value * (knobEndAngle - knobStartAngle);

    const float pointerLength =
        radius * 0.70f;

    g.setColour(
        juce::Colours::white);

    g.drawLine(
        cx,
        cy,
        cx + std::cos(angle) * pointerLength,
        cy + std::sin(angle) * pointerLength,
        2.5f);
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
    setSize(600, 660);
    setResizable(false, false);

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

    setupLabel(volumeLabel, "VOLUME");
    setupLabel(brightLabel, "BRIGHT");
    setupLabel(attackLabel, "ATTACK");
    setupLabel(driveLabel, "DRIVE");
    setupLabel(gateLabel, "GATE");

    bypassButton.setButtonText({});
    bypassButton.setClickingTogglesState(true);
    bypassButton.setAlpha(0.001f);

    bypassButton.onStateChange =
        [this]
        {
            footswitchPressed =
                bypassButton.getToggleState();

            updateLED();
            repaint();
        };

    addAndMakeVisible(bypassButton);

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

    led.setText(
        {},
        juce::dontSendNotification);

    led.setColour(
        juce::Label::backgroundColourId,
        juce::Colours::transparentBlack);

    addAndMakeVisible(led);

    startTimerHz(30);
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

    if (selector)
    {
        knob.setRange(
            1.0,
            6.0,
            1.0);

        knob.setNumDecimalPlacesToDisplay(0);
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
            "Arial",
            11.0f,
            juce::Font::bold));

    label.setColour(
        juce::Label::textColourId,
        juce::Colours::white.withAlpha(0.92f));

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
    const float editorW =
        (float)getWidth();

    const float editorH =
        (float)getHeight();

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
        20);

    brightLabel.setBounds(
        juce::roundToInt(rightX - 48.0f),
        juce::roundToInt(pedalY + 148.0f),
        96,
        20);

    const int mainSize = 72;

    attackKnob.setBounds(
        juce::roundToInt(leftX - mainSize * 0.5f),
        juce::roundToInt(pedalY + 170.0f),
        mainSize,
        mainSize);

    driveKnob.setBounds(
        juce::roundToInt(rightX - mainSize * 0.5f),
        juce::roundToInt(pedalY + 170.0f),
        mainSize,
        mainSize);

    const int gateSize = 50;

    gateKnob.setBounds(
        juce::roundToInt(centerX - gateSize * 0.5f),
        juce::roundToInt(pedalY + 181.0f),
        gateSize,
        gateSize);

    attackLabel.setBounds(
        juce::roundToInt(leftX - 48.0f),
        juce::roundToInt(pedalY + 242.0f),
        96,
        20);

    gateLabel.setBounds(
        juce::roundToInt(centerX - 42.0f),
        juce::roundToInt(pedalY + 232.0f),
        84,
        20);

    driveLabel.setBounds(
        juce::roundToInt(rightX - 48.0f),
        juce::roundToInt(pedalY + 242.0f),
        96,
        20);

    bypassButton.setBounds(
        juce::roundToInt(centerX - 65.0f),
        juce::roundToInt(pedalY + 415.0f),
        130,
        115);

    led.setBounds(
        juce::roundToInt(centerX - 16.0f),
        juce::roundToInt(pedalY + 338.0f),
        32,
        32);
}

//==============================================================
// LED STATE
//==============================================================

void RG_Precision_DriveAudioProcessorEditor::updateLED()
{
    repaint();
}

//==============================================================
// PAINT
//==============================================================

void RG_Precision_DriveAudioProcessorEditor::paint(
    juce::Graphics& g)
{
    g.fillAll(
        juce::Colour(18, 18, 18));

    const float editorW =
        (float)getWidth();

    const float editorH =
        (float)getHeight();

    const float pedalW = 350.0f;
    const float pedalH = 590.0f;

    const float pedalX =
        (editorW - pedalW) * 0.5f;

    const float pedalY =
        (editorH - pedalH) * 0.5f;

    //==========================================================
    // FIX: CENTER X
    //==========================================================

    const float centerX =
        pedalX + pedalW * 0.5f;

    //==========================================================
    // PEDAL SHADOW
    //==========================================================

    g.setColour(
        juce::Colours::black.withAlpha(0.65f));

    g.fillRoundedRectangle(
        pedalX + 5.0f,
        pedalY + 7.0f,
        pedalW,
        pedalH,
        9.0f);

    //==========================================================
    // POWDER-COAT BODY
    //==========================================================

    juce::ColourGradient bodyGradient(
        juce::Colour(52, 52, 52),
        pedalX,
        pedalY,

        juce::Colour(20, 20, 20),
        pedalX + pedalW,
        pedalY + pedalH,
        false);

    g.setGradientFill(bodyGradient);

    g.fillRoundedRectangle(
        pedalX,
        pedalY,
        pedalW,
        pedalH,
        8.0f);

    //==========================================================
    // ROUGH POWDER-COAT TEXTURE
    //==========================================================

    juce::Random textureRandom(0x7A31C9E5);

    g.saveState();

    g.reduceClipRegion(
        juce::Rectangle<int>(
            juce::roundToInt(pedalX + 3.0f),
            juce::roundToInt(pedalY + 3.0f),
            juce::roundToInt(pedalW - 6.0f),
            juce::roundToInt(pedalH - 6.0f)));

    for (int i = 0; i < 1200; ++i)
    {
        const float x =
            pedalX + textureRandom.nextFloat() * pedalW;

        const float y =
            pedalY + textureRandom.nextFloat() * pedalH;

        const float r =
            0.3f + textureRandom.nextFloat() * 1.3f;

        g.setColour(
            juce::Colours::black.withAlpha(
                0.08f + textureRandom.nextFloat() * 0.12f));

        g.fillEllipse(
            x,
            y,
            r,
            r);
    }

    for (int i = 0; i < 1400; ++i)
    {
        const float x =
            pedalX + textureRandom.nextFloat() * pedalW;

        const float y =
            pedalY + textureRandom.nextFloat() * pedalH;

        const float r =
            0.25f + textureRandom.nextFloat() * 1.0f;

        g.setColour(
            juce::Colours::white.withAlpha(
                0.025f + textureRandom.nextFloat() * 0.055f));

        g.fillEllipse(
            x,
            y,
            r,
            r);
    }

    g.restoreState();

    //==========================================================
    // BODY EDGE
    //==========================================================

    g.setColour(
        juce::Colours::black.withAlpha(0.9f));

    g.drawRoundedRectangle(
        pedalX,
        pedalY,
        pedalW,
        pedalH,
        8.0f,
        2.0f);

    //==========================================================
    // SIDE LABELS
    //==========================================================

    g.saveState();

    g.setFont(
        juce::Font(
            "Arial",
            12.0f,
            juce::Font::bold));

    g.setColour(
        juce::Colours::white.withAlpha(0.82f));

    g.drawText(
        "OUT",
        juce::roundToInt(pedalX + 5.0f),
        juce::roundToInt(pedalY + 270.0f),
        30,
        55,
        juce::Justification::centred,
        false);

    g.drawText(
        "IN",
        juce::roundToInt(pedalX + pedalW - 35.0f),
        juce::roundToInt(pedalY + 270.0f),
        30,
        55,
        juce::Justification::centred,
        false);

    g.restoreState();

    //==========================================================
    // CENTRAL BRANDING
    //==========================================================

    g.setColour(
        juce::Colours::white.withAlpha(0.94f));

    g.setFont(
        juce::Font(
            "Arial",
            18.0f,
            juce::Font::bold));

    g.drawText(
        "RG PRECISION DRIVE",
        juce::roundToInt(centerX - 125.0f),
        juce::roundToInt(pedalY + 278.0f),
        250,
        28,
        juce::Justification::centred,
        false);

    g.setColour(
        juce::Colour(50, 110, 255).withAlpha(0.75f));

    g.fillRoundedRectangle(
        centerX - 65.0f,
        pedalY + 319.0f,
        130.0f,
        2.0f,
        1.0f);

    //==========================================================
    // BLUE LED
    //==========================================================

    const float ledX =
        centerX;

    const float ledY =
        pedalY + 355.0f;

    const float ledRadius =
        footswitchPressed ? 8.0f : 6.5f;

    if (footswitchPressed)
    {
        g.setColour(
            juce::Colour(40, 120, 255).withAlpha(0.20f));

        g.fillEllipse(
            ledX - 15.0f,
            ledY - 15.0f,
            30.0f,
            30.0f);
    }

    g.setColour(
        footswitchPressed
            ? juce::Colour(45, 135, 255)
            : juce::Colour(20, 45, 75));

    g.fillEllipse(
        ledX - ledRadius,
        ledY - ledRadius,
        ledRadius * 2.0f,
        ledRadius * 2.0f);

    g.setColour(
        juce::Colours::white.withAlpha(
            footswitchPressed ? 0.65f : 0.20f));

    g.fillEllipse(
        ledX - ledRadius * 0.35f,
        ledY - ledRadius * 0.55f,
        ledRadius * 0.45f,
        ledRadius * 0.35f);

    //==========================================================
    // METAL FOOTSWITCH
    //==========================================================

    const float switchCX =
        centerX;

    const float switchCY =
        pedalY + 440.0f;

    const float switchRadius =
        footswitchPressed ? 34.0f : 37.0f;

    g.setColour(
        juce::Colours::black.withAlpha(0.75f));

    g.fillEllipse(
        switchCX - switchRadius - 2.0f,
        switchCY - switchRadius + 5.0f,
        (switchRadius + 2.0f) * 2.0f,
        (switchRadius + 2.0f) * 2.0f);

    g.setColour(
        juce::Colour(15, 15, 15));

    g.fillEllipse(
        switchCX - switchRadius,
        switchCY - switchRadius,
        switchRadius * 2.0f,
        switchRadius * 2.0f);

    juce::ColourGradient metalSwitch(
        juce::Colour(220, 220, 220),
        switchCX - switchRadius,
        switchCY - switchRadius,

        juce::Colour(70, 70, 70),
        switchCX + switchRadius,
        switchCY + switchRadius,
        true);

    g.setGradientFill(metalSwitch);

    g.fillEllipse(
        switchCX - switchRadius + 4.0f,
        switchCY - switchRadius + 4.0f,
        (switchRadius - 4.0f) * 2.0f,
        (switchRadius - 4.0f) * 2.0f);

    const float inner =
        switchRadius * 0.66f;

    juce::ColourGradient innerMetal(
        juce::Colour(180, 180, 180),
        switchCX - inner,
        switchCY - inner,

        juce::Colour(55, 55, 55),
        switchCX + inner,
        switchCY + inner,
        true);

    g.setGradientFill(innerMetal);

    g.fillEllipse(
        switchCX - inner,
        switchCY - inner,
        inner * 2.0f,
        inner * 2.0f);

    g.setColour(
        juce::Colours::white.withAlpha(0.28f));

    g.fillEllipse(
        switchCX - inner * 0.45f,
        switchCY - inner * 0.55f,
        inner * 0.65f,
        inner * 0.35f);

    g.setColour(
        juce::Colours::black.withAlpha(0.25f));

    g.fillEllipse(
        switchCX - inner * 0.18f,
        switchCY - inner * 0.18f,
        inner * 0.36f,
        inner * 0.36f);

    //==========================================================
    // BOTTOM BRAND
    //==========================================================

    g.setColour(
        juce::Colours::white.withAlpha(0.78f));

    g.setFont(
        juce::Font(
            "Arial",
            13.0f,
            juce::Font::bold));

    g.drawText(
        "RG ELECTRONICS",
        juce::roundToInt(centerX - 100.0f),
        juce::roundToInt(pedalY + 535.0f),
        200,
        22,
        juce::Justification::centred,
        false);
}

//==============================================================
// MOUSE DOWN
//==============================================================

void RG_Precision_DriveAudioProcessorEditor::mouseDown(
    const juce::MouseEvent& e)
{
    AudioProcessorEditor::mouseDown(e);
}

//==============================================================
// MOUSE UP
//==============================================================

void RG_Precision_DriveAudioProcessorEditor::mouseUp(
    const juce::MouseEvent& e)
{
    AudioProcessorEditor::mouseUp(e);
}

//==============================================================
// TIMER
//==============================================================

void RG_Precision_DriveAudioProcessorEditor::timerCallback()
{
    gateGlowLevel =
        juce::jlimit(
            0.0f,
            1.0f,
            audioProcessor.getOutputLevel());

    repaint();
}
