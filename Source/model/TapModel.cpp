#include "TapModel.h"

double TapModel::pushTap (const juce::Time& t)
{
    if (tapPoints.size() == 0)
    {
        tapPoints.push_front (t);
        return 0.0;
    }

    constexpr auto bpmDriftTolerance = 50.0;

    const auto prevPoint = tapPoints.front();
    tapPoints.push_front (t);
    const auto lastTapBpm = msToBPM ((tapPoints.front() - prevPoint).inMilliseconds());
    cleanup();
    lastDetectedBPM = estimateBPM();
    if (fabs (lastDetectedBPM - lastTapBpm) > bpmDriftTolerance)
    {
        // remove older points
        for (size_t i = 0; i < (tapPoints.size() - 1); i++)
            tapPoints.pop_back();
        lastDetectedBPM = estimateBPM();
    }
    return lastDetectedBPM;
}

double TapModel::estimateBPM()
{
    if (tapPoints.size() < 2)
        return 0.0;
    double sum = 0.0;
    for (auto it = (tapPoints.rbegin() + 1); it != tapPoints.rend(); it++)
    {
        const auto& prev = *(it - 1);
        const auto delta = *it - prev;
        const auto curBpm = msToBPM (delta.inMilliseconds());
        sum += curBpm;
    }
    return sum / (tapPoints.size() - 1);
}

int TapModel::getXforCurrentRange (const juce::Time& t, int areaWidth) const
{
    const auto start = tapPoints.back();
    const float range = (tapPoints.front() - start).inMilliseconds();
    const auto pos = t - start;

    const auto x = juce::roundToInt (juce::roundToInt (range) == 0 ? 1 : (pos.inMilliseconds() / range) * (areaWidth - 1));
    jassert (x >= 0 && x < areaWidth);
    return x;
}

void TapModel::cleanup()
{
    if (tapPoints.size() > maxPointsToKeep)
        tapPoints.pop_back();
}

void TapModel::clear()
{
    tapPoints.clear();
}

double TapModel::msToBPM (std::int64_t ms)
{
    auto secs = ms / 1000.0;
    return 60.0 / secs;
}
