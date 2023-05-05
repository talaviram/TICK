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
      parameters (*this, nullptr, Identifier (JucePlugin_Name), { std::make_unique<AudioParameterFloat> (ParameterID (IDs::filterCutoff.toString(), 1), // parameter ID
                                                                                                         "Filter Cutoff", // parameter name
                                                                                                         TickUtils::makeLogarithmicRange<float> (100.0, 20000.0f),
                                                                                                         20000.0f,
                                                                                                         AudioParameterFloatAttributes().withStringFromValueFunction ([] (auto val, auto)
                                                                                                                                                                      { return String (roundToInt (val)) + "Hz"; })
                                                                                                             .withLabel ("Hz")),
                                                                  std::make_unique<AudioParameterFloat> (ParameterID (IDs::masterGain.toString(), 1), // parameter ID
                                                                                                         "Master Gain", // parameter name
                                                                                                         NormalisableRange<float> (-60.0f, 6.0f),
                                                                                                         0.0f,
                                                                                                         AudioParameterFloatAttributes().withStringFromValueFunction ([] (auto val, auto)
                                                                                                                                                                      { return String (roundToInt (val)) + "dB"; })
                                                                                                             .withLabel ("dB")) })
{
    // init samples reading
    ticks.clear();

    filterCutoff = parameters.getRawParameterValue (IDs::filterCutoff.toString());
    masterGain = parameters.getRawParameterValue (IDs::masterGain.toString());

    // load default preset
    setStateInformation (BinaryData::factory_default_preset, BinaryData::factory_default_presetSize);

    settings.useHostTransport.setValue (wrapperType != WrapperType::wrapperType_Standalone, nullptr);
    playheadPosition_ = juce::AudioPlayHead::PositionInfo();
    settings.isDirty = false;
}

TickAudioProcessor::~TickAudioProcessor()
{
    tickState.clear();
    ticks.clear();
}

