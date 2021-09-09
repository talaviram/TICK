#pragma once

#include "JuceHeader.h"
#include "utils/git_version.h"

class AboutView : public juce::Component
{
public:
    AboutView (String wrapperType) : about (aboutText, nullptr), wrapperType (wrapperType)
    {
        using namespace juce;
        background = Drawable::createFromImageData (BinaryData::background_png, BinaryData::background_pngSize);
        addAndMakeVisible (*background);
        about.setColour (TextEditor::textColourId, Colours::white);
        about.setColour (TextEditor::backgroundColourId, Colours::white.withAlpha (0.1f));
        about.setColour (TextEditor::outlineColourId, Colours::transparentBlack);
        about.setColour (TextEditor::focusedOutlineColourId, Colours::transparentBlack);
        about.setLineNumbersShown (false);
        about.setWantsKeyboardFocus (false);
        about.setReadOnly (true);
        aboutText.replaceAllContent (BinaryData::about_text_txt);
        addAndMakeVisible (about);
        logo = Drawable::createFromImageData (BinaryData::tick_icon_with_text_svg, BinaryData::tick_icon_with_text_svgSize);
    }

    void mouseUp (const juce::MouseEvent&) override
    {
        setVisible (false);
    }

    void resized() override
    {
        const auto isHorizontal = getWidth() > getHeight();
        auto area = getLocalBounds();
        background->setTransformToFit (area.toFloat(), RectanglePlacement (RectanglePlacement::stretchToFit));
        if (isHorizontal)
            area.removeFromLeft (getWidth() * 0.3);
        else
        {
            area.removeFromTop (150);
            area.removeFromBottom (80);
        }
        about.setBounds (area.reduced (10));
    }

    void paintOverChildren (juce::Graphics& g) override
    {
        juce::String arch = " ["
#if JUCE_ARM
                            "arm"
#if JUCE_64BIT
                            "64"
#endif
#elif JUCE_INTEL
                            "x86"
#if JUCE_64BIT
                            "_64"
#endif
#else
#error "Unexpected Arch!"
#endif
                            "]\n";
        const auto isHorizontal = getWidth() > getHeight();
        auto area = getLocalBounds().removeFromTop (150);
        if (isHorizontal)
            area = area.removeFromLeft (getWidth() * 0.3);
        logo->drawWithin (g, area.toFloat(), RectanglePlacement(), 1.0f);
        g.setFont (Font (15.0f));
        g.setColour (Colours::white);
        juce::String version = JucePlugin_VersionString " (" + juce::String (GIT_COMMIT) + ")\n";
        auto textArea = getLocalBounds();
        if (isHorizontal)
            textArea = textArea.removeFromLeft (getWidth() * 0.3);
        g.drawFittedText (version + wrapperType + arch + (JucePlugin_Manufacturer ", Copyright 2019-2021"), textArea.removeFromBottom (80), Justification::centred, 1);
    }

private:
    std::unique_ptr<juce::Drawable> logo, background;
    juce::CodeDocument aboutText;
    juce::CodeEditorComponent about;
    juce::String wrapperType {};
};
