#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class RG_Precision_DriveAudioProcessorEditor
    : public juce::AudioProcessorEditor,
      private juce::Timer
{
public:
    RG_Precision_DriveAudioProcessorEditor(
        RG_Precision_DriveAudioProcessor&);

    ~RG_Precision_DriveAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    //==============================================================
    // CLICKABLE FOOTSWITCH
    //==============================================================

    void mouseDown(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;

private:
    void timerCallback() override;

    RG_Precision_DriveAudioProcessor& audioProcessor;

    //==============================================================
    // AUDIO REACTIVE GATE GLOW
    //==============================================================

    float gateGlowLevel = 0.0f;

    //==============================================================
    // CUSTOM PEDAL KNOB
    //==============================================================

    class PedalKnob : public juce::Slider
    {
    public:
        PedalKnob();

        void paint(juce::Graphics&) override;

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PedalKnob)
    };

    //==============================================================
    // CONTROLS
    //==============================================================

    PedalKnob volumeKnob;
    PedalKnob brightKnob;
    PedalKnob attackKnob;
    PedalKnob driveKnob;
    PedalKnob gateKnob;

    //==============================================================
    // LABELS
    //==============================================================

    juce::Label volumeLabel;
    juce::Label brightLabel;
    juce::Label attackLabel;
    juce::Label driveLabel;
    juce::Label gateLabel;

    //==============================================================
    // BYPASS / LED
    //==============================================================

    juce::ToggleButton bypassButton;
    juce::Label led;

    //==============================================================
    // FOOTSWITCH STATE
    //==============================================================

    bool footswitchPressed = false;

    //==============================================================
    // ATTACHMENTS
    //==============================================================

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::SliderAttachment>
        volumeAttachment;

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::SliderAttachment>
        brightAttachment;

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::SliderAttachment>
        attackAttachment;

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::SliderAttachment>
        driveAttachment;

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::SliderAttachment>
        gateAttachment;

    std::unique_ptr<
        juce::AudioProcessorValueTreeState::ButtonAttachment>
        bypassAttachment;

    //==============================================================
    // HELPERS
    //==============================================================

    void setupKnob(
        PedalKnob&,
        juce::Slider::SliderStyle,
        bool selector = false);

    void setupLabel(
        juce::Label&,
        const juce::String&);

    void updateLED();

    //==============================================================
    // FIXED EDITOR SIZE
    //==============================================================

    static constexpr int editorWidth = 500;
    static constexpr int editorHeight = 700;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        RG_Precision_DriveAudioProcessorEditor)
};
