// Provide plug-in state as JSON
#pragma once

#include "PluginProcessor.h"

class JsonState
{
public:
    JsonState (TickAudioProcessor&);
    juce::String transport();

    // https://forum.juce.com/t/valuetree-json/55279
    juce::String valueTreeToJsonString (const juce::ValueTree&, const int numDecimalPlaces = 3, const int indent = 4);
    juce::ValueTree valueTreeFromJsonString (const juce::String&, juce::Identifier);

private:
    TickAudioProcessor& processor;
};
