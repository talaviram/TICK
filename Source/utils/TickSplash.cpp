#include "TickSplash.h"

static const int millisecondsToDisplaySplash = 3000;
static uint32 splashDisplayTime = 0;

TickSplash::TickSplash (Component& parent)
{
    if (splashDisplayTime == 0
        || Time::getMillisecondCounter() < splashDisplayTime + (uint32) millisecondsToDisplaySplash)
    {
        logo = Drawable::createFromImageData (BinaryData::ticklogo_svg, BinaryData::ticklogo_svgSize);

        setAlwaysOnTop (true);
        parent.addAndMakeVisible (this);
    }
}

void TickSplash::paint (Graphics& g)
{
    auto area = getLocalBounds();
    g.fillAll (Colours::black);
    g.setFont (Font (15.0f));
    g.setColour (Colours::white);
    area.removeFromBottom (80);
    g.drawFittedText (JucePlugin_Manufacturer ", Copyright 2019-2021", area.removeFromBottom (40), Justification::centredBottom, 1);
    g.setFont (Font (40.0f));
    g.drawFittedText (JucePlugin_Desc, area.removeFromBottom (40), Justification::centredTop, 1);
    logo->drawWithin (g, area.toFloat(), juce::RectanglePlacement(), 1.0f);

    if (splashDisplayTime == 0)
        splashDisplayTime = Time::getMillisecondCounter();

    if (! isTimerRunning())
        startTimer (millisecondsToDisplaySplash);
}

void TickSplash::timerCallback()
{
    if (Time::getMillisecondCounter() >= splashDisplayTime + (uint32) millisecondsToDisplaySplash)
        delete this;
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
