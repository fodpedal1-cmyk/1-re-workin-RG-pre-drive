#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>

//==============================================================================
// RC FILTER
//==============================================================================

float RG_Precision_DriveAudioProcessor::RCFilter::lowpass(
    float input, float cutoff, float sampleRate)
{
    if (sampleRate <= 0.0f)
        return input;

    cutoff = juce::jlimit(1.0f, sampleRate * 0.45f, cutoff);

    const float dt = 1.0f / sampleRate;
    const float rc =
        1.0f /
        (2.0f * juce::MathConstants<float>::pi * cutoff);

    const float alpha = dt / (rc + dt);

    lpState += alpha * (input - lpState);

    return lpState;
}

float RG_Precision_DriveAudioProcessor::RCFilter::highpass(
    float input, float cutoff, float sampleRate)
{
    if (sampleRate <= 0.0f)
        return input;

    cutoff = juce::jlimit(1.0f, sampleRate * 0.45f, cutoff);

    const float dt = 1.0f / sampleRate;
    const float rc =
        1.0f /
        (2.0f * juce::MathConstants<float>::pi * cutoff);

    const float alpha = rc / (rc + dt);

    const float output =
        alpha * (hpState + input - hpInput);

    hpState = output;
    hpInput = input;

    return output;
}

void RG_Precision_DriveAudioProcessor::RCFilter::reset()
{
    lpState = 0.0f;
    hpState = 0.0f;
    hpInput = 0.0f;
}

//==============================================================================
// DC BLOCK
//==============================================================================

float RG_Precision_DriveAudioProcessor::DCBlock::process(float input)
{
    constexpr float R = 1000000.0f;
    constexpr float C = 47.0e-9f;

    const float alpha =
        R * C /
        (R * C + 1.0f / 44100.0f);

    const float output =
        input - x1 + alpha * y1;

    x1 = input;
    y1 = output;

    return output;
}

void RG_Precision_DriveAudioProcessor::DCBlock::reset()
{
    x1 = 0.0f;
    y1 = 0.0f;
}

//==============================================================================
// OP AMP MODEL
//==============================================================================

float RG_Precision_DriveAudioProcessor::OpAmpModel::process(
    float input,
    float gain,
    float sampleRate)
{
    gain = juce::jmax(1.0f, gain);

    const float target = input * gain;

    float bandwidth = gainBandwidth / gain;

    bandwidth =
        juce::jlimit(
            20.0f,
            sampleRate * 0.45f,
            bandwidth);

    const float dt = 1.0f / sampleRate;

    const float alpha =
        1.0f -
        std::exp(
            -2.0f *
            juce::MathConstants<float>::pi *
            bandwidth *
            dt);

    float desired =
        state +
        alpha * (target - state);

    const float maxChange =
        slewRate * dt;

    const float difference =
        desired - state;

    if (difference > maxChange)
        desired = state + maxChange;
    else if (difference < -maxChange)
        desired = state - maxChange;

    desired =
        juce::jlimit(
            -outputLimit,
            outputLimit,
            desired);

    state = desired;

    return state;
}

void RG_Precision_DriveAudioProcessor::OpAmpModel::reset()
{
    state = 0.0f;
}

//==============================================================================
// DIODE CLIPPER
//==============================================================================

float RG_Precision_DriveAudioProcessor::DiodeClipper::process(
    float input)
{
    const float sign =
        input >= 0.0f ? 1.0f : -1.0f;

    const float x = std::abs(input);

    if (x <= forwardVoltage)
        return input;

    const float excess =
        x - forwardVoltage;

    const float clipped =
        forwardVoltage +
        softness *
        std::tanh(excess / softness);

    return sign * clipped;
}

//==============================================================================
// ATTACK NETWORK
// ORIGINAL HIGH-PASS
//==============================================================================

float RG_Precision_DriveAudioProcessor::AttackNetwork::process(
    float input,
    int attackPosition,
    float sampleRate)
{
    attackPosition =
        juce::jlimit(
            1,
            6,
            attackPosition);

    const int index =
        attackPosition - 1;

    const float C =
        capacitors[index];

    const float cutoff =
        1.0f /
        (2.0f *
         juce::MathConstants<float>::pi *
         resistance *
         C);

    return filter.highpass(
        input,
        cutoff,
        sampleRate);
}

