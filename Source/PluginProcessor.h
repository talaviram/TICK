/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "../libsamplerate/src/samplerate.h"

#include "model/JuceState.h"

//==============================================================================
/**
*/
class TickAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    TickAudioProcessor();
    ~TickAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    static BusesProperties getDefaultLayout();

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
#endif

    void processBlock (juce::AudioSampleBuffer&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    bool isHostSyncSupported();

    juce::AudioPlayHead::CurrentPositionInfo lastKnownPosition_;

    double getCurrentBeatPos();
    int getBeat() { return tickState.beat; }

    TicksHolder& getTicks() { return ticks; };
    TickSettings& getState() { return settings; }

    juce::AudioProcessorValueTreeState& getAPVTS() { return parameters; };

private:
    double ppqEndVal;

    // audio samples bank
    TicksHolder ticks;

    // programmable assignment of samples per beat.
    // hard-coded to support upto 64.
    TickSettings settings;

    struct TickState
    {
        int currentSample = -1;
        int tickLengthInSamples = 0;
        int tickStartPosition = 0;
        int beat = 0;
        float beatGain = 1.0f;
        double beatPos = 0;
        juce::AudioSampleBuffer sample;
        float* refer[1];
        bool isClear = true;

        void fillTickSample (juce::AudioBuffer<float>& bufferToFill);
        void addTickSample (juce::AudioBuffer<float>& bufferToFill, int startPos, int endPos);
        void clear();
    } tickState;

    juce::IIRFilter lpfFilter;
    //==============================================================================
    // Parameters
    std::atomic<float> tickMultiplier {1.0f};
    std::atomic<float>* filterCutoff;
    juce::AudioProcessorValueTreeState parameters;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TickAudioProcessor)
};
