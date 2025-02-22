
#include "TicksHolder.h"

TicksHolder::TicksHolder()
{
    formatManager.registerBasicFormats();
}
void TicksHolder::addTicks (std::vector<std::unique_ptr<Tick>> ticksToAdd, double samplerate)
{
    ticks.addTicks (std::move (ticksToAdd), samplerate, true);
    sendChangeMessage();
}

void TicksHolder::addTick (std::unique_ptr<Tick>&& tickToAdd)
{
    tickToAdd->setSampleRate (sampleRate);
    ticks.addTick (std::move (tickToAdd));
    sendChangeMessage();
}
void TicksHolder::removeTick (int idx)
{
    ticks.removeTick (idx);
    sendChangeMessage();
}
void TicksHolder::replaceTick (int idx, std::unique_ptr<Tick>&& newTick)
{
    ticks.replaceTick (idx, std::move (newTick));
    sendChangeMessage();
}
void TicksHolder::clear()
{
    ticks.clear();
    sendChangeMessage();
}

void TicksHolder::setSampleRate (double newSampleRate)
{
    if (juce::approximatelyEqual (sampleRate, newSampleRate))
        return;

    sampleRate = newSampleRate;
    for (auto& tick : ticks)
        tick->setSampleRate (sampleRate);
}

Tick* TicksHolder::convertAudioToTick (const juce::String& tickName, juce::AudioFormatReader* reader)
{
    if (reader != nullptr)
    {
        const auto numSamples = (int) std::ceil (std::min<double> (static_cast<double> (reader->lengthInSamples), reader->sampleRate * 2.0));
        juce::AudioSampleBuffer sampleToLoad ((int) reader->numChannels, numSamples);
        reader->read (&sampleToLoad, 0, numSamples, 0, true, true);
        auto tick = new Tick (tickName.toRawUTF8(), sampleToLoad.getArrayOfReadPointers(), numSamples, (int) reader->numChannels, reader->sampleRate);
        return tick;
    }
    return nullptr;
}

Tick* TicksHolder::importAudioFile (juce::File file)
{
    jassert (file.exists());
    std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file));
    return convertAudioToTick (file.getFileNameWithoutExtension(), reader.get());
}

Tick* TicksHolder::importURL (juce::URL url)
{
#if JUCE_ANDROID
    auto androidDocument = juce::AndroidDocument::fromDocument (url);
    auto stream = androidDocument.createInputStream();
#else
    std::unique_ptr<juce::InputStream> stream (juce::URLInputSource (url).createInputStream());
#endif
    jassert (stream != nullptr);
    auto name = url.getFileName().upToLastOccurrenceOf (".", false, false);
    std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (std::move (stream)));
    return convertAudioToTick (name, reader.get());
}

Tick* TicksHolder::importAudioStream (const juce::String& name, std::unique_ptr<juce::InputStream> stream)
{
    std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (std::move (stream)));
    return convertAudioToTick (name, reader.get());
}

spin_lock& TicksHolder::getLock() { return ticks.inuseLock; }

size_t TicksHolder::getNumOfTicks() { return ticks.getNumOfTicks(); }

inline Ticks::iterator TicksHolder::begin() noexcept
{
    return ticks.begin();
}
inline Ticks::const_iterator TicksHolder::begin() const noexcept
{
    return ticks.begin();
}
inline Ticks::iterator TicksHolder::end() noexcept
{
    return ticks.end();
}
inline Ticks::const_iterator TicksHolder::end() const noexcept
{
    return ticks.end();
}
inline Ticks::iterator TicksHolder::data() noexcept
{
    return begin();
}
inline Ticks::const_iterator TicksHolder::data() const noexcept
{
    return begin();
}

juce::MemoryInputStream* TicksHolder::getTickAsWave (int index)
{
    const auto idx = static_cast<size_t> (index);
    auto* wav = new juce::MemoryOutputStream();
    juce::StringPairArray meta;
    // our non-standard meta data
    meta.set (juce::WavAudioFormat::riffInfoTitle, juce::String (ticks[idx].getName()));
    meta.set (juce::WavAudioFormat::riffInfoStartTimecode, juce::String (ticks[idx].getStartInSec()));
    meta.set (juce::WavAudioFormat::riffInfoEndTimecode, juce::String (ticks[idx].getEndInSec()));
    meta.set (juce::WavAudioFormat::riffInfoComment, juce::String (ticks[idx].getGain()));
    std::unique_ptr<juce::AudioFormatWriter> writer;
    writer.reset (juce::WavAudioFormat().createWriterFor (wav, BASE_SAMPLERATE, 1, 32, meta, 0));
    float* src[1];
    src[0] = ticks[idx].getTickSource();
    const auto result = writer->write ((const int**) src, ticks[idx].getSourceLengthInSamples());
    jassertquiet (result);
    writer->flush();
    return new juce::MemoryInputStream (wav->getData(), wav->getDataSize(), true);
}

std::unique_ptr<Tick> TicksHolder::createTickFromWave (juce::InputStream* input)
{
    std::unique_ptr<juce::AudioFormatReader> reader (juce::WavAudioFormat().createReaderFor (input, true));
    jassert (reader->lengthInSamples > 0);
    juce::AudioBuffer<float> wavData;
    wavData.setSize (1, (int) reader->lengthInSamples);
    reader->read (&wavData, 0, (int) reader->lengthInSamples, 0, true, false);
    auto tick = std::make_unique<Tick> (reader->metadataValues.getValue (juce::WavAudioFormat::riffInfoTitle, "").toRawUTF8(), wavData.getArrayOfReadPointers(), wavData.getNumSamples(), 1, reader->sampleRate);
    const auto start = reader->metadataValues.getValue (juce::WavAudioFormat::riffInfoStartTimecode, "0.0").getDoubleValue();
    const auto end = reader->metadataValues.getValue (juce::WavAudioFormat::riffInfoEndTimecode, "0.0").getDoubleValue();
    const auto gain = (float) reader->metadataValues.getValue (juce::WavAudioFormat::riffInfoComment, "1.0").getDoubleValue();
    tick->setRange (start, end);
    tick->setGain (gain);
    return tick;
}
