#pragma once

#include "JuceHeader.h"

class AboutView : public juce::Component
{
public:
    AboutView() : about (aboutText, nullptr)
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
        auto area = getLocalBounds();
        background->setTransformToFit (area.toFloat(), RectanglePlacement());
        area.removeFromTop (150);
        area.removeFromBottom (50);
        about.setBounds (area.reduced (10));
    }

    void paintOverChildren (juce::Graphics& g) override
    {
        auto area = getLocalBounds().removeFromTop (150);
        logo->drawWithin (g, area.toFloat(), RectanglePlacement(), 1.0f);
        g.setFont (Font (15.0f));
        g.setColour (Colours::white);
        g.drawFittedText (JucePlugin_Manufacturer ", Copyright 2019-2021", getLocalBounds().removeFromBottom (40), Justification::centred, 1);
    }

private:
    std::unique_ptr<juce::Drawable> logo, background;
    juce::CodeDocument aboutText;
    juce::CodeEditorComponent about;
};
