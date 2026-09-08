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
    // EDITOR
    //==========================================================

    constexpr int editorW = 600;
    constexpr int editorH = 660;

    //==========================================================
    // PEDAL BODY
    //==========================================================

    constexpr float pedalX = 125.0f;
    constexpr float pedalY = 35.0f;
    constexpr float pedalW = 350.0f;
    constexpr float pedalH = 590.0f;

    //==========================================================
    // TEXTURE CACHE
    //==========================================================

    juce::Image orangePeelTexture;

    juce::Image createOrangePeelTexture()
    {
        juce::Image img(
            juce::Image::RGB,
            700,
            1000,
            true);

        juce::Graphics tg(img);

        tg.fillAll(
            juce::Colour(0xff202020));

        juce::Random r(0x7A31C9E5);

        //======================================================
        // LARGE PITS
        //======================================================

        for (int i = 0; i < 9000; ++i)
        {
            const float x =
                r.nextFloat() * 700.0f;

            const float y =
                r.nextFloat() * 1000.0f;

            const float s =
                0.25f + r.nextFloat() * 1.8f;

            tg.setColour(
                juce::Colour(
                    juce::uint8(20 + r.nextInt(25)),
                    juce::uint8(20 + r.nextInt(25)),
                    juce::uint8(20 + r.nextInt(25)),
                    juce::uint8(55 + r.nextInt(80))));

            tg.fillEllipse(
                x,
                y,
                s,
                s);
        }

        //======================================================
        // SMALL BUMPS
        //======================================================

        for (int i = 0; i < 7500; ++i)
        {
            const float x =
                r.nextFloat() * 700.0f;

            const float y =
                r.nextFloat() * 1000.0f;

            const float s =
                0.15f + r.nextFloat() * 1.1f;

            tg.setColour(
                juce::Colour(
                    juce::uint8(55 + r.nextInt(25)),
                    juce::uint8(55 + r.nextInt(25)),
                    juce::uint8(55 + r.nextInt(25)),
                    juce::uint8(35 + r.nextInt(55))));

            tg.fillEllipse(
                x,
                y,
                s,
                s);
        }

        //======================================================
        // MICRO RELIEF
        //======================================================

        for (int i = 0; i < 4500; ++i)
        {
            const float x =
                r.nextFloat() * 700.0f;

            const float y =
                r.nextFloat() * 1000.0f;

            tg.setColour(
                juce::Colour(
                    juce::uint8(70 + r.nextInt(20)),
                    juce::uint8(70 + r.nextInt(20)),
                    juce::uint8(70 + r.nextInt(20)),
                    juce::uint8(20 + r.nextInt(35))));

            tg.fillRect(
                juce::Rectangle<float>(
                    x,
                    y,
                    0.4f,
                    0.4f));
        }

        return img;
    }
}

//==============================================================
// PEDAL KNOB
//==============================================================

PedalKnob::PedalKnob()
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
// KNOB PAINT
//==============================================================

