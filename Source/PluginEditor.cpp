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

    //==========================================================
    // CACHED THICK + VISIBLE ORANGE-PEEL TEXTURE
    //==========================================================

    juce::Image orangePeelTexture;

    void createOrangePeelTexture(
        int width,
        int height)
    {
        orangePeelTexture = juce::Image(
            juce::Image::ARGB,
            width,
            height,
            true);

        juce::Graphics tg(
            orangePeelTexture);

        juce::Random r(
            0x7A31C9E5);

        //======================================================
        // DEEP IRREGULAR POWDER-COAT VALLEYS
        //======================================================

        for (int i = 0; i < 9000; ++i)
        {
            const float x =
                r.nextFloat()
                * static_cast<float>(width);

            const float y =
                r.nextFloat()
                * static_cast<float>(height);

            const float rx =
                0.70f
                + r.nextFloat() * 1.35f;

            const float ry =
                0.55f
                + r.nextFloat() * 1.15f;

            const float rotation =
                r.nextFloat()
                * juce::MathConstants<float>::twoPi;

            juce::Path pit;

            constexpr int points = 6;

            for (int p = 0;
                 p < points;
                 ++p)
            {
                const float angle =
                    (static_cast<float>(p)
                     / static_cast<float>(points))
                    * juce::MathConstants<float>::twoPi;

                const float irregular =
                    0.65f
                    + r.nextFloat() * 0.55f;

                const float px =
                    std::cos(angle)
                    * rx
                    * irregular;

                const float py =
                    std::sin(angle)
                    * ry
                    * irregular;

                const float c =
                    std::cos(rotation);

                const float s =
                    std::sin(rotation);

                const float finalX =
                    x
                    + px * c
                    - py * s;

                const float finalY =
                    y
                    + px * s
                    + py * c;

                if (p == 0)
                {
                    pit.startNewSubPath(
                        finalX,
                        finalY);
                }
                else
                {
                    pit.lineTo(
                        finalX,
                        finalY);
                }
            }

            pit.closeSubPath();

            tg.setColour(
                juce::Colour(
                    0xff000000)
                    .withAlpha(0.20f));

            tg.fillPath(pit);
        }

        //======================================================
        // RAISED ORANGE-PEEL BUMPS
        //======================================================

        for (int i = 0; i < 7500; ++i)
        {
            const float x =
                r.nextFloat()
                * static_cast<float>(width);

            const float y =
                r.nextFloat()
                * static_cast<float>(height);

            const float rx =
                0.65f
                + r.nextFloat() * 1.45f;

            const float ry =
                0.50f
                + r.nextFloat() * 1.20f;

            const float rotation =
                r.nextFloat()
                * juce::MathConstants<float>::twoPi;

            juce::Path bump;

            constexpr int points = 7;

            for (int p = 0;
                 p < points;
                 ++p)
            {
                const float angle =
                    (static_cast<float>(p)
                     / static_cast<float>(points))
                    * juce::MathConstants<float>::twoPi;

                const float irregular =
                    0.70f
                    + r.nextFloat() * 0.50f;

                const float px =
                    std::cos(angle)
                    * rx
                    * irregular;

                const float py =
                    std::sin(angle)
                    * ry
                    * irregular;

                const float c =
                    std::cos(rotation);

                const float s =
                    std::sin(rotation);

                const float finalX =
                    x
                    + px * c
                    - py * s;

                const float finalY =
                    y
                    + px * s
                    + py * c;

                if (p == 0)
                {
                    bump.startNewSubPath(
                        finalX,
                        finalY);
                }
                else
                {
                    bump.lineTo(
                        finalX,
                        finalY);
                }
            }

            bump.closeSubPath();

            tg.setColour(
                juce::Colour(
                    0xffB8BDC0)
                    .withAlpha(0.105f));

            tg.fillPath(bump);
        }

        //======================================================
        // SMALL MICRO RELIEF
        //======================================================

        for (int i = 0; i < 4500; ++i)
        {
            const float x =
                r.nextFloat()
                * static_cast<float>(width);

            const float y =
                r.nextFloat()
                * static_cast<float>(height);

            const float rx =
                0.25f
                + r.nextFloat() * 0.70f;

            const float ry =
                0.20f
                + r.nextFloat() * 0.60f;

            if (r.nextBool())
            {
                tg.setColour(
                    juce::Colour(
                        0xffD0D4D6)
                        .withAlpha(0.075f));
            }
            else
            {
                tg.setColour(
                    juce::Colour(
                        0xff000000)
                        .withAlpha(0.075f));
            }

            tg.fillEllipse(
                x - rx,
                y - ry,
                rx * 2.0f,
                ry * 2.0f);
        }
    }
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

