/*
  ==============================================================================

    BottomBar.cpp
    Created: 6 Sep 2021 3:05:19pm
    Author:  Tal Aviram

  ==============================================================================
*/

#include "TransportBar.h"

using namespace juce;

TransportBar::TransportBar()
    : transportButton ("transportButton", juce::DrawableButton::ImageFitted), syncIndicator ("syncIndicator", juce::DrawableButton::ImageAboveTextLabel)
{
    auto syncOn = juce::Drawable::createFromImageData (BinaryData::lock_clock24px_svg, BinaryData::lock_clock24px_svgSize);
    syncOn->replaceColour (Colours::black, Colours::white);
    syncIndicator.setTooltip ("Transport Controlled from Host");
    syncIndicator.setButtonText ("EXT SYNC");
    syncIndicator.setImages (syncOn.get(), nullptr, nullptr, nullptr);
    syncIndicator.setInterceptsMouseClicks (false, false);
    addChildComponent (syncIndicator);

    auto transportIconPlay = Drawable::createFromImageData (BinaryData::playbutton_svg, BinaryData::playbutton_svgSize);
    auto transportIconStop = Drawable::createFromImageData (BinaryData::stopbutton_svg, BinaryData::stopbutton_svgSize);
    transportIconPlay->replaceColour (Colours::black, Colours::white);
    transportIconStop->replaceColour (Colours::black, Colours::white);
    transportButton.setImages (transportIconPlay.get(), nullptr, nullptr, nullptr, transportIconStop.get());
    transportButton.setColour (juce::DrawableButton::ColourIds::backgroundColourId, juce::Colours::transparentBlack);
    transportButton.setColour (juce::DrawableButton::ColourIds::backgroundOnColourId, juce::Colours::transparentBlack);
    transportButton.setClickingTogglesState (true);
    addAndMakeVisible (transportButton);

    transportPosition.setInterceptsMouseClicks (false, false);
    transportPosition.setJustificationType (juce::Justification::centred);
    transportPosition.setText ("0|0|000", juce::dontSendNotification);
    addAndMakeVisible (transportPosition);
}

void TransportBar::resized()
{
    auto area = getLocalBounds();
    transportPosition.setBounds (area);
    transportButton.setBounds (area.reduced (6, 0));
    syncIndicator.setBounds (transportButton.getBounds().removeFromLeft (getHeight()).reduced (4));
}