void PedalKnob::paint(
    juce::Graphics& g)
{
    const auto bounds =
        getLocalBounds().toFloat();

    const float w =
        bounds.getWidth();

    const float h =
        bounds.getHeight();

    const float size =
        juce::jmin(w, h);

    const float cx =
        bounds.getCentreX();

    const float cy =
        bounds.getCentreY();

    const float radius =
        size * 0.43f;

    const bool isGate =
        getComponentID() == "GATE";

    //==========================================================
    // SHADOW
    //==========================================================

    g.setColour(
        juce::Colours::black.withAlpha(0.75f));

    g.fillEllipse(
        cx - radius + 2.5f,
        cy - radius + 3.5f,
        radius * 2.0f,
        radius * 2.0f);

    //==========================================================
    // GATE CRYSTAL KNOB
    //==========================================================

    if (isGate)
    {
        juce::ColourGradient crystal(
            juce::Colour(0xffd9e4e8),
            cx - radius,
            cy - radius,

            juce::Colour(0xff526068),
            cx + radius,
            cy + radius,

            true);

        g.setGradientFill(
            crystal);

        g.fillEllipse(
            cx - radius,
            cy - radius,
            radius * 2.0f,
            radius * 2.0f);

        g.setColour(
            juce::Colours::black.withAlpha(0.55f));

        g.drawEllipse(
            cx - radius + 2.0f,
            cy - radius + 2.0f,
            radius * 2.0f - 4.0f,
            radius * 2.0f - 4.0f,
            1.2f);

        // Crystal highlight
        g.setColour(
            juce::Colours::white.withAlpha(0.55f));

        g.fillEllipse(
            cx - radius * 0.48f,
            cy - radius * 0.48f,
            radius * 0.42f,
            radius * 0.22f);

        // Pointer
        const float angle =
            juce::jmap(
                (float)getValue(),
                0.0f,
                1.0f,
                knobStartAngle,
                knobEndAngle);

        const float pointerR =
            radius * 0.72f;

        juce::Line<float> pointer(
            cx,
            cy,
            cx + std::cos(angle) * pointerR,
            cy + std::sin(angle) * pointerR);

        g.setColour(
            juce::Colours::white.withAlpha(0.9f));

        g.drawLine(
            pointer,
            1.5f);

        return;
    }

    //==========================================================
    // STANDARD BLACK KNOB
    //==========================================================

    juce::ColourGradient knobGradient(
        juce::Colour(0xff454545),
        cx - radius,
        cy - radius,

        juce::Colour(0xff050505),
        cx + radius,
        cy + radius,

        true);

    g.setGradientFill(
        knobGradient);

    g.fillEllipse(
        cx - radius,
        cy - radius,
        radius * 2.0f,
        radius * 2.0f);

    //==========================================================
    // OUTER EDGE
    //==========================================================

    g.setColour(
        juce::Colour(0xff111111));

    g.drawEllipse(
        cx - radius,
        cy - radius,
        radius * 2.0f,
        radius * 2.0f,
        2.0f);

    //==========================================================
    // INNER EDGE
    //==========================================================

    g.setColour(
        juce::Colours::white.withAlpha(0.08f));

    g.drawEllipse(
        cx - radius + 3.0f,
        cy - radius + 3.0f,
        radius * 2.0f - 6.0f,
        radius * 2.0f - 6.0f,
        1.0f);

    //==========================================================
    // TOP HIGHLIGHT
    //==========================================================

    g.setColour(
        juce::Colours::white.withAlpha(0.10f));

    g.fillEllipse(
        cx - radius * 0.48f,
        cy - radius * 0.55f,
        radius * 0.65f,
        radius * 0.25f);

    //==========================================================
    // POINTER
    //==========================================================

    const float angle =
        juce::jmap(
            (float)getValue(),
            0.0f,
            1.0f,
            knobStartAngle,
            knobEndAngle);

    const float pointerR =
        radius * 0.72f;

    juce::Line<float> pointer(
        cx,
        cy,
        cx + std::cos(angle) * pointerR,
        cy + std::sin(angle) * pointerR);

    g.setColour(
        juce::Colours::white.withAlpha(0.95f));

    g.drawLine(
        pointer,
        2.0f);
}

//==============================================================
// CONSTRUCTOR
//==============================================================

