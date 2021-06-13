#pragma once

#include <JuceHeader.h>

class TapModel
{
public:
    // push tap and returns current detected BPM
    double pushTap (const juce::Time&);
    void clear();

    double getLastDetectedBPM() const { return lastDetectedBPM; }

    int getXforCurrentRange (const juce::Time& t, int areaWidth) const;

    static double msToBPM (std::int64_t ms);

private:
    double estimateBPM();
    void cleanup();

    static constexpr auto maxPointsToKeep = 9;
    std::deque<juce::Time> tapPoints;
    double lastDetectedBPM { 0 };
};
