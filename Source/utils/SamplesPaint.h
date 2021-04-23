#pragma once

#include "model/JuceState.h"

class SamplesPaint
    : public juce::ChangeBroadcaster,
      juce::ChangeListener
{
public:
    SamplesPaint (TicksHolder& ticks);
    ~SamplesPaint();

    void drawTick (juce::Graphics& g, juce::Rectangle<int> bounds, int tickIndex, float scale = 1.0f, juce::Colour baseColour = juce::Colours::white.withAlpha (0.3f));

private:
    void changeListenerCallback (juce::ChangeBroadcaster* source) override;

    juce::AudioThumbnailCache thumbCache;
    juce::AudioFormatManager audioFormatManager;
    std::vector<std::unique_ptr<juce::AudioThumbnail>> thumbnails;
    TicksHolder& ticks;
};