RG_PrecisionDriveAudioProcessorEditor::
RG_PrecisionDriveAudioProcessorEditor(
    RG_PrecisionDriveAudioProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p)
{
    setSize(
        editorW,
        editorH);

    setResizable(
        false,
        false);

    //==========================================================
    // KNOBS
    //==========================================================

    addAndMakeVisible(volumeKnob);
    addAndMakeVisible(brightKnob);
    addAndMakeVisible(attackKnob);
    addAndMakeVisible(gateKnob);
    addAndMakeVisible(driveKnob);

    gateKnob.setComponentID(
        "GATE");

    //==========================================================
    // BYPASS
    //==========================================================

    addAndMakeVisible(
        bypassButton);

    bypassButton.setButtonText(
        "");

    bypassButton.setClickingTogglesState(
        true);

    bypassButton.setAlpha(
        0.001f);

    //==========================================================
    // ATTACHMENTS
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

    gateAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>(
                audioProcessor.apvts,
                "GATE",
                gateKnob);

    driveAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::SliderAttachment>(
                audioProcessor.apvts,
                "DRIVE",
                driveKnob);

    bypassAttachment =
        std::make_unique<
            juce::AudioProcessorValueTreeState::ButtonAttachment>(
                audioProcessor.apvts,
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

RG_PrecisionDriveAudioProcessorEditor::
~RG_PrecisionDriveAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================
// RESIZED
//==============================================================

void RG_PrecisionDriveAudioProcessorEditor::resized()
{
    //==========================================================
    // TOP CONTROLS
    //==========================================================

    volumeKnob.setBounds(
        160,
        105,
        95,
        95);

    brightKnob.setBounds(
        345,
        105,
        95,
        95);

    //==========================================================
    // MIDDLE CONTROLS
    //==========================================================

    attackKnob.setBounds(
        150,
        265,
        92,
        92);

    gateKnob.setBounds(
        296,
        292,
        52,
        52);

    driveKnob.setBounds(
        358,
        265,
        92,
        92);

    //==========================================================
    // FOOTSWITCH HIT AREA
    //==========================================================

    bypassButton.setBounds(
        235,
        535,
        130,
        90);
}

//==============================================================
// PAINT
//==============================================================

void RG_PrecisionDriveAudioProcessorEditor::paint(
    juce::Graphics& g)
{
    //==========================================================
    // BLACK BACKGROUND
    //==========================================================

    g.fillAll(
        juce::Colours::black);

    //==========================================================
    // PEDAL RECTANGLE
    //==========================================================

    auto pedal =
        juce::Rectangle<float>(
            pedalX,
            pedalY,
            pedalW,
            pedalH);

    //==========================================================
    // BASE
    //==========================================================

    g.setColour(
        juce::Colour(0xff242424));

    g.fillRoundedRectangle(
        pedal,
        5.0f);

    //==========================================================
    // TEXTURE
    //==========================================================

    if (orangePeelTexture.isNull())
        orangePeelTexture =
            createOrangePeelTexture();

    g.saveState();

    g.reduceClipRegion(
        pedal.toNearestInt());

    g.setOpacity(
        0.62f);

    // FIXED JUCE drawImage CALL
    g.drawImageWithin(
        orangePeelTexture,
        pedal.getX(),
        pedal.getY(),
        pedal.getWidth(),
        pedal.getHeight(),
        juce::RectanglePlacement::stretchToFit,
        false);

    g.restoreState();

    //==========================================================
    // OUTER BORDER
    //==========================================================

    g.setColour(
        juce::Colour(0xff080808));

    g.drawRoundedRectangle(
        pedal,
        5.0f,
        6.0f);

    //==========================================================
    // INNER BORDER
    //==========================================================

    auto inner =
        pedal.reduced(5.0f);

    g.setColour(
        juce::Colour(0xff4b4b4b)
            .withAlpha(0.75f));

    g.drawRoundedRectangle(
        inner,
        3.5f,
        1.0f);

    //==========================================================
    // TOP METAL HIGHLIGHT
    //==========================================================

    g.setColour(
        juce::Colours::white.withAlpha(0.055f));

    g.drawLine(
        pedal.getX() + 12.0f,
        pedal.getY() + 7.0f,
        pedal.getRight() - 12.0f,
        pedal.getY() + 7.0f,
        1.0f);

    //==========================================================
    // 9V
    //==========================================================

    g.setColour(
        juce::Colours::white.withAlpha(0.68f));

    g.setFont(
        juce::Font(
            9.0f,
            juce::Font::plain));

    g.drawText(
        "9V",
        145,
        51,
        35,
        12,
        juce::Justification::centred);

    g.drawText(
        "CENTER NEGATIVE",
        205,
        51,
        130,
        12,
        juce::Justification::centred);

    //==========================================================
    // LABELS
    //==========================================================

    g.setColour(
        juce::Colours::white.withAlpha(0.90f));

    g.setFont(
        juce::Font(
            11.0f,
            juce::Font::bold));

    g.drawText(
        "VOLUME",
        155,
        87,
        105,
        16,
        juce::Justification::centred);

    g.drawText(
        "BRIGHT",
        340,
        87,
        105,
        16,
        juce::Justification::centred);

    g.drawText(
        "ATTACK",
        145,
        245,
        105,
        16,
        juce::Justification::centred);

    g.drawText(
        "DRIVE",
        350,
        245,
        105,
        16,
        juce::Justification::centred);

    //==========================================================
    // GATE
    //==========================================================

    g.setColour(
        juce::Colours::white.withAlpha(0.82f));

    g.setFont(
        juce::Font(
            8.0f,
            juce::Font::bold));

    g.drawText(
        "GATE",
        285,
        351,
        75,
        16,
        juce::Justification::centred);

    //==========================================================
    // GATE HALO
    //==========================================================

    g.setColour(
        juce::Colours::white.withAlpha(0.05f));

    g.drawEllipse(
        289.0f,
        285.0f,
        66.0f,
        66.0f,
        1.0f);

    //==========================================================
    // PRODUCT NAME
    //==========================================================

    g.setColour(
        juce::Colours::white.withAlpha(0.95f));

    g.setFont(
        juce::Font(
            17.0f,
            juce::Font::bold));

    g.drawText(
        "RG PRECISION DRIVE",
        145,
        414,
        310,
        25,
        juce::Justification::centred);

    //==========================================================
    // IN / OUT
    //==========================================================

    g.setFont(
        juce::Font(
            8.0f,
            juce::Font::bold));

    g.setColour(
        juce::Colours::white.withAlpha(0.62f));

    g.drawText(
        "OUT",
        140,
        438,
        40,
        14,
        juce::Justification::centred);

    g.drawText(
        "IN",
        420,
        438,
        40,
        14,
        juce::Justification::centred);

    //==========================================================
    // BLUE LED
    //==========================================================

    const float ledX =
        300.0f;

    const float ledY =
        482.0f;

    const float glow =
        juce::jlimit(
            0.0f,
            1.0f,
            gateGlowLevel);

    if (glow > 0.01f)
    {
        juce::ColourGradient glowGradient(
            juce::Colours::deepskyblue.withAlpha(
                0.38f * glow),

            ledX,
            ledY,

            juce::Colours::transparentBlack,

            ledX + 30.0f,
            ledY + 30.0f,

            true);

        g.setGradientFill(
            glowGradient);

        g.fillEllipse(
            ledX - 25.0f,
            ledY - 25.0f,
            50.0f,
            50.0f);
    }

    // LED black surround
    g.setColour(
        juce::Colours::black);

    g.fillEllipse(
        ledX - 9.0f,
        ledY - 9.0f,
        18.0f,
        18.0f);

    // LED
    g.setColour(
        juce::Colours::deepskyblue.withAlpha(
            0.85f *
            juce::jmax(0.35f, glow)));

    g.fillEllipse(
        ledX - 6.0f,
        ledY - 6.0f,
        12.0f,
        12.0f);

    // LED highlight
    g.setColour(
        juce::Colours::white.withAlpha(0.55f));

    g.fillEllipse(
        ledX - 3.0f,
        ledY - 4.0f,
        4.0f,
        3.0f);

    //==========================================================
    // FOOTSWITCH
    //==========================================================

    const float switchCX =
        300.0f;

    const float switchCY =
        footswitchPressed
            ? 563.0f
            : 557.0f;

    // Shadow
    g.setColour(
        juce::Colours::black.withAlpha(0.85f));

    g.fillEllipse(
        switchCX - 38.0f,
        570.0f,
        76.0f,
        30.0f);

    // Outer ring
    g.setColour(
        juce::Colour(0xff111111));

    g.fillEllipse(
        switchCX - 32.0f,
        switchCY - 32.0f,
        64.0f,
        64.0f);

    // Metal
    juce::ColourGradient metal(
        juce::Colour(0xffeeeeee),
        switchCX - 20.0f,
        switchCY - 25.0f,

        juce::Colour(0xff505050),
        switchCX + 25.0f,
        switchCY + 28.0f,

        true);

    g.setGradientFill(
        metal);

    g.fillEllipse(
        switchCX - 25.0f,
        switchCY - 25.0f,
        50.0f,
        50.0f);

    // Inner ring
    g.setColour(
        juce::Colour(0xff262626));

    g.drawEllipse(
        switchCX - 20.0f,
        switchCY - 20.0f,
        40.0f,
        40.0f,
        2.0f);

    // Highlight
    g.setColour(
        juce::Colours::white.withAlpha(0.32f));

    g.fillEllipse(
        switchCX - 13.0f,
        switchCY - 14.0f,
        18.0f,
        9.0f);

    //==========================================================
    // RG ELECTRONICS
    //==========================================================

    g.setColour(
        juce::Colours::white.withAlpha(0.72f));

    g.setFont(
        juce::Font(
            9.0f,
            juce::Font::bold));

    g.drawText(
        "RG ELECTRONICS",
        155,
        601,
        290,
        15,
        juce::Justification::centred);
}

//==============================================================
// MOUSE DOWN
//==============================================================

void RG_PrecisionDriveAudioProcessorEditor::mouseDown(
    const juce::MouseEvent&)
{
    footswitchPressed = true;
    repaint();
}

//==============================================================
// MOUSE UP
//==============================================================

void RG_PrecisionDriveAudioProcessorEditor::mouseUp(
    const juce::MouseEvent&)
{
    footswitchPressed = false;
    repaint();
}

//==============================================================
// UPDATE LED
//==============================================================

void RG_PrecisionDriveAudioProcessorEditor::updateLED()
{
    const float level =
        audioProcessor.getOutputLevel();

    gateGlowLevel =
        juce::jlimit(
            0.0f,
            1.0f,
            level);

    repaint();
}

//==============================================================
// TIMER
//==============================================================

void RG_PrecisionDriveAudioProcessorEditor::timerCallback()
{
    updateLED();
}
