#pragma once

#include "TickModel.h"
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_events/juce_events.h>

// wrapping Ticks with juce::ChangeBroadcaster for notifications
struct TicksHolder : public juce::ChangeBroadcaster
{
public:
    TicksHolder();
    void addTicks (std::vector<std::unique_ptr<Tick>> ticksToAdd, double samplerate);
    void addTick (std::unique_ptr<Tick>&& tickToAdd);
    void removeTick (int idx);
    void replaceTick (int idx, std::unique_ptr<Tick>&& newTick);
    void clear();
    void setSampleRate (double newSampleRate);

private:
    Tick* convertAudioToTick (const juce::String& tickName, juce::AudioFormatReader* reader);

public:
    Tick* importAudioFile (juce::File file);
    Tick* importURL (juce::URL url);
    Tick* importAudioStream (const juce::String& name, std::unique_ptr<juce::InputStream> stream);

    std::mutex& getLock();

    size_t getNumOfTicks();

    typedef typename std::vector<std::unique_ptr<Tick>>::iterator iterator;
    typedef typename std::vector<std::unique_ptr<Tick>>::const_iterator const_iterator;

    Tick& operator[] (std::size_t idx) { return ticks[idx]; }
    const Tick& operator[] (std::size_t idx) const { return ticks[idx]; }

    //==============================================================================
    /** Returns a pointer to the first element in the array.
     This method is provided for compatibility with standard C++ iteration mechanisms.
     */
    Ticks::iterator begin() noexcept;
    /** Returns a pointer to the first element in the array.
     This method is provided for compatibility with standard C++ iteration mechanisms.
     */
    const_iterator begin() const noexcept;
    /** Returns a pointer to the element which follows the last element in the array.
     This method is provided for compatibility with standard C++ iteration mechanisms.
     */
    Ticks::iterator end() noexcept;
    /** Returns a pointer to the element which follows the last element in the array.
     This method is provided for compatibility with standard C++ iteration mechanisms.
     */
    const_iterator end() const noexcept;
    /** Returns a pointer to the first element in the array.
     This method is provided for compatibility with the standard C++ containers.
     */
    Ticks::iterator data() noexcept;
    /** Returns a pointer to the first element in the array.
     This method is provided for compatibility with the standard C++ containers.
     */
    const_iterator data() const noexcept;
    juce::MemoryInputStream* getTickAsWave (int index);
    // this expects specific WAVE with the proper metadata.
    static std::unique_ptr<Tick> createTickFromWave (juce::InputStream* input);

private:
    double sampleRate { BASE_SAMPLERATE };
    juce::AudioFormatManager formatManager;
    Ticks ticks;
};