void RG_Precision_DriveAudioProcessor::AttackNetwork::reset()
{
    filter.reset();
}

//==============================================================================
// GATE
// ORIGINAL GATE DSP — UNCHANGED
//==============================================================================

float RG_Precision_DriveAudioProcessor::GateDetector::process(
    float input,
    float gateAmount,
    float sampleRate)
{
    gateAmount =
        juce::jlimit(
            0.0f,
            1.0f,
            gateAmount);

    if (gateAmount <= 0.001f)
    {
        gain = 1.0f;
        envelope = std::abs(input);
        return input;
    }

    const float level =
        std::abs(input);

    constexpr float envelopeAttackMs = 1.0f;
    constexpr float envelopeReleaseMs = 80.0f;

    const float attackCoeff =
        std::exp(
            -1.0f /
            (sampleRate *
             envelopeAttackMs *
             0.001f));

    const float releaseCoeff =
        std::exp(
            -1.0f /
            (sampleRate *
             envelopeReleaseMs *
             0.001f));

    if (level > envelope)
    {
        envelope =
            attackCoeff * envelope +
            (1.0f - attackCoeff) * level;
    }
    else
    {
        envelope =
            releaseCoeff * envelope +
            (1.0f - releaseCoeff) * level;
    }

    const float threshold =
        0.0015f +
        std::pow(
            gateAmount,
            0.65f) *
        0.12f;

    const float knee =
        juce::jmax(
            0.001f,
            threshold * 0.18f);

    float targetGain = 1.0f;

    if (envelope <= threshold - knee)
    {
        targetGain = 0.0f;
    }
    else if (envelope >= threshold + knee)
    {
        targetGain = 1.0f;
    }
    else
    {
        const float position =
            (envelope -
             (threshold - knee)) /
            (2.0f * knee);

        const float curved =
            position *
            position *
            (3.0f - 2.0f * position);

        targetGain = curved;
    }

    constexpr float gateAttackMs = 0.5f;
    constexpr float gateReleaseMs = 60.0f;

    const float gateAttackCoeff =
        std::exp(
            -1.0f /
            (sampleRate *
             gateAttackMs *
             0.001f));

    const float gateReleaseCoeff =
        std::exp(
            -1.0f /
            (sampleRate *
             gateReleaseMs *
             0.001f));

    if (targetGain > gain)
    {
        gain =
            gateAttackCoeff * gain +
            (1.0f - gateAttackCoeff) *
            targetGain;
    }
    else
    {
        gain =
            gateReleaseCoeff * gain +
            (1.0f - gateReleaseCoeff) *
            targetGain;
    }

    return input * gain;
}

void RG_Precision_DriveAudioProcessor::GateDetector::reset()
{
    envelope = 0.0f;
    gain = 1.0f;
}

//==============================================================================
// BRIGHT NETWORK
// ORIGINAL BRIGHT DSP — UNCHANGED
//==============================================================================

float RG_Precision_DriveAudioProcessor::BrightNetwork::process(
    float input,
    float brightAmount,
    float sampleRate)
{
    brightAmount =
        juce::jlimit(
            0.0f,
            1.0f,
            brightAmount);

    if (sampleRate <= 0.0f)
        return input;

    const float cutoff =
        1500.0f +
        brightAmount * 4500.0f;

    const float low =
        filter.lowpass(
            input,
            cutoff,
            sampleRate);

    const float high =
        input - low;

    const float boost =
        brightAmount * 4.0f;

    const float output =
        input + high * boost;

    return output;
}

void RG_Precision_DriveAudioProcessor::BrightNetwork::reset()
{
    filter.reset();
}

//==============================================================================
// CONSTRUCTOR
//==============================================================================

RG_Precision_DriveAudioProcessor::
RG_Precision_DriveAudioProcessor()

    : AudioProcessor(
        BusesProperties()
            .withInput(
                "Input",
                juce::AudioChannelSet::stereo(),
                true)
            .withOutput(
                "Output",
                juce::AudioChannelSet::stereo(),
                true)),

      parameters(
          *this,
          nullptr,
          "PARAMETERS",
          createParameterLayout())
{
}

