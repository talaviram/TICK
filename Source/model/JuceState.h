/*
  ==============================================================================

    JuceState.h
    Utility wrapper to use model with juce::ValueTree concepts
    Created: 21 Sep 2019 2:51:34pm
    Author:  Tal Aviram

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"
#include "TickModel.h"

namespace IDs
{
#define DECLARE_ID(name) const juce::Identifier name (#name);

    DECLARE_ID (TICK_SETTINGS)

    DECLARE_ID (presetName)
    DECLARE_ID (uuid)

    DECLARE_ID (viewSize)

    DECLARE_ID (useHostTransport)
    DECLARE_ID (filterCutoff)
    DECLARE_ID (showWaveform)
    DECLARE_ID (showBeatNumber)
    DECLARE_ID (isVertical)

    DECLARE_ID (BEAT)
    DECLARE_ID (index)
    DECLARE_ID (tickId)
    DECLARE_ID (gain)

    DECLARE_ID (TRANSPORT)
    DECLARE_ID (numerator)
    DECLARE_ID (denumerator)
    DECLARE_ID (bpm)
    DECLARE_ID (isPlaying)

    DECLARE_ID (numOfTicks)

#undef DECLARE_ID
} // namespace IDs

constexpr int defaultWidth = 510;
constexpr int defaultHeight = 300;
typedef juce::Point<int> ViewDiemensions;

template <>
struct juce::VariantConverter<ViewDiemensions>
{
    static ViewDiemensions fromVar (const juce::var& v)
    {
        const auto asStr = v.toString().trim();
        const auto sep = asStr.indexOf (",");
        if (! asStr.containsAnyOf ("0123456789") || (sep <= 0 || (sep > 0 && asStr.lastIndexOf (",") != sep)))
            return { defaultWidth, defaultHeight };

        return { std::stoi (asStr.substring (0, sep).toStdString()), std::stoi (asStr.substring (sep + 1).toStdString()) };
    }

    static juce::var toVar (const ViewDiemensions& v)
    {
        return juce::String::formatted ("%d,%d", v.getX(), v.getY());
    }
};

template <typename Type, typename Constrainer>
struct ConstrainerWrapper
{
    ConstrainerWrapper() = default;

    template <typename OtherType>
    ConstrainerWrapper (const OtherType& other)
    {
        value = Constrainer::constrain (other);
    }

    ConstrainerWrapper& operator= (const ConstrainerWrapper& other) noexcept
    {
        value = Constrainer::constrain (other.value);
        return *this;
    }

    bool operator== (const ConstrainerWrapper& other) const noexcept { return value == other.value; }
    bool operator!= (const ConstrainerWrapper& other) const noexcept { return value != other.value; }

    operator juce::var() const noexcept
    {
        const auto current = Constrainer::constrain (value);
        const auto rounded = juce::roundToInt (current);
        return juce::String (value, rounded == current ? 0 : 2);
    }
    operator Type() const noexcept { return Constrainer::constrain (value); }

    Type value = Type();
    bool isRounded { false };
};

struct BPMConstrainer
{
    static constexpr auto minBPM = 1.0f;
    static constexpr auto maxBPM = 999.0f;

    static float constrain (const float& v)
    {
        return juce::Range<float> (minBPM, maxBPM).clipValue (v);
    }
};

struct BeatAssignment
{
    juce::CachedValue<int> tickIdx;
    juce::CachedValue<float> gain;
};

//==============================================================================

// wrapping Ticks with juce::ChangeBroadcaster for notifications
struct TicksHolder : public juce::ChangeBroadcaster
{
public:
    TicksHolder()
    {
        formatManager.registerBasicFormats();
    }
    void addTicks (std::vector<std::unique_ptr<Tick>> ticksToAdd, double samplerate)
    {
        ticks.addTicks (std::move (ticksToAdd), samplerate, true);
        sendChangeMessage();
    }

    void addTick (std::unique_ptr<Tick>&& tickToAdd)
    {
        tickToAdd->setSampleRate (sampleRate);
        ticks.addTick (std::move (tickToAdd));
        sendChangeMessage();
    }
    void removeTick (int idx)
    {
        ticks.removeTick (idx);
        sendChangeMessage();
    }
    void replaceTick (int idx, std::unique_ptr<Tick>&& newTick)
    {
        ticks.replaceTick (idx, std::move (newTick));
        sendChangeMessage();
    }
    void clear()
    {
        ticks.clear();
        sendChangeMessage();
    }

    void setSampleRate (double newSampleRate)
    {
        if (sampleRate == newSampleRate)
            return;

        sampleRate = newSampleRate;
        for (auto& tick : ticks)
            tick->setSampleRate (sampleRate);
    }

private:
    Tick* convertAudioToTick (const juce::String& tickName, juce::AudioFormatReader* reader)
    {
        if (reader != nullptr)
        {
            const auto numSamples = std::min<double> (static_cast<double> (reader->lengthInSamples), reader->sampleRate * 2.0);
            juce::AudioSampleBuffer sampleToLoad (reader->numChannels, numSamples);
            reader->read (&sampleToLoad, 0, numSamples, 0, true, true);
            auto tick = new Tick (tickName.toRawUTF8(), sampleToLoad.getArrayOfReadPointers(), numSamples, reader->numChannels, reader->sampleRate);
            return tick;
        }
        return nullptr;
    }

public:
    Tick* importAudioFile (juce::File file)
    {
        jassert (file.exists());
        std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file));
        return convertAudioToTick (file.getFileNameWithoutExtension(), reader.get());
    }

    Tick* importURL (juce::URL url)
    {
        std::unique_ptr<juce::InputStream> stream (juce::URLInputSource (url).createInputStream());
        jassert (stream != nullptr);
        std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (std::move (stream)));
        return convertAudioToTick (url.getLocalFile().getFileNameWithoutExtension(), reader.get());
    }

    Tick* importAudioStream (const juce::String& name, std::unique_ptr<juce::InputStream> stream)
    {
        std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (std::move (stream)));
        return convertAudioToTick (name, reader.get());
    }

    spin_lock& getLock() { return ticks.inuseLock; }

    size_t getNumOfTicks() { return ticks.getNumOfTicks(); }

    typedef typename std::vector<std::unique_ptr<Tick>>::iterator iterator;
    typedef typename std::vector<std::unique_ptr<Tick>>::const_iterator const_iterator;

    Tick& operator[] (std::size_t idx) { return ticks[idx]; }
    const Tick& operator[] (std::size_t idx) const { return ticks[idx]; }

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

    juce::MemoryInputStream* getTickAsWave (int idx)
    {
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

    // this expects specific WAVE with the proper metadata.
    static std::unique_ptr<Tick> createTickFromWave (juce::InputStream* input)
    {
        std::unique_ptr<juce::AudioFormatReader> reader (juce::WavAudioFormat().createReaderFor (input, true));
        jassert (reader->lengthInSamples > 0);
        juce::AudioBuffer<float> wavData;
        wavData.setSize (1, (int) reader->lengthInSamples);
        reader->read (&wavData, 0, (int) reader->lengthInSamples, 0, true, false);
        auto tick = std::make_unique<Tick> (reader->metadataValues.getValue (juce::WavAudioFormat::riffInfoTitle, "").toRawUTF8(), wavData.getArrayOfReadPointers(), wavData.getNumSamples(), 1, reader->sampleRate);
        auto start = reader->metadataValues.getValue (juce::WavAudioFormat::riffInfoStartTimecode, "0.0").getDoubleValue();
        auto end = reader->metadataValues.getValue (juce::WavAudioFormat::riffInfoEndTimecode, "0.0").getDoubleValue();
        auto gain = reader->metadataValues.getValue (juce::WavAudioFormat::riffInfoComment, "1.0").getDoubleValue();
        tick->setRange (start, end);
        tick->setGain (gain);
        return tick;
    }

private:
    double sampleRate { BASE_SAMPLERATE };
    juce::AudioFormatManager formatManager;
    Ticks ticks;
};

//==============================================================================

struct View
{
    juce::Value windowSize; // desktop only
    juce::Value isEdit;
    juce::Value showEditSamples;
    juce::Value showPresetsView;
};

// Transport State
// can be pulled from host or be internal
struct Transport : public juce::Value::Listener
{
    Transport()
    {
        meterAsText.addListener (this);
    }

    ~Transport()
    {
        meterAsText.removeListener (this);
    }

    juce::CachedValue<int> numerator;
    juce::CachedValue<int> denumerator;
    juce::CachedValue<ConstrainerWrapper<float, BPMConstrainer>> bpm;
    juce::CachedValue<bool> isPlaying;

    juce::Value meterAsText;
    // manage meter as text
    void valueChanged (juce::Value& value) override
    {
        juce::String meterToParse = value.getValue();
        DBG ("new unparsed: " << meterToParse);
        auto num = meterToParse.upToFirstOccurrenceOf ("/", false, true).trim().getIntValue();
        auto denum = meterToParse.fromFirstOccurrenceOf ("/", false, true).trim().getIntValue();
        if (num > 0 && denum > 0)
        {
            DBG ("new val -> " << juce::String (num) << " / " << juce::String (denum));
            numerator.setValue (juce::jlimit (1, /* TODO: refactor cpp/h and use TickSettings::kMaxBeatAssignments */ 64, num), nullptr);
            denumerator.setValue (juce::jlimit (1, 128, denum), nullptr);
        }
        else
        {
            meterAsText.setValue (juce::String (numerator.get()) + "/" + juce::String (denumerator.get()));
        }
    }
};

