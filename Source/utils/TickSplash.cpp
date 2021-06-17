#include "TickSplash.h"

bool TickSplash::didShowSplashOnce = false;

// consider JUCE splash...
static const int millisecondsToDisplaySplash = 2 * 2000;
static uint32 splashDisplayTime = 0;

TickSplash::TickSplash (Component& parent)
{
    if (splashDisplayTime == 0
        || Time::getMillisecondCounter() < splashDisplayTime + (uint32) millisecondsToDisplaySplash)
    {
        logo = Drawable::createFromImageData (BinaryData::tick_icon_with_text_svg, BinaryData::tick_icon_with_text_svgSize);

        setAlwaysOnTop (true);
        parent.addAndMakeVisible (this);
    }
}

void TickSplash::paint (Graphics& g)
{
    TickSplash::didShowSplashOnce = true;
    auto area = getLocalBounds();
    g.fillAll (Colours::black);
    g.setFont (Font (15.0f));
    g.setColour (Colours::white);
    area.removeFromBottom (80);
    g.drawFittedText (JucePlugin_Manufacturer ", Copyright 2019-2021", area.removeFromBottom (40), Justification::centredBottom, 1);
    logo->drawWithin (g, area.reduced (getWidth() * 0.2).toFloat(), juce::RectanglePlacement(), 1.0f);

    if (splashDisplayTime == 0)
        splashDisplayTime = Time::getMillisecondCounter();

    if (! isTimerRunning())
        startTimer (millisecondsToDisplaySplash);
}

void TickSplash::timerCallback()
{
    if (getParentComponent() != nullptr && Time::getMillisecondCounter() >= splashDisplayTime + (uint32) millisecondsToDisplaySplash)
        getParentComponent()->removeChildComponent (this);
}

void TickSplash::parentSizeChanged()
{
    if (auto* p = getParentComponent())
        setBounds (p->getLocalBounds());
}

void TickSplash::parentHierarchyChanged()
{
    toFront (false);
}