//==============================================================================
// DESTRUCTOR
//==============================================================================

RG_Precision_DriveAudioProcessor::
~RG_Precision_DriveAudioProcessor()
{
}

//==============================================================================
// PARAMETER LAYOUT
//==============================================================================

juce::AudioProcessorValueTreeState::ParameterLayout
RG_Precision_DriveAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>>
        params;

    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "VOLUME",
            "VOL",
            0.0f,
            1.0f,
            0.75f));

    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "BRIGHT",
            "BRIGHT",
            0.0f,
            1.0f,
            0.50f));

    params.push_back(
        std::make_unique<juce::AudioParameterInt>(
            "ATTACK",
            "ATTACK",
            1,
            6,
            3));

    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "DRIVE",
            "DRIVE",
            1.0f,
            11.0f,
            5.0f));

    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "GATE",
            "GATE",
            0.0f,
            1.0f,
            0.15f));

    params.push_back(
        std::make_unique<juce::AudioParameterBool>(
            "BYPASS",
            "BYPASS",
            false));

    return { params.begin(), params.end() };
}

//==============================================================================
// PREPARE TO PLAY
//==============================================================================

void RG_Precision_DriveAudioProcessor::prepareToPlay(
    double sampleRate,
    int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);

    currentSampleRate = sampleRate;

    volumeSmoothed.reset(
        sampleRate,
        0.020);

    brightSmoothed.reset(
        sampleRate,
        0.020);

    driveSmoothed.reset(
        sampleRate,
        0.020);

    gateSmoothed.reset(
        sampleRate,
        0.020);

    attackSmoothed.reset(
        sampleRate,
        0.020);

    volumeSmoothed.setCurrentAndTargetValue(
        *parameters.getRawParameterValue("VOLUME"));

    brightSmoothed.setCurrentAndTargetValue(
        *parameters.getRawParameterValue("BRIGHT"));

    driveSmoothed.setCurrentAndTargetValue(
        *parameters.getRawParameterValue("DRIVE"));

    gateSmoothed.setCurrentAndTargetValue(
        *parameters.getRawParameterValue("GATE"));

    attackSmoothed.setCurrentAndTargetValue(
        *parameters.getRawParameterValue("ATTACK"));

    dcBlockL.reset();
    dcBlockR.reset();

    gateL.reset();
    gateR.reset();

    attackL.reset();
    attackR.reset();

    opAmpBufferL.reset();
    opAmpBufferR.reset();

    opAmpDriveL.reset();
    opAmpDriveR.reset();

    diodeL.forwardVoltage = 0.62f;
    diodeR.forwardVoltage = 0.62f;

    diodeL.softness = 0.12f;
    diodeR.softness = 0.12f;

    brightL.reset();
    brightR.reset();

    outputLevel.store(
        0.0f,
        std::memory_order_relaxed);
}

//==============================================================================
// RELEASE RESOURCES
//==============================================================================

void RG_Precision_DriveAudioProcessor::releaseResources()
{
}

//==============================================================================
// BUS LAYOUT
//==============================================================================

bool RG_Precision_DriveAudioProcessor::isBusesLayoutSupported(
    const BusesLayout& layouts) const
{
    const auto& mainIn =
        layouts.getChannelSet(true, 0);

    const auto& mainOut =
        layouts.getChannelSet(false, 0);

    if (mainOut !=
            juce::AudioChannelSet::mono() &&
        mainOut !=
            juce::AudioChannelSet::stereo())
    {
        return false;
    }

    if (mainIn != mainOut)
        return false;

    return true;
}

//==============================================================================
// PROCESS BLOCK
// ORIGINAL DSP + OUTPUT METER ONLY
//==============================================================================