void RG_Precision_DriveAudioProcessorEditor::PedalKnob::paint(
    juce::Graphics& g)
{
    auto bounds =
        getLocalBounds().toFloat();

    const float diameter =
        juce::jmin(
            bounds.getWidth(),
            bounds.getHeight())
        - 10.0f;

    const float x =
        bounds.getCentreX()
        - diameter * 0.5f;

    const float y =
        bounds.getCentreY()
        - diameter * 0.5f;

    juce::Rectangle<float> knobBounds(
        x,
        y,
        diameter,
        diameter);

    const float centreX =
        knobBounds.getCentreX();

    const float centreY =
        knobBounds.getCentreY();

    //==========================================================
    // SHADOW
    //==========================================================

    g.setColour(
        juce::Colour(0x80000000));

    g.fillEllipse(
        knobBounds.expanded(4.0f));

    //==========================================================
    // GLOSS BLACK PLASTIC
    //==========================================================

    juce::ColourGradient glossGradient(
        juce::Colour(0xff505050),
        knobBounds.getX(),
        knobBounds.getY(),
        juce::Colour(0xff050505),
        knobBounds.getRight(),
        knobBounds.getBottom(),
        true);

    g.setGradientFill(
        glossGradient);

    g.fillEllipse(
        knobBounds);

    //==========================================================
    // DARK EDGE
    //==========================================================

    g.setColour(
        juce::Colour(0xff020202));

    g.drawEllipse(
        knobBounds,
        2.0f);

    //==========================================================
    // INNER BLACK PLASTIC
    //==========================================================

    auto innerBounds =
        knobBounds.reduced(5.0f);

    juce::ColourGradient bodyGradient(
        juce::Colour(0xff252525),
        innerBounds.getX(),
        innerBounds.getY(),
        juce::Colour(0xff080808),
        innerBounds.getRight(),
        innerBounds.getBottom(),
        true);

    g.setGradientFill(
        bodyGradient);

    g.fillEllipse(
        innerBounds);

    //==========================================================
    // NO WHITE HIGHLIGHT
    // NO CENTER CAP
    //==========================================================

    //==========================================================
    // POINTER
    //==========================================================

    auto range =
        getNormalisableRange();

    const float normalized =
        (float) range.convertTo0to1(
            getValue());

    const float angle =
        knobStartAngle
        + normalized
        * (knobEndAngle
           - knobStartAngle);

    const float pointerStart =
        diameter * 0.08f;

    const float pointerLength =
        diameter * 0.38f;

    const float startX =
        centreX
        + std::cos(angle)
        * pointerStart;

    const float startY =
        centreY
        + std::sin(angle)
        * pointerStart;

    const float endX =
        centreX
        + std::cos(angle)
        * pointerLength;

    const float endY =
        centreY
        + std::sin(angle)
        * pointerLength;

    //==========================================================
    // POINTER SHADOW
    //==========================================================

    g.setColour(
        juce::Colour(0x90000000));

    g.drawLine(
        startX + 1.0f,
        startY + 1.0f,
        endX + 1.0f,
        endY + 1.0f,
        5.0f);

    //==========================================================
    // WHITE POINTER
    //==========================================================

    g.setColour(
        juce::Colour(0xffffffff));

    g.drawLine(
        startX,
        startY,
        endX,
        endY,
        3.5f);
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
    // VOLUME
    //==========================================================

    setupKnob(
        volumeKnob,
        juce::Slider::RotaryHorizontalVerticalDrag);

    setupLabel(
        volumeLabel,
        "VOLUME");

    //==========================================================
    // BRIGHT
    //==========================================================

    setupKnob(
        brightKnob,
        juce::Slider::RotaryHorizontalVerticalDrag);

    setupLabel(
        brightLabel,
        "BRIGHT");

    //==========================================================
    // ATTACK
    //==========================================================

    setupKnob(
        attackKnob,
        juce::Slider::RotaryHorizontalVerticalDrag,
        true);

    setupLabel(
        attackLabel,
        "ATTACK");

    //==========================================================
    // DRIVE
    //==========================================================

    setupKnob(
        driveKnob,
        juce::Slider::RotaryHorizontalVerticalDrag);

    setupLabel(
        driveLabel,
        "DRIVE");

    //==========================================================
    // GATE
    //==========================================================

    setupKnob(
        gateKnob,
        juce::Slider::RotaryHorizontalVerticalDrag);

    setupLabel(
        gateLabel,
        "GATE");

    //==========================================================
    // FOOTSWITCH
    //==========================================================

    bypassButton.setButtonText("");

    bypassButton.setClickingTogglesState(true);

    bypassButton.setColour(
        juce::ToggleButton::textColourId,
        juce::Colours::transparentBlack);

    bypassButton.setColour(
        juce::ToggleButton::tickColourId,
        juce::Colours::transparentBlack);

    bypassButton.setAlpha(0.001f);

    addAndMakeVisible(
        bypassButton);

    led.setVisible(false);

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

    if (selector)
    {
        knob.setRange(
            1.0,
            6.0,
            1.0);

        knob.setNumDecimalPlacesToDisplay(
            0);
    }
    else
    {
        knob.setRange(
            0.0,
            1.0,
            0.001);
    }

    addAndMakeVisible(
        knob);
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

    label.setJustificationType(
        juce::Justification::centred);

    label.setFont(
        juce::Font(
            14.0f,
            juce::Font::bold));

    label.setColour(
        juce::Label::textColourId,
        juce::Colour(0xff2494ff));

    label.setColour(
        juce::Label::backgroundColourId,
        juce::Colours::transparentBlack);

    addAndMakeVisible(
        label);
}

