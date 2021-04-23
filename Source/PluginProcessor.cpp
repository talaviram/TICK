/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include "utils/UtilityFunctions.h"

AudioProcessor::BusesProperties TickAudioProcessor::getDefaultLayout()
{
    // workaround to Ableton Live 10
    if (PluginHostType::getPluginLoadedAs() == AudioProcessor::wrapperType_VST3)
        return BusesProperties()
            .withInput ("Input", AudioChannelSet::stereo(), true)
            .withOutput ("Output", AudioChannelSet::stereo(), true);

    return BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput ("Input", AudioChannelSet::stereo(), true)
#endif
        .withOutput ("Output", AudioChannelSet::stereo(), true)
#endif
        ;
}

//==============================================================================
TickAudioProcessor::TickAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor (getDefaultLayout())
#endif
      ,
      settings (ticks),
      parameters (*this, nullptr, Identifier (JucePlugin_Name),
                  {
                      std::make_unique<AudioParameterFloat> ("multiplier", // parameter ID
                                                             "Tick Multiplier", // parameter name
                                                             NormalisableRange<float> (0.25f, 4.0f, 0.25),
                                                             1.0f), // default value
                      std::make_unique<AudioParameterFloat> (IDs::filterCutoff.toString(), // parameter ID
                                                             "Filter Cutoff", // parameter name
                                                             TickUtils::makeLogarithmicRange<float> (100.0, 20000.0f),
                                                             20000.0f,
                                                             "Hz",
                                                             AudioProcessorParameter::genericParameter,
                                                             [=] (int val, int maxLen) {
                                                                 return String (roundToInt (val)) + "Hz";
                                                             }) // default value
                  })
{
    // init samples reading
    ticks.clear();

    filterCutoff = parameters.getRawParameterValue (IDs::filterCutoff.toString());
    tickMultiplier = parameters.getRawParameterValue ("multiplier");

    // load default preset
    setStateInformation (BinaryData::factory_default_preset, BinaryData::factory_default_presetSize);

    settings.useHostTransport.setValue (wrapperType != WrapperType::wrapperType_Standalone, nullptr);
    lastKnownPosition_.resetToDefault();
    settings.isDirty = false;
}

TickAudioProcessor::~TickAudioProcessor()
{
    tickState.clear();
    ticks.clear();
}

//==============================================================================
const String TickAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TickAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool TickAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool TickAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double TickAudioProcessor::getTailLengthSeconds() const
{
    return 2.0;
}

int TickAudioProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
        // so this should be at least 1, even if you're not really implementing programs.
}

int TickAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TickAudioProcessor::setCurrentProgram (int index)
{
}

const String TickAudioProcessor::getProgramName (int index)
{
    return {};
}

void TickAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void TickAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    getState().samplerate = sampleRate;
    ticks.setSampleRate (sampleRate);
    tickState.clear();
}

void TickAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TickAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // standalone can assert due to different layouts!
    if (wrapperType == wrapperType_Standalone)
        return true;

#if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

        // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

bool TickAudioProcessor::isHostSyncSupported()
{
    return wrapperType != AudioProcessor::wrapperType_Standalone;
}

void TickAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    //    if (wrapperType != wrapperType_Standalone)
    //        lastKnownPosition_.resetToDefault();

    buffer.clear();

    // standalone mode
    if (! isHostSyncSupported() || ! getState().useHostTransport.get())
    {
        lastKnownPosition_.isPlaying = settings.transport.isPlaying.get();
        lastKnownPosition_.timeSigNumerator = settings.transport.numerator.get();
        lastKnownPosition_.timeSigDenominator = settings.transport.denumerator.get();
        lastKnownPosition_.bpm = settings.transport.bpm.get();
        if (lastKnownPosition_.isPlaying && ! tickState.isClear)
        {
            const double bufInSecs = buffer.getNumSamples() / getSampleRate();
            const double iqps = lastKnownPosition_.bpm / 60.0; // quarter-per-second
            lastKnownPosition_.ppqPosition += iqps * bufInSecs;
        }
        else
        {
            lastKnownPosition_.ppqPosition = 0.0;
            tickState.clear();
        }
    }
    else if (getPlayHead())
    {
        getPlayHead()->getCurrentPosition (lastKnownPosition_);
    }

    // setValue only triggers if value is different
    settings.transport.bpm.setValue (lastKnownPosition_.bpm, nullptr);
    settings.transport.numerator.setValue (lastKnownPosition_.timeSigNumerator, nullptr);
    settings.transport.denumerator.setValue (lastKnownPosition_.timeSigDenominator, nullptr);

    if (lastKnownPosition_.isPlaying)
    {
        std::unique_lock<std::mutex> lock (ticks.getLock(), std::try_to_lock);
        if (! lock.owns_lock())
        {
            tickState.clear();
            return;
        }

        // calculate where tick starts in samples...
        const auto pos = lastKnownPosition_.ppqPosition;
        const auto bps = lastKnownPosition_.bpm / 60.0;
        const auto bpSmp = getSampleRate() / bps;
        const auto ttq = (4.0 / lastKnownPosition_.timeSigDenominator); // tick to quarter
        const auto tickAt = ttq / tickMultiplier->load(); // tick every (1.0 = 1/4, 0.5 = 1/8, ...)
        const auto tickLengthInSamples = tickAt * bpSmp;

        const auto ppqFromBufStart = fmod (pos, tickAt);
        const double ppqOffset = tickAt - ppqFromBufStart;
        const auto bufStartInSecs = lastKnownPosition_.timeInSeconds;
        const auto bufEndInSecs = bufStartInSecs + (buffer.getNumSamples() / getSampleRate());
        ppqEndVal = pos + ((bufEndInSecs - bufStartInSecs) * bps);
        const auto bufLengthInPPQ = bps * (buffer.getNumSamples() / getSampleRate());

        auto ppqToBufEnd = bufLengthInPPQ;
        auto ppqPosInBuf = ppqOffset;
        auto currentSampleToTick = 0;

        // reset tick state
        tickState.tickStartPosition = 0;

        if (ppqFromBufStart == 0)
        {
            ppqPosInBuf = 0.0;
        }
        int i = 0;

        if (ticks.getNumOfTicks() == 0)
        {
            tickState.clear();
            return;
        }

        while (ppqToBufEnd > ppqPosInBuf)
        {
            i++;
            jassert (ppqToBufEnd >= ppqPosInBuf);
            // add tick(s) to current buffer
            currentSampleToTick = roundToInt (ppqPosInBuf * bpSmp);
            ppqPosInBuf += tickAt; // next sample
            tickState.beat = floor (fmod ((pos + ppqPosInBuf) / ttq, lastKnownPosition_.timeSigNumerator)); // + 1;
            if (tickState.beat == 0)
                tickState.beat = lastKnownPosition_.timeSigNumerator;
            const auto& beatAssign = settings.beatAssignments[jlimit (1, TickSettings::kMaxBeatAssignments, tickState.beat) - 1];
            const auto tickIdx = jlimit (0, jmax ((int) ticks.getNumOfTicks() - 1, 0), beatAssign.tickIdx.get());
            tickState.refer[0] = ticks[tickIdx].getTickAudioBuffer();
            tickState.sample.makeCopyOf (AudioSampleBuffer (tickState.refer, 1, ticks[tickIdx].getLengthInSamples()));
            tickState.sample.applyGain (ticks[tickIdx].getGain());
            // LPF is per beat, less responsive but more optimized
            lpfFilter.setCoefficients (IIRCoefficients::makeLowPass (getSampleRate(), filterCutoff->load()));
            lpfFilter.processSamples (tickState.sample.getWritePointer (0), tickState.sample.getNumSamples());
            tickState.beatGain = beatAssign.gain.get();
            tickState.addTickSample (buffer, currentSampleToTick, tickLengthInSamples);
        }
        tickState.fillTickSample (buffer);
    }
}

//==============================================================================
bool TickAudioProcessor::hasEditor() const
{
    return true;
}

AudioProcessorEditor* TickAudioProcessor::createEditor()
{
    return new TickAudioProcessorEditor (*this);
}

//==============================================================================
void TickAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // save
    MemoryOutputStream writeStream (destData, false);
    settings.saveToArchive (writeStream, ticks);
}

void TickAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto* stream = new MemoryInputStream (data, sizeInBytes, false);
    ZipFile archive (stream, true);
    settings.loadFromArchive (archive, ticks);
}

double TickAudioProcessor::getCurrentBeatPos()
{
    const auto ttq = (4.0 / lastKnownPosition_.timeSigDenominator); // tick to quarter
    auto subDiv = fmod (lastKnownPosition_.ppqPosition, ttq) / ttq;
    return tickState.beat + subDiv;
}

void TickAudioProcessor::TickState::addTickSample (AudioBuffer<float>& bufferToFill, int startPos, int length)
{
    isClear = false;
    currentSample = 0;
    tickStartPosition = startPos;
    tickLengthInSamples = length;
    fillTickSample (bufferToFill);
    tickStartPosition = -1;
}

void TickAudioProcessor::TickState::fillTickSample (AudioBuffer<float>& bufferToFill)
{
    if (tickStartPosition < 0)
        return; // fillTick was consumed

    if (currentSample < 0)
        return; // not active tick.

    auto constrainedLength = jmin (tickLengthInSamples - currentSample, sample.getNumSamples() - currentSample, bufferToFill.getNumSamples() - tickStartPosition);
    const auto maxSampleChannelIndex = sample.getNumChannels() - 1;
    for (auto ch = 0; ch < bufferToFill.getNumChannels(); ch++)
    {
        bufferToFill.copyFrom (ch, tickStartPosition, sample, jlimit (0, maxSampleChannelIndex, ch), currentSample, constrainedLength);
    }

    bufferToFill.applyGain (beatGain);

    currentSample += constrainedLength;
    if (currentSample == sample.getNumSamples())
    {
        currentSample = -1; // mark as not valid.
    }
}

void TickAudioProcessor::TickState::clear()
{
    isClear = true;
    currentSample = -1;
    tickLengthInSamples = tickStartPosition = beat = 0;
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TickAudioProcessor();
}
