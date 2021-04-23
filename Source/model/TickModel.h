/*
  ==============================================================================

    TickModel.h
    Provides common models for the actual tick.
    Tick is pretty much a 'one-shot' sample to be played on a specific musical
    division.
    Created: 13 Sep 2019 9:36:39am
    Author:  Tal Aviram

  ==============================================================================
*/

#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

static const double BASE_SAMPLERATE = 44100.0;

/* A mono (single-channel) audio sample of up to 2.0seconds.
   Stored as (float) 32bit / 44.1khz.
 */
struct Tick
{
public:
    /* Constructor for tick. */
    Tick (const char* name, const float** audioToSave, int numOfSamples, int numOfChannels = 1, double sampleRate = BASE_SAMPLERATE);

    std::string getName() const;

    // Due to use of juce::AudioBuffer we return non-consts...

    /* Returns pointer to audio samples buffer.
       (It'll be resampled to last sample rate set)
       If range is set, this will point from the start pos sample.
     */
    float* getTickAudioBuffer();
    /* Returns length in samples.
       If range is set, length will be based on end pos.
     */
    int getLengthInSamples() const;

    /* Returns base stored data in 44.1khz
     */
    float* getTickSource();

    int getSourceLengthInSamples() const;
    double getSourceLengthInSec() const;

    /* Resample internal buffer if needed to current samplerate.
       This method *will* allocate memory.
     */
    void setSampleRate (double newSampleRate);
    double getSampleRate() const;

    float getGain() const;
    void setGain (float newGain);

    void setRange (double startInSec, double endInSec);
    double getStartInSec() const;
    double getEndInSec() const;

private:
    void mono_resample (const float* src, float* dst, double ratio, int numOfInSamples, int numOfOutSamples);

    std::string name;
    double lastSampleRate;

    std::unique_ptr<float[]> sample, resampled;
    int numOfTickSamples;
    int numOfResampledSamples;

    float gain { 1.0f };

    // allows fine tune within sample.
    double startPosInSec, endPosInSec;

    double resampledSamplerate;
};

//==============================================================================

struct Ticks
{
public:
    void addTick (std::unique_ptr<Tick>&& tickToAdd);
    void removeTick (int idx);
    void replaceTick (int idx, std::unique_ptr<Tick>&& newTick);

    void addTicks (std::vector<std::unique_ptr<Tick>> ticks, const double samplerate, bool clearTicks);

    Tick& operator[] (std::size_t idx) { return *ticks[idx]; }
    const Tick& operator[] (std::size_t idx) const { return *ticks[idx]; }

    typedef typename std::vector<std::unique_ptr<Tick>>::iterator iterator;
    typedef typename std::vector<std::unique_ptr<Tick>>::const_iterator const_iterator;

    //==============================================================================
    /** Returns a pointer to the first element in the array.
     This method is provided for compatibility with standard C++ iteration mechanisms.
     */
    inline iterator begin() noexcept
    {
        return ticks.begin();
    }

    /** Returns a pointer to the first element in the array.
     This method is provided for compatibility with standard C++ iteration mechanisms.
     */
    inline const_iterator begin() const noexcept
    {
        return ticks.begin();
    }

    /** Returns a pointer to the element which follows the last element in the array.
     This method is provided for compatibility with standard C++ iteration mechanisms.
     */
    inline iterator end() noexcept
    {
        return ticks.end();
    }

    /** Returns a pointer to the element which follows the last element in the array.
     This method is provided for compatibility with standard C++ iteration mechanisms.
     */
    inline const_iterator end() const noexcept
    {
        return ticks.end();
    }

    /** Returns a pointer to the first element in the array.
     This method is provided for compatibility with the standard C++ containers.
     */
    inline iterator data() noexcept
    {
        return begin();
    }

    /** Returns a pointer to the first element in the array.
     This method is provided for compatibility with the standard C++ containers.
     */
    inline const_iterator data() const noexcept
    {
        return begin();
    }

    void clear();
    size_t getNumOfTicks() const;

    std::mutex inuseLock;

private:
    std::vector<std::unique_ptr<Tick>> ticks;
};