// Contains settings of current click/ticks
class TickSettings : public juce::ValueTree::Listener, public juce::ChangeListener
{
public:
    static const auto kMaxBeatAssignments = 64;
    static const auto kMaxTicks = 16;

#define INFO_FILE_NAME "Info.xml"

    // Tick uses existing standards
    // The settings are basically and XML
    // And each sample (tick) is a 32bit/44.1khz mono WAVE
    // with metadata for start/end and name.
    void saveToArchive (juce::OutputStream& streamToWrite, TicksHolder& ticks, const bool discardTransport = false, const bool isPreset = true)
    {
        auto stateToStore = state.createCopy();
        if (discardTransport)
        {
            stateToStore.removeChild (stateToStore.getChildWithName (IDs::TRANSPORT), nullptr);
        }
        else
        {
            stateToStore.getChildWithName (IDs::TRANSPORT).setProperty (IDs::isPlaying, false, nullptr);
        }
        const auto uuid = juce::Uuid();
        if (isDirty || presetHash.isEmpty())
            stateToStore.setProperty (IDs::uuid, uuid.toDashedString(), nullptr);
        else
            stateToStore.setProperty (IDs::uuid, presetHash, nullptr);

        // don't store host state in preset.
        if (isPreset)
        {
            stateToStore.removeProperty (IDs::showWaveform, nullptr);
            stateToStore.removeProperty (IDs::showBeatNumber, nullptr);
            stateToStore.removeProperty (IDs::viewSize, nullptr);
            stateToStore.removeProperty (IDs::isVertical, nullptr);
            // we don't really use APVTS so we manually add it
            stateToStore.removeProperty (IDs::filterCutoff, nullptr);
        }

        juce::String stateUuid = stateToStore.getProperty (IDs::uuid);
        jassert (stateUuid.isNotEmpty());

        auto xml = stateToStore.createXml();
        juce::MemoryOutputStream stream;
        DBG (xml->toString());
        xml->writeTo (stream);
        juce::ZipFile::Builder builder;
        auto inputStream = new juce::MemoryInputStream (stream.getData(), stream.getDataSize(), false);
        builder.addEntry (inputStream, 5, INFO_FILE_NAME, juce::Time::getCurrentTime());
        for (auto i = 0; i < ticks.getNumOfTicks(); i++)
        {
            builder.addEntry (ticks.getTickAsWave (i), 5, juce::String (i) + ".wav", juce::Time::getCurrentTime());
        }
        // TODO: make this async?
        double progress = 0.0;
        builder.writeToStream (streamToWrite, &progress);
        while (progress < 1.0)
            ;
        streamToWrite.flush();
    }

