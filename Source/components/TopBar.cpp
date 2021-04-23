#include "TopBar.h"

TopBar::TopBar() : leftButton ("LeftButton", juce::DrawableButton::ImageFitted), rightButton ("RightButton", juce::DrawableButton::ButtonStyle::ImageFitted)
{
    using namespace juce;

    addAndMakeVisible (leftButton);
    addAndMakeVisible (rightButton);

    centerLabel.setFont (juce::Font (30.0));
    centerLabel.setJustificationType (juce::Justification::left);
    addAndMakeVisible (centerLabel);
}

void TopBar::resized()
{
    auto area = getLocalBounds();
    leftButton.setBounds (area.removeFromLeft (getHeight()).reduced (5));
    rightButton.setBounds (area.removeFromRight (getHeight()).reduced (5));
    centerLabel.setBounds (area.reduced (15, 0));
}