void TickAudioProcessor::setExternalProps (juce::PropertySet* s)
{
    static_cast<TickAudioProcessorEditor*> (getActiveEditor())->standaloneProps = s;
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

void TickAudioProcessor::setCurrentProgram (int /*index*/)
{
}

const String TickAudioProcessor::getProgramName (int /*index*/)
{
    return {};
}

void TickAudioProcessor::changeProgramName (int /*index*/, const String& /*newName*/)
{
}

//==============================================================================
void TickAudioProcessor::prepareToPlay (double sampleRate, int /*samplesPerBlock*/)
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

void TickAudioProcessor::handlePreCount (const double inputPPQ)
{
    const int preCount = getState().transport.preCount.get();
    if (preCount <= 0 || getState().useHostTransport.get())
        return;
    // auto stop
    const auto ts = playheadPosition_.getTimeSignature().orFallback (AudioPlayHead::TimeSignature ({ 4 / 4 }));
    const auto ttq = (4.0 / ts.denominator); // tick to quarter
    const auto expectedBar = std::floor (inputPPQ / ttq / ts.numerator);
    if (expectedBar == preCount)
        getState().transport.isPlaying.setValue (false, nullptr);
}

void TickAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer&)
{
    ScopedNoDenormals noDenormals;
    //    if (wrapperType != wrapperType_Standalone)
    //        lastKnownPosition_.resetToDefault();

    buffer.clear();

    // standalone mode
    if (! isHostSyncSupported() || ! getState().useHostTransport.get())
    {
#if JUCE_IOS
        AbletonLink::Requests requests;
        if (playheadPosition_.getIsPlaying() != settings.transport.isPlaying.get())
            requests.isPlaying = settings.transport.isPlaying.get();

        if (playheadPosition_.getBpm().hasValue() && *playheadPosition_.getBpm() != settings.transport.bpm.get())
            requests.bpm = settings.transport.bpm.get();
#endif

        playheadPosition_.setIsPlaying (settings.transport.isPlaying.get());
        playheadPosition_.setTimeSignature(AudioPlayHead::TimeSignature ({(int)settings.transport.numerator.get(), (int)settings.transport.denumerator.get()}));
        playheadPosition_.setBpm (settings.transport.bpm.get());
        if (playheadPosition_.getIsPlaying() && ! tickState.isClear)
        {
            const double bufInSecs = buffer.getNumSamples() / getSampleRate();
            const double iqps = playheadPosition_.getBpm().orFallback(120.0f) / 60.0; // quarter-per-second
            playheadPosition_.setPpqPosition (playheadPosition_.getPpqPosition().orFallback(0) + (iqps * bufInSecs));
            playheadPosition_.setTimeInSamples(playheadPosition_.getTimeInSamples().orFallback(0) + buffer.getNumSamples());
            playheadPosition_.setTimeInSeconds (playheadPosition_.getTimeInSeconds().orFallback(0) + bufInSecs);
        }
        else
        {
            playheadPosition_.setPpqPosition({});
            playheadPosition_.setPpqPositionOfLastBarStart({});
            playheadPosition_.setTimeInSamples({});
            playheadPosition_.setTimeInSeconds({});
            tickState.clear();
        }

#if JUCE_IOS
        if (m_link.isLinkConnected())
        {
            m_link.linkPosition (playheadPosition_, requests);
            settings.transport.isPlaying.setValue (playheadPosition_.getIsPlaying(), nullptr);
        }
#endif
    }
    else if (getPlayHead())
    {
        playheadPosition_ = getPlayHead()->getPosition().orFallback(AudioPlayHead::PositionInfo());
    }

    // setValue only triggers if value is different
    if (playheadPosition_.getBpm().hasValue())
        settings.transport.bpm.setValue ((float) *playheadPosition_.getBpm(), nullptr);
    if (playheadPosition_.getTimeSignature().hasValue())
    {
        const auto ts = *playheadPosition_.getTimeSignature();
        settings.transport.numerator.setValue (ts.numerator, nullptr);
        settings.transport.denumerator.setValue (ts.denominator, nullptr);
    }


    if (playheadPosition_.getIsPlaying())
    {
        if (! ticks.getLock().try_lock())
            return;

        const auto ppqPosition = playheadPosition_.getPpqPosition().orFallback(0);
        const auto lastBarStart = playheadPosition_.getPpqPositionOfLastBarStart().orFallback(0);
        const auto bpm = playheadPosition_.getBpm().orFallback(120.0f);
        const auto ts = playheadPosition_.getTimeSignature().orFallback(AudioPlayHead::TimeSignature ({4, 4}));
        
        // calculate where tick starts in samples...
        jassert (lastBarStart == 0 || ppqPosition >= lastBarStart);
        const auto pos = ppqPosition - lastBarStart;
        const auto bps = bpm / 60.0;
        const auto bpSmp = getSampleRate() / bps;
        const auto ttq = (4.0 / ts.denominator); // tick to quarter
        const auto tickAt = ttq / tickMultiplier; // tick every (1.0 = 1/4, 0.5 = 1/8, ...)
        const auto tickLengthInSamples = (int) std::ceil (tickAt * bpSmp);

        const auto ppqFromBufStart = fmod (pos, tickAt);
        const double ppqOffset = tickAt - ppqFromBufStart;
        const auto bufStartInSecs = playheadPosition_.getTimeInSeconds().orFallback(0);
        const auto bufEndInSecs = bufStartInSecs + (buffer.getNumSamples() / getSampleRate());
        ppqEndVal = pos + ((bufEndInSecs - bufStartInSecs) * bps);
        const auto bufLengthInPPQ = bps * (buffer.getNumSamples() / getSampleRate());

        // stop if precount is on and counted enough bars
        handlePreCount (ppqEndVal + bufLengthInPPQ);

        auto ppqToBufEnd = bufLengthInPPQ;
        auto ppqPosInBuf = ppqOffset;
        auto currentSampleToTick = 0;

        // reset tick state
        tickState.tickStartPosition = 0;

        if (ppqFromBufStart == 0)
        {
            ppqPosInBuf = 0.0;
        }

        if (ticks.getNumOfTicks() == 0)
        {
            tickState.clear();
            return;
        }

        while (ppqToBufEnd > ppqPosInBuf)
        {
            jassert (ppqToBufEnd >= ppqPosInBuf);
            // add tick(s) to current buffer
            currentSampleToTick = roundToInt (ppqPosInBuf * bpSmp);
            ppqPosInBuf += tickAt; // next sample
            tickState.beat = juce::roundToInt (floor (fmod ((pos + ppqPosInBuf) / ttq, ts.numerator))); // + 1;
            if (tickState.beat == 0)
                tickState.beat = ts.numerator;
            const auto& beatAssign = settings.beatAssignments[jlimit (1, TickSettings::kMaxBeatAssignments, tickState.beat) - 1];
            const auto tickIdx = (size_t) jlimit (0, jmax ((int) ticks.getNumOfTicks() - 1, 0), beatAssign.tickIdx.get());
            tickState.refer[0] = ticks[tickIdx].getTickAudioBuffer();
            tickState.sample.makeCopyOf (AudioSampleBuffer (tickState.refer, 1, ticks[tickIdx].getLengthInSamples()));
            tickState.sample.applyGain (ticks[tickIdx].getGain());
            // LPF is per beat, less responsive but more optimized
            lpfFilter.setCoefficients (IIRCoefficients::makeLowPass (getSampleRate(), filterCutoff->load()));
            lpfFilter.processSamples (tickState.sample.getWritePointer (0), tickState.sample.getNumSamples());
            if (tickState.currentSample >= 0)
                TickUtils::fadeOut (tickState.sample);
            // hard-clip if needed
            TickUtils::processClip (tickState.sample);
            tickState.beatGain = beatAssign.gain.get();
            tickState.addTickSample (buffer, currentSampleToTick, tickLengthInSamples);
        }
        tickState.fillTickSample (buffer);
        ticks.getLock().unlock();
    }
    buffer.applyGain (Decibels::decibelsToGain (masterGain->load()));
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
    settings.cutoffFilter.setValue (filterCutoff->load(), nullptr);
    settings.masterGain.setValue (masterGain->load(), nullptr);
    settings.saveToArchive (writeStream, ticks, false, false);
}

void TickAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto* stream = new MemoryInputStream (data, (size_t) sizeInBytes, false);
    ZipFile archive (stream, true);
    settings.loadFromArchive (archive, ticks, false);
    auto* cutOff = parameters.getParameter (IDs::filterCutoff);
    cutOff->setValueNotifyingHost (cutOff->convertTo0to1 (settings.cutoffFilter.get()));
    auto* gain = parameters.getParameter (IDs::masterGain);
    gain->setValueNotifyingHost (gain->convertTo0to1 (settings.masterGain.get()));
}

double TickAudioProcessor::getCurrentBeatPos()
{
    const auto ppq = playheadPosition_.getPpqPosition().orFallback(0);
    const auto ts = playheadPosition_.getTimeSignature().orFallback(AudioPlayHead::TimeSignature ({4/4}));
    const auto ttq = (4.0 / ts.denominator); // tick to quarter
    auto subDiv = fmod (ppq, ttq) / ttq;
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