//==============================================================
// RESIZED
//==============================================================

void RG_Precision_DriveAudioProcessorEditor::resized()
{
    //==========================================================
    // TOP ROW
    // 120 x 120 EACH
    //==========================================================

    volumeLabel.setBounds(
        60,
        35,
        145,
        22);

    volumeKnob.setBounds(
        72,
        53,
        120,
        120);

    brightLabel.setBounds(
        295,
        35,
        145,
        22);

    brightKnob.setBounds(
        307,
        53,
        120,
        120);

    //==========================================================
    // MIDDLE ROW
    // 120 x 120 EACH
    //==========================================================

    attackLabel.setBounds(
        30,
        205,
        135,
        22);

    attackKnob.setBounds(
        37,
        225,
        120,
        120);

    driveLabel.setBounds(
        335,
        205,
        135,
        22);

    driveKnob.setBounds(
        342,
        225,
        120,
        120);

    //==========================================================
    // GATE
    // EXACTLY AS BEFORE
    //==========================================================

    gateKnob.setBounds(
        215,
        255,
        70,
        70);

    gateLabel.setBounds(
        200,
        330,
        100,
        24);

    //==========================================================
    // FOOTSWITCH HIT AREA
    //==========================================================

    bypassButton.setBounds(
        175,
        520,
        150,
        105);
}

//==============================================================
// LED + AUDIO REACTIVE GATE GLOW
//==============================================================

