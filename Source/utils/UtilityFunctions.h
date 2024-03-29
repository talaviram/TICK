/*
  ==============================================================================

    UtilityFunctions.h
    Handy functions
    Created: 3 Jan 2020 5:33:00pm
    Author:  Tal Aviram

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"

using namespace juce;

struct TickUtils
{
    static const char* kPresetExtension;

    static String getPresetHash (InputStream& presetZip);

    static void setDrawableFromSVG (DrawableButton*, Drawable* baseImage);

    // Windows and Android makes it hard embedding folders into build.
    // the zip file that's encoded by Projucer provides a simple way keeping all factory audio samples.
    static std::unique_ptr<ZipFile> getFactorySamples()
    {
        return std::make_unique<ZipFile> (new MemoryInputStream (BinaryData::factory_samples_zip, BinaryData::factory_samples_zipSize, false), true);
    }

#if JUCE_ANDROID
    static bool canUseNewerAndroidFileAPI()
    {
        return android_get_device_api_level() > 28;
    }
#endif

    static bool canImport()
    {
        return
#if JUCE_ANDROID
            juce::RuntimePermissions::isGranted (juce::RuntimePermissions::readExternalStorage);
#else
            true
#endif
        ;
    }

    static bool canExport()
    {
        return
#if JUCE_ANDROID
            ! canUseNewerAndroidFileAPI() ? juce::RuntimePermissions::isGranted (juce::RuntimePermissions::writeExternalStorage) : true;
#else
            true
#endif
        ;
    }

    static bool usePlatformDialog()
    {
        return juce::FileChooser::isPlatformDialogAvailable();
    }

    static void processClip (juce::AudioBuffer<float>&);

    static void fadeOut (juce::AudioBuffer<float>&);

    // bluntly taken from juce demo. might need to add branches for different hosts/PPQ resolution
    // if possible
    static String generateTimecodeDisplay (const AudioPlayHead::PositionInfo& pos)
    {
        const auto quarterNotes = pos.getPpqPosition().orFallback(0.0);
        const auto ts = pos.getTimeSignature().orFallback (AudioPlayHead::TimeSignature({0,0}));
        const auto maybeBar = pos.getBarCount();
        const auto numerator = ts.numerator;
        const auto denominator = ts.denominator;
        if (numerator == 0 || denominator == 0)
            return "1|1|000";

        const auto quarterNotesPerBar = (numerator * 4 / denominator);
        const auto beats = (fmod (quarterNotes - pos.getPpqPositionOfLastBarStart().orFallback(0), quarterNotesPerBar) / quarterNotesPerBar) * numerator;

        const auto bar = maybeBar.orFallback (((int) quarterNotes) / quarterNotesPerBar + 1);
        const auto beat = ((int) beats) + 1;
        const auto ticks = ((int) (fmod (beats, 1.0) * 960.0 + 0.5));

        return String::formatted ("%d|%d|%03d", bar, beat, ticks);
    }

    static bool isValidPreset (const juce::File& file, bool deep = false);

    static File getUserFolder();

    class TextEditorItem : public PopupMenu::CustomComponent
    {
    public:
        TextEditorItem()
        {
            addAndMakeVisible (editor);
        }
        void resized() override
        {
            editor.setBounds (getLocalBounds().reduced (12, 0));
        }
        void getIdealSize (int& idealWidth, int& idealHeight) override
        {
            idealWidth = editor.getTextWidth();
            idealHeight = editor.getTextHeight();
        }
        TextEditor editor;
    };

    class ParameterSliderItem : public PopupMenu::CustomComponent
    {
    public:
        ParameterSliderItem (AudioProcessorValueTreeState& state, const String& paramID) : PopupMenu::CustomComponent (false), attachment (state, paramID, slider)
        {
            slider.setTextBoxStyle (Slider::NoTextBox, true, 0, 0);
            addAndMakeVisible (slider);
        }
        void resized() override
        {
            slider.setBounds (getLocalBounds().reduced (12, 0));
        }
        void getIdealSize (int& idealWidth, int& idealHeight) override
        {
            idealWidth = 100;
            idealHeight = 40;
        }
        Slider slider;
        AudioProcessorValueTreeState::SliderAttachment attachment;
    };

    // copy-pasted from Foley's - https://github.com/ffAudio/foleys_gui_magic/blob/master/Helpers/foleys_Conversions.h#L41
    template <typename FloatType>
    static inline juce::NormalisableRange<FloatType> makeLogarithmicRange (FloatType min, FloatType max)
    {
        return juce::NormalisableRange<FloatType> (
            min, max, [] (FloatType start, FloatType end, FloatType normalised) { return start + (std::pow (FloatType (2), normalised * FloatType (10)) - FloatType (1)) * (end - start) / FloatType (1023); }, [] (FloatType start, FloatType end, FloatType value) { return (std::log (((value - start) * FloatType (1023) / (end - start)) + FloatType (1)) / std::log (FloatType (2))) / FloatType (10); }, [] (FloatType start, FloatType end, FloatType value) {
                    // optimised for frequencies: >3 kHz: 2 decimals
                    if (value > FloatType (3000))
                        return juce::jlimit (start, end, FloatType (100) * juce::roundToInt (value / FloatType (100)));

                    // optimised for frequencies: 1-3 kHz: 1 decimal
                    if (value > FloatType (1000))
                        return juce::jlimit (start, end, FloatType (10) * juce::roundToInt (value / FloatType (10)));

                    return juce::jlimit (start, end, FloatType (juce::roundToInt (value))); });
    }

#if JUCE_IOS

    static juce::File getAppGroupContainerLocation (const juce::String& appGroupID);

#endif
};
