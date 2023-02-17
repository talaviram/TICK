/*
  ==============================================================================

    BottomBar.h
    Created: 6 Sep 2021 3:05:19pm
    Author:  Tal Aviram

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class TransportBar : public juce::Component
{
public:
    TransportBar();
    void resized() override;
    juce::DrawableButton transportButton;
    juce::DrawableButton syncIndicator, preCountIndicator;
    juce::Label transportPosition;
};