    void loadFromArchive (juce::ZipFile& archive, TicksHolder& ticks, const bool isPreset = true)
    {
        if (archive.getNumEntries() == 0)
        {
            jassertfalse;
            return;
        }

        const int xmlIdx = archive.getIndexOfFileName (INFO_FILE_NAME);
        if (xmlIdx == -1 && archive.getEntry (xmlIdx)->uncompressedSize > 512 * 1000)
        {
            jassertfalse;
            return;
        };

        auto data = std::unique_ptr<juce::InputStream> (archive.createStreamForEntry (0));
        const auto stateDataToLoad = juce::ValueTree::fromXml (data->readString());

        const juce::String unverifiedUuid = stateDataToLoad.getProperty (IDs::uuid);
        if (unverifiedUuid.isEmpty())
            state.setProperty (IDs::uuid, juce::Uuid().toDashedString(), nullptr);
        else
            presetHash = unverifiedUuid;

        load (stateDataToLoad);
        // this might happen if it was made by a user with macOS compress that adds additional data!
        jassert (numOfTicks.get() == archive.getNumEntries() - 1);

        std::vector<std::unique_ptr<Tick>> ticksToAdd;
        for (auto i = 1; i <= numOfTicks.get(); i++)
        {
            ticksToAdd.push_back (std::unique_ptr<Tick> (ticks.createTickFromWave (archive.createStreamForEntry (i))));
        }
        ticks.addTicks (std::move (ticksToAdd), samplerate);
        isDirty.store (false);
    }

