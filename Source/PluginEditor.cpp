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

    constexpr int pedalX = 20;
    constexpr int pedalY = 25;
    constexpr int pedalW = 460;
    constexpr int pedalH = 610;

    juce::Image orangePeelTexture;

//==============================================================
// ORANGE PEEL / POWDER COAT TEXTURE
//==============================================================

    void createOrangePeelTexture()
    {
        if (orangePeelTexture.isValid())
            return;

        orangePeelTexture =
            juce::Image(
                juce::Image::ARGB,
                pedalW,
                pedalH,
                true);

        juce::Graphics tg(orangePeelTexture);

        tg.fillAll(juce::Colour(0xff191919));

        juce::Random random(0x71A93C2D);

        // Large irregular pits
        for (int i = 0; i < 9000; ++i)
        {
            const float x =
                random.nextFloat() * pedalW;

            const float y =
                random.nextFloat() * pedalH;

            const float r =
                0.25f + random.nextFloat() * 1.35f;

            const float alpha =
                0.035f + random.nextFloat() * 0.08f;

            tg.setColour(
                juce::Colours::black.withAlpha(alpha));

            tg.fillEllipse(
                x - r,
                y - r,
                r * 2.0f,
                r * 2.0f);
        }

        // Tiny raised bumps
        for (int i = 0; i < 7500; ++i)
        {
            const float x =
                random.nextFloat() * pedalW;

            const float y =
                random.nextFloat() * pedalH;

            const float r =
                0.15f + random.nextFloat() * 0.8f;

            const float alpha =
                0.025f + random.nextFloat() * 0.055f;

            tg.setColour(
                juce::Colours::white.withAlpha(alpha));

            tg.fillEllipse(
                x - r,
                y - r,
                r * 2.0f,
                r * 2.0f);
        }

        // Micro relief
        for (int i = 0; i < 4500; ++i)
        {
            const float x =
                random.nextFloat() * pedalW;

            const float y =
                random.nextFloat() * pedalH;

            tg.setColour(
                juce::Colours::black.withAlpha(0.035f));

            tg.fillRect(
                juce::roundToInt(x),
                juce::roundToInt(y),
                1,
                1);
        }
    }
}

//==============================================================
// PEDAL KNOB
//==============================================================

RG_Precision_DriveAudioProcessorEditor::PedalKnob::
PedalKnob()
{
    setSliderStyle(
        juce::Slider::RotaryHorizontalVerticalDrag);

    setTextBoxStyle(
        juce::Slider::NoTextBox,
        false,
        0,
        0);

    setRange(
        0.0,
        1.0,
        0.001);

    setRotaryParameters(
        knobStartAngle,
        knobEndAngle,
        true);

    setDoubleClickReturnValue(
        true,
        0.5);
}

//==============================================================
// PEDAL KNOB PAINT
//==============================================================