void RG_Precision_DriveAudioProcessorEditor::updateLED()
{
    const float target =
        juce::jlimit(
            0.0f,
            1.0f,
            audioProcessor.getOutputLevel());

    if (target > gateGlowLevel)
    {
        gateGlowLevel +=
            (target - gateGlowLevel)
            * 0.40f;
    }
    else
    {
        gateGlowLevel +=
            (target - gateGlowLevel)
            * 0.12f;
    }

    if (gateGlowLevel < 0.0005f)
        gateGlowLevel = 0.0f;

    repaint();
}

//==============================================================
// TIMER
//==============================================================

void RG_Precision_DriveAudioProcessorEditor::timerCallback()
{
    updateLED();
}

//==============================================================
// MOUSE DOWN
//==============================================================

void RG_Precision_DriveAudioProcessorEditor::mouseDown(
    const juce::MouseEvent&)
{
    footswitchPressed = true;

    repaint();
}

//==============================================================
// MOUSE UP
//==============================================================

void RG_Precision_DriveAudioProcessorEditor::mouseUp(
    const juce::MouseEvent&)
{
    footswitchPressed = false;

    repaint();
}

//==============================================================
// PAINT
//==============================================================

void RG_Precision_DriveAudioProcessorEditor::paint(
    juce::Graphics& g)
{
    auto area =
        getLocalBounds().toFloat();

    //==========================================================
    // PEDAL BODY - DARK BLACK / GUNMETAL
    //==========================================================

    juce::ColourGradient backgroundGradient(
        juce::Colour(0xff292B2D),
        area.getX(),
        area.getY(),
        juce::Colour(0xff090A0B),
        area.getRight(),
        area.getBottom(),
        false);

    g.setGradientFill(
        backgroundGradient);

    g.fillRoundedRectangle(
        area.reduced(4.0f),
        18.0f);

    //==========================================================
    // CACHED THICK ORANGE-PEEL
    //==========================================================

    if (orangePeelTexture.isNull()
        || orangePeelTexture.getWidth() != getWidth()
        || orangePeelTexture.getHeight() != getHeight())
    {
        createOrangePeelTexture(
            getWidth(),
            getHeight());
    }

    g.saveState();

    juce::Path textureClip;

    textureClip.addRoundedRectangle(
        area.reduced(4.0f),
        18.0f);

    g.reduceClipRegion(
        textureClip);

    g.drawImageAt(
        orangePeelTexture,
        0,
        0);

    g.restoreState();

    //==========================================================
    // OUTER ENCLOSURE EDGE - GUNMETAL
    //==========================================================

    g.setColour(
        juce::Colour(0xff4A4F54));

    g.drawRoundedRectangle(
        area.reduced(4.0f),
        18.0f,
        3.0f);

    //==========================================================
    // MAIN ENCLOSURE BORDER - WHITE
    //==========================================================

    g.setColour(
        juce::Colour(0xffffffff));

    g.drawRoundedRectangle(
        area.reduced(8.0f),
        16.0f,
        1.5f);

    //==========================================================
    // INNER BORDER - SOFT STEEL GRAY
    //==========================================================

    g.setColour(
        juce::Colour(0xff6f767d));

    g.drawRoundedRectangle(
        area.reduced(12.0f),
        13.0f,
        1.0f);

    //==========================================================
    // 9V POWER LABEL + CENTER-NEGATIVE SYMBOL
    // MOVED DOWN SLIGHTLY
    //==========================================================

    g.setColour(
        juce::Colour(0xff70757c));

    g.setFont(
        juce::Font(
            13.0f,
            juce::Font::bold));

    g.drawFittedText(
        "9V",
        juce::Rectangle<int>(
            220,
            17,
            60,
            18),
        juce::Justification::centred,
        1);

    //==========================================================
    // CENTER-NEGATIVE POLARITY SYMBOL
    //==========================================================

    const float polarityY = 40.0f;
    const float centerX = 250.0f;

    g.setColour(
        juce::Colour(0xff70757c));

    g.setFont(
        juce::Font(
            12.0f,
            juce::Font::bold));

    // LEFT MINUS
    g.drawFittedText(
        "-",
        juce::Rectangle<int>(
            208,
            31,
            18,
            18),
        juce::Justification::centred,
        1);

    // RIGHT PLUS
    g.drawFittedText(
        "+",
        juce::Rectangle<int>(
            274,
            31,
            18,
            18),
        juce::Justification::centred,
        1);

    // LEFT CONNECTION
    g.drawLine(
        228.0f,
        polarityY,
        243.0f,
        polarityY,
        1.5f);

    // RIGHT CONNECTION
    g.drawLine(
        257.0f,
        polarityY,
        272.0f,
        polarityY,
        1.5f);

    // CENTER NEGATIVE DOT
    g.fillEllipse(
        centerX - 5.0f,
        polarityY - 5.0f,
        10.0f,
        10.0f);

    //==========================================================
    // GATE AUDIO-REACTIVE BLUE HALO
    //==========================================================

    if (gateGlowLevel > 0.001f)
    {
        const float centreX = 250.0f;
        const float centreY = 290.0f;

        const float intensity =
            juce::jlimit(
                0.0f,
                1.0f,
                gateGlowLevel);

        g.setColour(
            juce::Colour(
                static_cast<juce::uint8>(
                    20.0f
                    + 45.0f * intensity),
                static_cast<juce::uint8>(
                    100.0f
                    + 80.0f * intensity),
                255,
                static_cast<juce::uint8>(
                    15.0f
                    + 70.0f * intensity)));

        g.fillEllipse(
            centreX - 43.0f,
            centreY - 43.0f,
            86.0f,
            86.0f);

        g.setColour(
            juce::Colour(
                0x203da9ff));

        g.fillEllipse(
            centreX - 39.0f,
            centreY - 39.0f,
            78.0f,
            78.0f);

        g.setColour(
            juce::Colour(
                static_cast<juce::uint8>(
                    40.0f
                    + 80.0f * intensity),
                static_cast<juce::uint8>(
                    130.0f
                    + 80.0f * intensity),
                255,
                static_cast<juce::uint8>(
                    25.0f
                    + 110.0f * intensity)));

        g.drawEllipse(
            centreX - 38.0f,
            centreY - 38.0f,
            76.0f,
            76.0f,
            2.0f);
    }

    //==========================================================
    // TITLE
    //==========================================================

    g.setColour(
        juce::Colour(0xff2494ff));

    g.setFont(
        juce::Font(
            25.0f,
            juce::Font::bold));

    g.drawFittedText(
        "RG PRECISION DRIVE",
        juce::Rectangle<int>(
            55,
            380,
            390,
            42),
        juce::Justification::centred,
        1);

    //==========================================================
    // OUT / IN
    //==========================================================

    g.setColour(
        juce::Colour(0xff2494ff));

    g.setFont(
        juce::Font(
            13.0f,
            juce::Font::bold));

    g.drawFittedText(
        "OUT",
        juce::Rectangle<int>(
            15,
            419,
            35,
            24),
        juce::Justification::centred,
        1);

    g.drawFittedText(
        "IN",
        juce::Rectangle<int>(
            450,
            419,
            35,
            24),
        juce::Justification::centred,
        1);

    //==========================================================
    // BLUE FOOTSWITCH LED
    //==========================================================

    const bool fxOn =
        !bypassButton.getToggleState();

    const float ledX = 250.0f;
    const float ledY = 492.0f;
    const float ledSize = 25.0f;

    if (fxOn)
    {
        g.setColour(
            juce::Colour(0x403da9ff));

        g.fillEllipse(
            ledX - ledSize * 0.9f,
            ledY - ledSize * 0.9f,
            ledSize * 1.8f,
            ledSize * 1.8f);

        g.setColour(
            juce::Colour(0x803da9ff));

        g.fillEllipse(
            ledX - ledSize * 0.65f,
            ledY - ledSize * 0.65f,
            ledSize * 1.3f,
            ledSize * 1.3f);

        g.setColour(
            juce::Colour(0xff258cff));

        g.fillEllipse(
            ledX - ledSize * 0.5f,
            ledY - ledSize * 0.5f,
            ledSize,
            ledSize);

        g.setColour(
            juce::Colour(0xff9fd8ff));

        g.fillEllipse(
            ledX - 4.0f,
            ledY - 5.0f,
            8.0f,
            7.0f);
    }
    else
    {
        g.setColour(
            juce::Colour(0xff101316));

        g.fillEllipse(
            ledX - ledSize * 0.5f,
            ledY - ledSize * 0.5f,
            ledSize,
            ledSize);

        g.setColour(
            juce::Colour(0xff30353a));

        g.drawEllipse(
            ledX - ledSize * 0.5f,
            ledY - ledSize * 0.5f,
            ledSize,
            ledSize,
            1.0f);
    }

    //==========================================================
    // FOOTSWITCH
    // ONLY THE METAL CENTER MOVES
    //==========================================================

    const bool pressed =
        bypassButton.isMouseButtonDown();

    const float switchX = 250.0f;

    //==========================================================
    // FIXED OUTER ASSEMBLY POSITION
    //==========================================================

    const float switchY = 570.0f;

    const float switchRadius = 48.0f;

    //==========================================================
    // ONLY METAL CENTER MOVEMENT
    //==========================================================

    const float centerY =
        pressed ? 575.0f : 570.0f;

    //==========================================================
    // SHADOW - FIXED
    //==========================================================

    g.setColour(
        juce::Colour(0x90000000));

    g.fillEllipse(
        switchX - switchRadius - 4.0f,
        switchY - switchRadius + 7.0f,
        (switchRadius + 4.0f) * 2.0f,
        (switchRadius + 4.0f) * 2.0f);

    //==========================================================
    // OUTER METAL RING - FIXED
    //==========================================================

    juce::ColourGradient metalGradient(
        juce::Colour(0xffeeeeee),
        switchX - switchRadius,
        switchY - switchRadius,
        juce::Colour(0xff55585c),
        switchX + switchRadius,
        switchY + switchRadius,
        true);

    g.setGradientFill(
        metalGradient);

    g.fillEllipse(
        switchX - switchRadius,
        switchY - switchRadius,
        switchRadius * 2.0f,
        switchRadius * 2.0f);

    //==========================================================
    // DARK INNER RING - FIXED
    //==========================================================

    g.setColour(
        juce::Colour(0xff25282b));

    g.fillEllipse(
        switchX - 39.0f,
        switchY - 39.0f,
        78.0f,
        78.0f);

    //==========================================================
    // METAL CENTER - ONLY THIS MOVES
    //==========================================================

    juce::ColourGradient centerMetal(
        juce::Colour(0xffd9dcdf),
        switchX - 28.0f,
        centerY - 28.0f,
        juce::Colour(0xff666a6e),
        switchX + 28.0f,
        centerY + 28.0f,
        true);

    g.setGradientFill(
        centerMetal);

    g.fillEllipse(
        switchX - 30.0f,
        centerY - 30.0f,
        60.0f,
        60.0f);

    //==========================================================
    // CENTER HIGHLIGHT - MOVES WITH METAL CENTER
    //==========================================================

    g.setColour(
        juce::Colour(0x90ffffff));

    g.fillEllipse(
        switchX - 17.0f,
        centerY - 21.0f,
        34.0f,
        13.0f);

    //==========================================================
    // CENTER EDGE - MOVES WITH METAL CENTER
    //==========================================================

    g.setColour(
        juce::Colour(0x80666a6e));

    g.drawEllipse(
        switchX - 30.0f,
        centerY - 30.0f,
        60.0f,
        60.0f,
        1.5f);

    //==========================================================
    // BRAND
    //==========================================================

    g.setColour(
        juce::Colour(0xff70757c));

    g.setFont(
        juce::Font(
            17.0f,
            juce::Font::bold));

    g.drawFittedText(
        "RG ELECTRONICS",
        juce::Rectangle<int>(
            40,
            650,
            420,
            30),
        juce::Justification::centred,
        1);
}