    TickSettings (TicksHolder& holder) : ticksHolder (holder)
    {
        state = juce::ValueTree (IDs::TICK_SETTINGS);

        // init defaults
        state.setProperty (IDs::presetName, "Empty", nullptr);
        state.setProperty (IDs::numOfTicks, 0, nullptr);
        for (int i = 0; i < kMaxBeatAssignments; i++)
        {
            auto beat = juce::ValueTree (IDs::BEAT);
            beat.setProperty (IDs::index, i, nullptr);
            beat.setProperty (IDs::tickId, 0, nullptr);
            beat.setProperty (IDs::gain, 1.0f, nullptr);
            state.appendChild (beat, nullptr);
        }

        setCachedValues();
#if JUCE_LINUX
        const juce::MessageManagerLock mmLock;
#endif
        state.addListener (this);
        ticksHolder.addChangeListener (this);
    }

    TickSettings (const juce::ValueTree& values, TicksHolder& holder)
        : state (values), ticksHolder (holder)
    {
        setCachedValues();
        state.addListener (this);
    }

    ~TickSettings()
    {
        ticksHolder.removeChangeListener (this);
        state.removeListener (this);
    }

    void clear()
    {
        for (auto i = 0; i < kMaxBeatAssignments; i++)
        {
            presetName.setValue ("Empty", nullptr);
            beatAssignments[i].gain = 1.0f; // oddly reset to default not working?
            beatAssignments[i].tickIdx.resetToDefault();
            ticksHolder.clear();
        }
    }

    void load (const juce::ValueTree& stateToLoad)
    {
        presetHash = stateToLoad.getProperty (IDs::uuid);
        presetName.setValue (stateToLoad.getProperty (IDs::presetName), nullptr);
        numOfTicks.setValue (stateToLoad.getProperty (IDs::numOfTicks), nullptr);
        if (stateToLoad.hasProperty (IDs::showWaveform))
            showWaveform.setValue (stateToLoad.getProperty (IDs::showWaveform, false), nullptr);
        if (stateToLoad.hasProperty (IDs::showBeatNumber))
            showBeatNumber.setValue (stateToLoad.getProperty (IDs::showBeatNumber, false), nullptr);
        if (stateToLoad.hasProperty (IDs::isVertical))
            isVertical.setValue (stateToLoad.getProperty (IDs::isVertical, false), nullptr);
        cutoffFilter.setValue (stateToLoad.getProperty (IDs::filterCutoff, cutoffFilter.getDefault()), nullptr);
        view.windowSize.setValue (stateToLoad.getProperty (IDs::viewSize));

        // only use this if transport data existed
        const bool loadedUseHostState = stateToLoad.getProperty (IDs::useHostTransport);

        for (auto child : stateToLoad)
        {
            if (child.getType() == IDs::TRANSPORT)
            {
                transport.bpm.setValue (child.getProperty (IDs::bpm), nullptr);
                transport.numerator.setValue (child.getProperty (IDs::numerator), nullptr);
                transport.denumerator.setValue (child.getProperty (IDs::denumerator), nullptr);
                transport.meterAsText = getMeterAsText();
                useHostTransport.setValue (loadedUseHostState, nullptr);
            }
            if (child.getType() == IDs::BEAT)
            {
                auto& assignment = beatAssignments[static_cast<int> (child.getProperty (IDs::index))];
                assignment.tickIdx.setValue (child.getProperty (IDs::tickId), &undoManager);
                assignment.gain.setValue (child.getProperty (IDs::gain), &undoManager);
            }
        }
    }

