/*
  ==============================================================================

    ManageSamplesView.cpp
    Created: 2 Oct 2020 12:44:40pm
    Author:  Tal Aviram

  ==============================================================================
*/

#include "ManageSamplesView.h"
#include "DialogComponent.h"
#include "JUX/components/ListBoxMenu.h"
#include "LookAndFeel.h"
#include "utils/UtilityFunctions.h"

ManageSamplesView::ManageSamplesView (SamplesPaint& samplesPaint, TicksHolder& ticksRef)
    : ticks (ticksRef), samplesPaint (samplesPaint)
{
    using namespace juce;

    addAndMakeVisible (editSection);
    addAndMakeVisible (closeButton);

    invalidateSampleCells();
    samplesPaint.addChangeListener (this);
}

void ManageSamplesView::updateSelection (const int index)
{
    lastSelection = index;
    auto* tick = index >= 0 && ticks.getNumOfTicks() > 0 ? &ticks[index] : nullptr;
    editSection.setSelectedSample (tick);
}

void ManageSamplesView::invalidateSampleCells()
{
    resized();
    updateSelection (lastSelection);
}

void ManageSamplesView::resized()
{
#if JUCE_IOS || JUCE_ANDROID
    constexpr auto topZoneHeight = 50;
    constexpr auto reducedArea = 14;
#else
    constexpr auto topZoneHeight = 30;
    constexpr auto reducedArea = 4;
#endif
    constexpr auto editViewHeight = 150;
    closeButton.setBounds (getLocalBounds().removeFromTop (topZoneHeight).removeFromRight (topZoneHeight).reduced (reducedArea));
    auto area = getLocalBounds().reduced (8, 0);
    area.removeFromTop (topZoneHeight);
    editSection.setBounds (area.removeFromBottom (editViewHeight));
}

void ManageSamplesView::paint (juce::Graphics& g)
{
    Path p;
    p.addRoundedRectangle (0, 0, getWidth(), getHeight(), 5.0f, 5.0f, true, true, false, false);
    g.setColour (juce::Colours::black);
    g.fillPath (p);
}

void ManageSamplesView::changeListenerCallback (juce::ChangeBroadcaster*)
{
    invalidateSampleCells();
}

/// EditSection
ManageSamplesView::EditSection::EditSection()
{
    name.setFont (getLookAndFeel().getPopupMenuFont());
    name.setJustificationType (juce::Justification::centred);
    volume.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    volume.setRange (0.1f, 2.0f);
    volume.getValueObject().addListener (this);
    range.setSliderStyle (juce::Slider::TwoValueHorizontal);
    range.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    range.getMinValueObject().addListener (this);
    range.getMaxValueObject().addListener (this);

    addAndMakeVisible (name);
    addAndMakeVisible (volume);
    addAndMakeVisible (range);
}

void ManageSamplesView::EditSection::valueChanged (juce::Value& value)
{
    if (selectedTick == nullptr)
        return;

    if (value.refersToSameSourceAs (volume.getValueObject()))
        selectedTick->setGain (value.getValue());
    else if (value.refersToSameSourceAs (range.getMinValueObject()) || value.refersToSameSourceAs (range.getMaxValueObject()))
        selectedTick->setRange (range.getMinValue(), range.getMaxValue());
    getParentComponent()->repaint();
}

void ManageSamplesView::EditSection::setSelectedSample (Tick* tick)
{
    selectedTick = tick;
    setVisible (selectedTick != nullptr);
    if (getParentComponent())
        getParentComponent()->resized();
    if (isVisible())
    {
        name.setText (selectedTick->getName(), juce::dontSendNotification);
        // all tick samples are stored as mono 44.1khz/32bit upto 2 seconds!
        range.setRange (0.0, selectedTick->getSourceLengthInSamples() / BASE_SAMPLERATE);
        range.setPopupMenuEnabled (true);
        range.setMinValue (selectedTick->getStartInSec());
        range.setMaxValue (selectedTick->getEndInSec());
        volume.setValue (selectedTick->getGain());
    }
}

void ManageSamplesView::EditSection::resized()
{
    auto area = getLocalBounds();
    {
        auto topArea = area.removeFromTop (40);
        name.setBounds (topArea);
    }
    range.setBounds (area.removeFromBottom (40));
    volume.setBounds (area.removeFromBottom (40));
}

void ManageSamplesView::EditSection::paint (juce::Graphics& g)
{
    auto gradientFill = juce::ColourGradient (juce::Colours::black.withAlpha (0.3F), 0, 0, juce::Colours::darkgrey.withAlpha (0.3F), 0, getLocalBounds().getCentreY(), false);
    g.setGradientFill (gradientFill);
    g.fillRoundedRectangle (getLocalBounds().toFloat(), 3.0f);
    g.setColour (juce::Colours::white.withAlpha (0.4f));
}

ManageSamplesView::CloseButton::CloseButton()
    : juce::Button ("CloseButton")
{
}

void ManageSamplesView::CloseButton::paintButton (juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    g.setColour (juce::Colours::grey.withAlpha (0.6f));
    g.fillEllipse (getLocalBounds().toFloat());
    g.setColour (juce::Colours::white);
    constexpr auto pad = 6;
    g.drawLine (pad, pad, getWidth() - pad, getHeight() - pad, 1.5f);
    g.drawLine (pad, getHeight() - pad, getWidth() - pad, pad, 1.5f);
}
