#include "TopBar.h"

TopBar::TopBar() : leftButton ("LeftButton", juce::DrawableButton::ImageFitted), rightButton ("RightButton", juce::DrawableButton::ButtonStyle::ImageFitted)
{
    using namespace juce;

    setFocusContainerType (FocusContainerType::focusContainer);

    addAndMakeVisible (leftButton);
    addAndMakeVisible (rightButton);

    centerLabel.setFont (TickLookAndFeel::topBarTextSize);
    centerLabel.setJustificationType (juce::Justification::left);
    addAndMakeVisible (centerLabel);
}

void TopBar::resized()
{
    auto area = getLocalBounds();
    leftButton.setBounds (area.removeFromLeft (getHeight()).reduced (TickLookAndFeel::reducePixels * 2));
    rightButton.setBounds (area.removeFromRight (getHeight()).reduced (TickLookAndFeel::reducePixels * 2));
    if (extendedTopBar)
    {
        centerLabel.setBounds (area.removeFromLeft (250));
        extendedBarArea = area;
    }
    else
    {
        centerLabel.setBounds (area.reduced (15, 0));
        extendedBarArea = {};
    }
}