void RG_Precision_DriveAudioProcessorEditor::PedalKnob::
paint(juce::Graphics& g)
{
    auto area =
        getLocalBounds().toFloat();

    const float size =
        juce::jmin(
            area.getWidth(),
            area.getHeight());

    const float cx =
        area.getCentreX();

    const float cy =
        area.getCentreY();

    const float radius =
        size * 0.42f;

    //==========================================================
    // SHADOW
    //==========================================================

    g.setColour(
        juce::Colours::black.withAlpha(0.85f));

    g.fillEllipse(
        cx - radius + 3.0f,
        cy - radius + 5.0f,
        radius * 2.0f,
        radius * 2.0f);

    //==========================================================
    // OUTER METAL EDGE
    //==========================================================

    g.setColour(
        juce::Colour(0xff464646));

    g.fillEllipse(
        cx - radius,
        cy - radius,
        radius * 2.0f,
        radius * 2.0f);

    //==========================================================
    // BLACK KNOB BODY
    //==========================================================

    const float body =
        radius * 0.94f;

    g.setColour(
        juce::Colour(0xff171717));

    g.fillEllipse(
        cx - body,
        cy - body,
        body * 2.0f,
        body * 2.0f);

    //==========================================================
    // INNER SURFACE
    //==========================================================

    const float inner =
        radius * 0.84f;

    g.setColour(
        juce::Colour(0xff080808));

    g.fillEllipse(
        cx - inner,
        cy - inner,
        inner * 2.0f,
        inner * 2.0f);

    //==========================================================
    // SUBTLE HIGHLIGHT
    //==========================================================

    g.setColour(
        juce::Colours::white.withAlpha(0.06f));

    g.fillEllipse(
        cx - inner * 0.72f,
        cy - inner * 0.72f,
        inner * 1.44f,
        inner * 1.44f);

    //==========================================================
    // POINTER
    //==========================================================

    const double value =
        juce::jlimit(
            0.0,
            1.0,
            getValue());

    const double angle =
        static_cast<double>(knobStartAngle)
        + value *
        (static_cast<double>(knobEndAngle)
         - static_cast<double>(knobStartAngle));

    const float pointerLength =
        radius * 0.70f;

    juce::Line<float> pointer(
        cx,
        cy,
        cx + std::cos(angle) * pointerLength,
        cy + std::sin(angle) * pointerLength);

    g.setColour(
        juce::Colours::white);

    g.drawLine(
        pointer,
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
    createOrangePeelTexture();

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
    // BYPASS BUTTON
    //==========================================================

    addAndMakeVisible(
        bypassButton);

    bypassButton.setButtonText(
        "");

    bypassButton.setClickingTogglesState(
        true);

    bypassButton.setAlpha(
        0.001f);

    bypassButton.setComponentID(
        "5lxwj3");

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
                audioProcessor.apvts,
                "VOLUME",
                volumeKnob);

    brightAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>(
                audioProcessor.apvts,
                "BRIGHT",
                brightKnob);

    attackAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>(
                audioProcessor.apvts,
                "ATTACK",
                attackKnob);

    driveAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>(
                audioProcessor.apvts,
                "DRIVE",
                driveKnob);

    gateAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>(
                audioProcessor.apvts,
                "GATE",
                gateKnob);

    bypassAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::ButtonAttachment>(
                audioProcessor.apvts,
                "BYPASS",
                bypassButton);

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

void RG_Precision_DriveAudioProcessorEditor::
setupKnob(
    PedalKnob& knob,
    juce::Slider::SliderStyle style,
    bool selector)
{
    addAndMakeVisible(
        knob);

    knob.setSliderStyle(
        style);

    knob.setTextBoxStyle(
        juce::Slider::NoTextBox,
        false,
        0,
        0);

    //==========================================================
    // NORMALIZED RANGE
    //==========================================================

    if (selector)
    {
        // ATTACK remains a 1–6 selector.
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

    knob.setRotaryParameters(
        knobStartAngle,
        knobEndAngle,
        true);

    knob.setDoubleClickReturnValue(
        true,
        selector ? 1.0 : 0.5);

    knob.setComponentID(
        selector ? "ATTACK" : "");
}

//==============================================================
// SETUP LABEL
//==============================================================

void RG_Precision_DriveAudioProcessorEditor::
setupLabel(
    juce::Label& label,
    const juce::String& text)
{
    addAndMakeVisible(
        label);

    label.setText(
        text,
        juce::dontSendNotification);

    label.setFont(
        juce::Font(
            "Arial",
            15.0f,
            juce::Font::bold));

    label.setColour(
        juce::Label::textColourId,
        juce::Colours::white);

    label.setJustificationType(
        juce::Justification::centred);

    label.setInterceptsMouseClicks(
        false,
        false);
}

//==============================================================
// RESIZED
//==============================================================

void RG_Precision_DriveAudioProcessorEditor::
resized()
{
    //==========================================================
    // TOP ROW
    //==========================================================

    volumeLabel.setBounds(
        72,
        92,
        125,
        22);

    volumeKnob.setBounds(
        75,
        115,
        120,
        120);

    brightLabel.setBounds(
        303,
        92,
        125,
        22);

    brightKnob.setBounds(
        306,
        115,
        120,
        120);

    //==========================================================
    // MIDDLE ROW
    //==========================================================

    attackLabel.setBounds(
        60,
        260,
        135,
        22);

    attackKnob.setBounds(
        68,
        282,
        120,
        120);

    //==========================================================
    // GATE
    //==========================================================

    gateKnob.setBounds(
        221,
        305,
        58,
        58);

    gateLabel.setBounds(
        199,
        365,
        102,
        22);

    //==========================================================
    // DRIVE
    //==========================================================

    driveLabel.setBounds(
        305,
        260,
        135,
        22);

    driveKnob.setBounds(
        308,
        282,
        120,
        120);

    //==========================================================
    // FOOTSWITCH
    //==========================================================

    bypassButton.setBounds(
        185,
        515,
        130,
        105);
}

//==============================================================
// PAINT
//==============================================================

void RG_Precision_DriveAudioProcessorEditor::
paint(
    juce::Graphics& g)
{
    //==========================================================
    // PURE BLACK BACKGROUND
    //==========================================================

    g.fillAll(
        juce::Colours::black);

    const juce::Rectangle<float> pedal(
        static_cast<float>(pedalX),
        static_cast<float>(pedalY),
        static_cast<float>(pedalW),
        static_cast<float>(pedalH));

    //==========================================================
    // PEDAL BASE
    //==========================================================

    g.setColour(
        juce::Colour(0xff191919));

    g.fillRoundedRectangle(
        pedal,
        7.0f);

    //==========================================================
    // ORANGE PEEL TEXTURE
    //==========================================================

    g.saveState();

    g.reduceClipRegion(
        pedal.toNearestInt());

    g.drawImage(
        orangePeelTexture,
        pedalX,
        pedalY,
        pedalW,
        pedalH,
        0,
        0,
        pedalW,
        pedalH);

    g.restoreState();

    //==========================================================
    // PEDAL OUTER BORDER
    //==========================================================

    g.setColour(
        juce::Colour(0xff454545));

    g.drawRoundedRectangle(
        pedal,
        7.0f,
        2.0f);

    //==========================================================
    // INNER BORDER
    //==========================================================

    g.setColour(
        juce::Colours::black.withAlpha(0.65f));

    g.drawRoundedRectangle(
        pedal.reduced(5.0f),
        5.0f,
        1.0f);

    //==========================================================
    // TOP 9V / POLARITY MARKING
    //==========================================================

    g.setColour(
        juce::Colours::white.withAlpha(0.55f));

    g.setFont(
        juce::Font(
            "Arial",
            9.0f,
            juce::Font::plain));

    g.drawText(
        "9V DC",
        95,
        43,
        65,
        14,
        juce::Justification::centred);

    g.drawText(
        "+     -",
        380,
        43,
        60,
        14,
        juce::Justification::centred);

    //==========================================================
    // GATE GLOW
    //==========================================================

    const float glow =
        juce::jlimit(
            0.0f,
            1.0f,
            gateGlowLevel);

    if (glow > 0.01f)
    {
        for (int i = 5; i >= 1; --i)
        {
            const float r =
                29.0f + i * 6.0f;

            g.setColour(
                juce::Colour(0xff238cff)
                    .withAlpha(
                        0.012f * glow *
                        static_cast<float>(6 - i)));

            g.fillEllipse(
                250.0f - r,
                334.0f - r,
                r * 2.0f,
                r * 2.0f);
        }
    }

    //==========================================================
    // TITLE
    //==========================================================

    g.setColour(
        juce::Colours::white);

    g.setFont(
        juce::Font(
            "Arial",
            22.0f,
            juce::Font::bold));

    g.drawText(
        "RG PRECISION DRIVE",
        90,
        438,
        400,
        28,
        juce::Justification::centred);

    //==========================================================
    // INPUT / OUTPUT MARKING
    //==========================================================

    g.setFont(
        juce::Font(
            "Arial",
            8.5f,
            juce::Font::bold));

    g.setColour(
        juce::Colours::white.withAlpha(0.48f));

    g.drawText(
        "OUT",
        85,
        438,
        45,
        18,
        juce::Justification::centred);

    g.drawText(
        "IN",
        410,
        438,
        45,
        18,
        juce::Justification::centred);

    //==========================================================
    // BLUE LED GLOW
    //==========================================================

    const bool bypassed =
        bypassButton.getToggleState();

    const float ledAlpha =
        bypassed ? 1.0f : 0.12f;

    for (int i = 5; i >= 1; --i)
    {
        const float radius =
            7.0f + static_cast<float>(i) * 4.0f;

        g.setColour(
            juce::Colour(0xff168cff)
                .withAlpha(
                    0.025f *
                    static_cast<float>(6 - i) *
                    ledAlpha));

        g.fillEllipse(
            250.0f - radius,
            487.0f - radius,
            radius * 2.0f,
            radius * 2.0f);
    }

    //==========================================================
    // LED
    //==========================================================

    g.setColour(
        juce::Colour(0xff168cff)
            .withAlpha(ledAlpha));

    g.fillEllipse(
        294.0f,
        487.0f,
        12.0f,
        12.0f);

    g.setColour(
        juce::Colours::white.withAlpha(0.65f));

    g.fillEllipse(
        297.0f,
        489.0f,
        3.0f,
        3.0f);

    //==========================================================
    // METAL FOOTSWITCH
    //==========================================================

    const float switchCX =
        250.0f;

    const float switchCY =
        footswitchPressed ? 568.0f : 562.0f;

    const float switchRadius =
        37.0f;

    // Shadow
    g.setColour(
        juce::Colours::black.withAlpha(0.9f));

    g.fillEllipse(
        switchCX - switchRadius,
        switchCY - switchRadius + 7.0f,
        switchRadius * 2.0f,
        switchRadius * 2.0f);

    // Outer metal
    g.setColour(
        juce::Colour(0xff707070));

    g.fillEllipse(
        switchCX - switchRadius,
        switchCY - switchRadius,
        switchRadius * 2.0f,
        switchRadius * 2.0f);

    // Inner metal
    const float innerRadius =
        31.0f;

    g.setColour(
        juce::Colour(0xffb7b7b7));

    g.fillEllipse(
        switchCX - innerRadius,
        switchCY - innerRadius,
        innerRadius * 2.0f,
        innerRadius * 2.0f);

    // Dark lower face
    g.setColour(
        juce::Colour(0xff696969));

    g.fillEllipse(
        switchCX - 27.0f,
        switchCY - 25.0f,
        54.0f,
        50.0f);

    // Top reflection
    g.setColour(
        juce::Colours::white.withAlpha(0.22f));

    g.fillEllipse(
        switchCX - 20.0f,
        switchCY - 20.0f,
        40.0f,
        18.0f);

    // Center
    g.setColour(
        juce::Colour(0xff383838));

    g.fillEllipse(
        switchCX - 13.0f,
        switchCY - 13.0f,
        26.0f,
        26.0f);

    // Center highlight
    g.setColour(
        juce::Colours::white.withAlpha(0.18f));

    g.fillEllipse(
        switchCX - 8.0f,
        switchCY - 9.0f,
        16.0f,
        9.0f);

    //==========================================================
    // BRAND
    //==========================================================

    g.setColour(
        juce::Colours::white.withAlpha(0.55f));

    g.setFont(
        juce::Font(
            "Arial",
            8.5f,
            juce::Font::bold));

    g.drawText(
        "RG ELECTRONICS",
        160,
        610,
        180,
        18,
        juce::Justification::centred);
}

//==============================================================
// MOUSE DOWN
//==============================================================

void RG_Precision_DriveAudioProcessorEditor::
mouseDown(
    const juce::MouseEvent& event)
{
    if (bypassButton.getBounds().contains(
            event.getPosition()))
    {
        footswitchPressed = true;
        repaint();
    }
}

//==============================================================
// MOUSE UP
//==============================================================

void RG_Precision_DriveAudioProcessorEditor::
mouseUp(
    const juce::MouseEvent& event)
{
    juce::ignoreUnused(event);

    footswitchPressed =
        bypassButton.getToggleState();

    repaint();
}

//==============================================================
// UPDATE LED
//==============================================================

void RG_Precision_DriveAudioProcessorEditor::
updateLED()
{
    const float level =
        audioProcessor.getOutputLevel();

    gateGlowLevel =
        juce::jlimit(
            0.0f,
            1.0f,
            level * 4.0f);

    repaint();
}

//==============================================================
// TIMER
//==============================================================

void RG_Precision_DriveAudioProcessorEditor::
timerCallback()
{
    updateLED();
}
