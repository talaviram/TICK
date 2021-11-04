#pragma once

#include <JuceHeader.h>
#include <optional>

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

    struct Requests
    {
        std::optional<bool> isPlaying;
        std::optional<float> bpm;
    };

    // to be used on audio callback for the actual 'Link' sync
    void linkPosition (juce::AudioPlayHead::CurrentPositionInfo&, Requests);
private:
    ABLLinkRef ablLink;
};
#endif
