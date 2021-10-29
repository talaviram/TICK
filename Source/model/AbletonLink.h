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

    bool isLinkEnabled();
    bool isLinkConnected();

    // to be used on audio callback for the actual 'Link' sync
    void linkPosition (juce::AudioPlayHead::CurrentPositionInfo& pos);
private:
    ABLLinkRef ablLink;
};
#endif