    void valueTreePropertyChanged (juce::ValueTree& vt, const juce::Identifier& vid) override
    {
        if (vid == IDs::isPlaying)
            return;

        isDirty.store (true);
        if (vid == IDs::numerator || vid == IDs::denumerator)
        {
            if (transport.meterAsText.getValue() != getMeterAsText())
            {
                transport.meterAsText.setValue (getMeterAsText());
            }
        }
    }

    void changeListenerCallback (juce::ChangeBroadcaster*) override
    {
        numOfTicks.setValue (static_cast<int> (ticksHolder.getNumOfTicks()), nullptr);
    }

    BeatAssignment beatAssignments[kMaxBeatAssignments];
    juce::CachedValue<juce::String> presetName;

    juce::CachedValue<bool> useHostTransport;
    juce::CachedValue<bool> showWaveform;
    juce::CachedValue<bool> showBeatNumber;
    juce::CachedValue<bool> isVertical;
    juce::CachedValue<float> cutoffFilter;
    juce::CachedValue<int> numOfTicks;
    juce::String presetHash;
    double samplerate { 0 };
    int selectedEdit { -1 };

    View view;
    Transport transport;
    juce::UndoManager undoManager;
    juce::ValueTree state;

    std::atomic_bool isDirty { false };

private:
    juce::String getMeterAsText()
    {
        return juce::String (transport.numerator.get()) + "/" + juce::String (transport.denumerator.get());
    }

    void setCachedValues()
    {
        {
            presetName.referTo (state, IDs::presetName, nullptr, "Empty");
            useHostTransport.referTo (state, IDs::useHostTransport, nullptr, true);
            showWaveform.referTo (state, IDs::showWaveform, nullptr, false);
            showBeatNumber.referTo (state, IDs::showBeatNumber, nullptr, false);
            isVertical.referTo (state, IDs::isVertical, nullptr, false);
            cutoffFilter.referTo (state, IDs::filterCutoff, nullptr, 20000.0f);
            numOfTicks.referTo (state, IDs::numOfTicks, nullptr, 0);
            view.windowSize.referTo (state.getPropertyAsValue (IDs::viewSize, nullptr));
        }

        for (auto child : state)
        {
            if (child.getType() == IDs::BEAT)
            {
                int index = child.getProperty (IDs::index);
                beatAssignments[index].tickIdx.referTo (child, IDs::tickId, &undoManager);
                beatAssignments[index].gain.referTo (child, IDs::gain, &undoManager, 1.0f);
            }
        }

        {
            auto transportTree = juce::ValueTree (IDs::TRANSPORT);
            transportTree.setProperty (IDs::isPlaying, false, nullptr);
            transportTree.setProperty (IDs::numerator, 4, nullptr);
            transportTree.setProperty (IDs::denumerator, 4, nullptr);
            transportTree.setProperty (IDs::bpm, 120, nullptr);
            transport.meterAsText.setValue ("4/4");
            state.appendChild (transportTree, nullptr);
            transport.isPlaying.referTo (transportTree, IDs::isPlaying, nullptr);
            transport.numerator.referTo (transportTree, IDs::numerator, nullptr);
            transport.denumerator.referTo (transportTree, IDs::denumerator, nullptr);
            transport.bpm.referTo (transportTree, IDs::bpm, nullptr);
        }
        jassert (state.isValid());
    }
    TicksHolder& ticksHolder;
};