void RG_Precision_DriveAudioProcessor::processBlock(
    juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);

    juce::ScopedNoDenormals noDenormals;

    const int numChannels =
        buffer.getNumChannels();

    const int numSamples =
        buffer.getNumSamples();

    if (numChannels <= 0 || numSamples <= 0)
    {
        outputLevel.store(
            0.0f,
            std::memory_order_relaxed);

        return;
    }

    if (parameters.getRawParameterValue(
            "BYPASS")->load() > 0.5f)
    {
        outputLevel.store(
            0.0f,
            std::memory_order_relaxed);

        return;
    }

    volumeSmoothed.setTargetValue(
        *parameters.getRawParameterValue(
            "VOLUME"));

    brightSmoothed.setTargetValue(
        *parameters.getRawParameterValue(
            "BRIGHT"));

    driveSmoothed.setTargetValue(
        *parameters.getRawParameterValue(
            "DRIVE"));

    gateSmoothed.setTargetValue(
        *parameters.getRawParameterValue(
            "GATE"));

    attackSmoothed.setTargetValue(
        *parameters.getRawParameterValue(
            "ATTACK"));

    float peakLevel = 0.0f;

    for (int sample = 0;
         sample < numSamples;
         ++sample)
    {
        const float volume =
            volumeSmoothed.getNextValue();

        const float bright =
            brightSmoothed.getNextValue();

        const float drive =
            driveSmoothed.getNextValue();

        const float gate =
            gateSmoothed.getNextValue();

        const float attack =
            attackSmoothed.getNextValue();

        const int currentAttack =
            juce::jlimit(
                1,
                6,
                static_cast<int>(
                    std::round(attack)));

        //==========================================================
        // LEFT CHANNEL
        //==========================================================

        if (numChannels >= 1)
        {
            float x =
                buffer.getSample(0, sample);

            x = processSample(x, 0);

            x = processGate(
                x,
                gate,
                0);

            x = processAttack(
                x,
                currentAttack,
                0);

            x = processDrive(
                x,
                drive,
                0);

            x = processBright(
                x,
                bright,
                0);

            if (volume <= 0.0001f)
                x = 0.0f;
            else
                x *= volume * 2.40f;

            x =
                juce::jlimit(
                    -1.0f,
                    1.0f,
                    x);

            peakLevel =
                juce::jmax(
                    peakLevel,
                    std::abs(x));

            buffer.setSample(
                0,
                sample,
                x);
        }

        //==========================================================
        // RIGHT CHANNEL
        //==========================================================

        if (numChannels >= 2)
        {
            float x =
                buffer.getSample(1, sample);

            x = processSample(x, 1);

            x = processGate(
                x,
                gate,
                1);

            x = processAttack(
                x,
                currentAttack,
                1);

            x = processDrive(
                x,
                drive,
                1);

            x = processBright(
                x,
                bright,
                1);

            if (volume <= 0.0001f)
                x = 0.0f;
            else
                x *= volume * 2.40f;

            x =
                juce::jlimit(
                    -1.0f,
                    1.0f,
                    x);

            peakLevel =
                juce::jmax(
                    peakLevel,
                    std::abs(x));

            buffer.setSample(
                1,
                sample,
                x);
        }
    }

    outputLevel.store(
        juce::jlimit(
            0.0f,
            1.0f,
            peakLevel),
        std::memory_order_relaxed);
}

//==============================================================================
// INPUT / BUFFER
//==============================================================================

float RG_Precision_DriveAudioProcessor::processSample(
    float input,
    int channel)
{
    if (channel == 0)
    {
        float x =
            dcBlockL.process(input);

        x =
            opAmpBufferL.process(
                x,
                1.0f,
                static_cast<float>(
                    currentSampleRate));

        return x;
    }

    float x =
        dcBlockR.process(input);

    x =
        opAmpBufferR.process(
            x,
            1.0f,
            static_cast<float>(
                currentSampleRate));

    return x;
}

//==============================================================================
// GATE PROCESS
//==============================================================================

float RG_Precision_DriveAudioProcessor::processGate(
    float input,
    float gateAmount,
    int channel)
{
    gateAmount =
        juce::jlimit(
            0.0f,
            1.0f,
            gateAmount);

    if (channel == 0)
    {
        return gateL.process(
            input,
            gateAmount,
            static_cast<float>(
                currentSampleRate));
    }

    return gateR.process(
        input,
        gateAmount,
        static_cast<float>(
            currentSampleRate));
}

//==============================================================================
// ATTACK PROCESS
//==============================================================================

