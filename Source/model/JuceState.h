/*
  ==============================================================================

    JuceState.h
    Utility wrapper to use model with juce::ValueTree concepts
    Created: 21 Sep 2019 2:51:34pm
    Author:  Tal Aviram

  ==============================================================================
*/

#pragma once

#include "TickModel.h"
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "TicksHolder.h"

namespace IDs
{
#define DECLARE_ID(name) const juce::Identifier name (#name);

    DECLARE_ID (TICK_SETTINGS)

    DECLARE_ID (presetName)
    DECLARE_ID (uuid)

    DECLARE_ID (viewSize)

    DECLARE_ID (useHostTransport)
    DECLARE_ID (filterCutoff)
    DECLARE_ID (masterGain)
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
    DECLARE_ID (preCount)

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

    bool operator== (const ConstrainerWrapper& other) const noexcept { return juce::approximatelyEqual (value, other.value); }
    bool operator!= (const ConstrainerWrapper& other) const noexcept { return ! juce::approximatelyEqual (value, other.value); }

    operator juce::var() const noexcept
    {
        const auto current = (double) Constrainer::constrain (value);
        const auto rounded = std::round (current);

        return juce::String (value, juce::approximatelyEqual (rounded, current) ? 0 : 2);
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

    ~Transport() override
    {
        meterAsText.removeListener (this);
    }

    juce::CachedValue<int> numerator;
    juce::CachedValue<int> denumerator;
    juce::CachedValue<ConstrainerWrapper<float, BPMConstrainer>> bpm;
    juce::CachedValue<bool> isPlaying;
    juce::CachedValue<int> preCount;

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
            stateToStore.removeProperty (IDs::masterGain, nullptr);
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
        for (size_t i = 0; i < ticks.getNumOfTicks(); i++)
        {
            builder.addEntry (ticks.getTickAsWave ((int) i), 5, juce::String (i) + ".wav", juce::Time::getCurrentTime());
        }
        // TODO: make this async?
        double progress = 0.0;
        builder.writeToStream (streamToWrite, &progress);
        while (progress < 1.0)
            ;
        streamToWrite.flush();
    }

    void loadFromArchive (juce::ZipFile& archive, TicksHolder& ticks, const bool /*isPreset*/)
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

    ~TickSettings() override
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
        masterGain.setValue (stateToLoad.getProperty (IDs::masterGain, masterGain.getDefault()), nullptr);
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
                transport.preCount.setValue (child.getProperty (IDs::preCount), nullptr);
            }
            if (child.getType() == IDs::BEAT)
            {
                auto& assignment = beatAssignments[static_cast<int> (child.getProperty (IDs::index))];
                assignment.tickIdx.setValue (child.getProperty (IDs::tickId), &undoManager);
                assignment.gain.setValue (child.getProperty (IDs::gain), &undoManager);
            }
        }
    }

    void valueTreePropertyChanged (juce::ValueTree&, const juce::Identifier& vid) override
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
    juce::CachedValue<float> masterGain;
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
            masterGain.referTo (state, IDs::masterGain, nullptr, 0.0f);
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
            transportTree.setProperty (IDs::preCount, 0, nullptr);
            transport.meterAsText.setValue ("4/4");
            state.appendChild (transportTree, nullptr);
            transport.isPlaying.referTo (transportTree, IDs::isPlaying, nullptr);
            transport.numerator.referTo (transportTree, IDs::numerator, nullptr);
            transport.denumerator.referTo (transportTree, IDs::denumerator, nullptr);
            transport.bpm.referTo (transportTree, IDs::bpm, nullptr);
            transport.preCount.referTo (transportTree, IDs::preCount, nullptr);
        }
        jassert (state.isValid());
    }
    TicksHolder& ticksHolder;
};
