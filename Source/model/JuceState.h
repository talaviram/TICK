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

    DECLARE_ID (isEdit)
    DECLARE_ID (showEditSamples)
    DECLARE_ID (showPresetsView)
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
        value = Constrainer::constrain (static_cast<float> (other));
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

using BPMValue = ConstrainerWrapper<float, BPMConstrainer>;

struct BeatAssignment
{
    juce::CachedValue<int> tickIdx;
    juce::CachedValue<float> gain;
};

// Contains settings of current click/ticks
class TickSettings : public juce::ValueTree::Listener, public juce::Value::Listener, public juce::ChangeListener
{
public:
    static const auto kMaxBeatAssignments = 64;
    static const auto kMaxTicks = 16;

#define INFO_FILE_NAME "Info.xml"

    // Tick uses existing standards
    // The settings are basically and XML
    // And each sample (tick) is a 32bit/44.1khz mono WAVE
    // with metadata for start/end and name.
    void saveToArchive (juce::OutputStream& streamToWrite, TicksHolder& ticks, const bool discardTransport = false, const bool isPreset = true);

    void loadFromArchive (juce::ZipFile& archive, TicksHolder& ticks, const bool /*isPreset*/);

    TickSettings (TicksHolder& holder);
    TickSettings (const juce::ValueTree& values, TicksHolder& holder);
    ~TickSettings() override;

    void clear();
    void load (const juce::ValueTree& stateToLoad);
    void valueTreePropertyChanged (juce::ValueTree&, const juce::Identifier& vid) override;
    void changeListenerCallback (juce::ChangeBroadcaster*) override;
    // manage meter as text
    void valueChanged (juce::Value& value) override;

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

    std::map<juce::Identifier, juce::Value> view;
    std::map<juce::Identifier, juce::Value> transport;
    juce::Value meterAsText;

    juce::UndoManager undoManager;
    juce::ValueTree state;

    std::atomic_bool isDirty { false };

private:
    juce::String getMeterAsText();
    void setCachedValues();
    TicksHolder& ticksHolder;
};
