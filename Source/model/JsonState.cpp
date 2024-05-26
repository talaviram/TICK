#include "JsonState.h"
#include "utils/UtilityFunctions.h"

using namespace juce;

static DynamicObject::Ptr valueTreeToJsonObj (const ValueTree& tree)
{
    DynamicObject::Ptr result { new DynamicObject };
    for (int i = 0; i < tree.getNumProperties(); ++i)
    {
        const auto name = tree.getPropertyName (i);
        result->setProperty (name, tree.getProperty (name));
    }
    for (int i = 0; i < tree.getNumChildren(); ++i)
    {
        const auto child = tree.getChild (i);
        result->setProperty (child.getType(), var ((valueTreeToJsonObj (child)).get()));
    }
    return result;
}

String JsonState::valueTreeToJsonString (const ValueTree& tree, const int numDecimalPlaces, const int indent)
{
    MemoryOutputStream stream;
    valueTreeToJsonObj (tree)->writeAsJSON (stream, indent, false, numDecimalPlaces);
    return stream.getMemoryBlock().toString();
}

static ValueTree valueTreeFromJsonObj (DynamicObject* src, Identifier ident)
{
    ValueTree result (ident);
    for (auto& p : src->getProperties())
    {
        if (auto* obj = p.value.getDynamicObject(); obj != nullptr)
            result.appendChild (valueTreeFromJsonObj (obj, p.name), nullptr);
        else
            result.setProperty (p.name, p.value, nullptr);
    }
    return result;
}

ValueTree JsonState::valueTreeFromJsonString (const String& src, Identifier ident)
{
    auto parsed = JSON::parse (src);
    auto dynObj = parsed.getDynamicObject();
    if (dynObj == nullptr)
        return {};
    return valueTreeFromJsonObj (dynObj, ident);
}

using JsonMap = std::map<juce::Identifier, juce::var>;

JsonState::JsonState (TickAudioProcessor& p) : processor (p) {}

juce::String JsonState::transport()
{
    const auto tc = TickUtils::generateTimecodeDisplay (processor.playheadPosition_);
    const auto& state = processor.getState();
    const auto& transport = state.transport;
    juce::ValueTree map = { IDs::TRANSPORT, { { IDs::useHostTransport, state.useHostTransport.get() ? "host" : "internal" }, { IDs::isPlaying, transport.isPlaying.get() }, { IDs::bpm, transport.bpm.get() }, { IDs::numerator, transport.numerator.get() }, { IDs::denumerator, transport.denumerator.get() }, { IDs::preCount, transport.preCount.get() }, { "beat", processor.getBeat() }, { "subbeat", processor.getCurrentBeatPos() }, { "timecode", tc } } };
    return valueTreeToJsonString (map);
}
