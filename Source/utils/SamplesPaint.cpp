#include "SamplesPaint.h"

SamplesPaint::SamplesPaint (TicksHolder& holder)
    : thumbCache (4), ticks (holder)
{
    ticks.addChangeListener (this);
    changeListenerCallback (nullptr);
}

SamplesPaint::~SamplesPaint()
{
    ticks.removeChangeListener (this);
}

void SamplesPaint::changeListenerCallback (juce::ChangeBroadcaster*)
{
    thumbnails.clear();
    for (size_t i = 0; i < ticks.getNumOfTicks(); i++)
    {
        thumbnails.push_back (std::make_unique<juce::AudioThumbnail> (1, audioFormatManager, thumbCache));
        // all tick samples are stored as 44.1khz/32bit upto 2 seconds!
        thumbnails[i]->reset (1, BASE_SAMPLERATE);
        if (i < ticks.getNumOfTicks())
        {
            float* samples[1];
            samples[0] = ticks[i].getTickSource();
            thumbnails[i]->addBlock (0, juce::AudioBuffer<float> (samples, 1, ticks[i].getSourceLengthInSamples()), 0, ticks[i].getSourceLengthInSamples());
        }
    }
    sendChangeMessage();
}

void SamplesPaint::drawTick (juce::Graphics& g, juce::Rectangle<int> bounds, const int idx, const float scale, juce::Colour baseColour)
{
    auto tickIndex = static_cast<size_t> (idx);
    jassert (tickIndex < ticks.getNumOfTicks());
    const auto width = bounds.getWidth();
    const auto& tick = ticks[tickIndex];
    const auto totalLengthInSecs = tick.getSourceLengthInSec();
    const auto gain = tick.getGain() * scale;
    g.setColour (baseColour.withAlpha (0.3f));
    if (tickIndex >= thumbnails.size())
    {
        return; // thumbnail isn't ready yet! usually when editing
    }
    thumbnails[tickIndex]->drawChannels (g, bounds.removeFromLeft (static_cast<int> (width * (tick.getStartInSec() / totalLengthInSecs))), 0.0, tick.getStartInSec(), gain);
    thumbnails[tickIndex]->drawChannels (g, bounds.removeFromRight (static_cast<int> (width * (1.0 - tick.getEndInSec() / totalLengthInSecs))), tick.getEndInSec(), totalLengthInSecs, gain);
    g.setColour (baseColour);
    thumbnails[tickIndex]->drawChannels (g, bounds, tick.getStartInSec(), tick.getEndInSec(), gain);
}
