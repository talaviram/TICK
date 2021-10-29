#pragma once

#include <JuceHeader.h>

#if JUCE_IOS
#include "ABLLink.h"
class AbletonLink
{
public:
    AbletonLink (double initBpm = 120.0);
    ~AbletonLink();
    void showSettings (juce::Component& source, std::function<void()> onDismiss);
private:
    ABLLinkRef ablLink;
};
#endif
