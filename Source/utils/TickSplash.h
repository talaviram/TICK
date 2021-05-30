#pragma once

#include "JuceHeader.h"

using namespace juce;
class TickSplash : public Component,
                   private Timer
{
public:
    TickSplash (Component& parentToAddTo);

    static bool didShowSplashOnce;

private:
    void paint (Graphics&) override;
    void timerCallback() override;
    void parentSizeChanged() override;
    void parentHierarchyChanged() override;

    std::unique_ptr<juce::Drawable> logo;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TickSplash)
};