float RG_Precision_DriveAudioProcessor::processAttack(
    float input,
    float attack,
    int channel)
{
    const int position =
        juce::jlimit(
            1,
            6,
            static_cast<int>(
                std::round(attack)));

    if (channel == 0)
    {
        return attackL.process(
            input,
            position,
            static_cast<float>(
                currentSampleRate));
    }

    return attackR.process(
        input,
        position,
        static_cast<float>(
            currentSampleRate));
}

//==============================================================================
// DRIVE PROCESS
// ORIGINAL RESISTOR-BASED GAIN
//==============================================================================

float RG_Precision_DriveAudioProcessor::processDrive(
    float input,
    float drive,
    int channel)
{
    drive =
        juce::jlimit(
            1.0f,
            11.0f,
            drive);

    constexpr float Rg = 4700.0f;
    constexpr float RfMinimum = 10000.0f;
    constexpr float RfMaximumAdditional = 500000.0f;

    const float driveNormalized =
        (drive - 1.0f) / 10.0f;

    const float Rf =
        RfMinimum +
        driveNormalized *
        RfMaximumAdditional;

    const float gain =
        1.0f +
        Rf / Rg;

    if (channel == 0)
    {
        float x =
            opAmpDriveL.process(
                input,
                gain,
                static_cast<float>(
                    currentSampleRate));

        x =
            diodeL.process(x);

        return x;
    }

    float x =
        opAmpDriveR.process(
            input,
            gain,
            static_cast<float>(
                currentSampleRate));

    x =
        diodeR.process(x);

    return x;
}

//==============================================================================
// BRIGHT PROCESS
//==============================================================================

float RG_Precision_DriveAudioProcessor::processBright(
    float input,
    float bright,
    int channel)
{
    bright =
        juce::jlimit(
            0.0f,
            1.0f,
            bright);

    if (channel == 0)
    {
        return brightL.process(
            input,
            bright,
            static_cast<float>(
                currentSampleRate));
    }

    return brightR.process(
        input,
        bright,
        static_cast<float>(
            currentSampleRate));
}

//==============================================================================
// EDITOR
//==============================================================================

juce::AudioProcessorEditor*
RG_Precision_DriveAudioProcessor::createEditor()
{
    return new RG_Precision_DriveAudioProcessorEditor(*this);
}

bool RG_Precision_DriveAudioProcessor::hasEditor() const
{
    return true;
}

//==============================================================================
// PLUGIN INFORMATION
//==============================================================================

const juce::String
RG_Precision_DriveAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool RG_Precision_DriveAudioProcessor::acceptsMidi() const
{
    return false;
}

bool RG_Precision_DriveAudioProcessor::producesMidi() const
{
    return false;
}

bool RG_Precision_DriveAudioProcessor::isMidiEffect() const
{
    return false;
}

double RG_Precision_DriveAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

//==============================================================================
// PROGRAMS
//==============================================================================

int RG_Precision_DriveAudioProcessor::getNumPrograms()
{
    return 1;
}

int RG_Precision_DriveAudioProcessor::getCurrentProgram()
{
    return 0;
}

void RG_Precision_DriveAudioProcessor::setCurrentProgram(
    int index)
{
    juce::ignoreUnused(index);
}

const juce::String
RG_Precision_DriveAudioProcessor::getProgramName(
    int index)
{
    juce::ignoreUnused(index);
    return {};
}

void RG_Precision_DriveAudioProcessor::changeProgramName(
    int index,
    const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
// STATE
//==============================================================================

void RG_Precision_DriveAudioProcessor::getStateInformation(
    juce::MemoryBlock& destData)
{
    if (auto state =
            parameters.copyState().createXml())
    {
        copyXmlToBinary(
            *state,
            destData);
    }
}

void RG_Precision_DriveAudioProcessor::setStateInformation(
    const void* data,
    int sizeInBytes)
{
    if (auto state =
            getXmlFromBinary(
                data,
                sizeInBytes))
    {
        if (state->hasTagName(
                parameters.state.getType()))
        {
            parameters.replaceState(
                juce::ValueTree::fromXml(
                    *state));
        }
    }
}

//==============================================================================
// JUCE PLUGIN FACTORY
//==============================================================================

juce::AudioProcessor*
JUCE_CALLTYPE createPluginFilter()
{
    return new RG_Precision_DriveAudioProcessor();
}
