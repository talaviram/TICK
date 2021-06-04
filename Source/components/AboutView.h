#pragma once

#include "JuceHeader.h"

class AboutView : public juce::Component
{
public:
    AboutView() : about (aboutText, nullptr)
    {
        using namespace juce;
        about.setColour (TextEditor::textColourId, Colours::white);
        about.setColour (TextEditor::backgroundColourId, Colours::white.withAlpha (0.1f));
        about.setColour (TextEditor::outlineColourId, Colours::transparentBlack);
        about.setColour (TextEditor::focusedOutlineColourId, Colours::transparentBlack);
        about.setLineNumbersShown (false);
        about.setWantsKeyboardFocus (false);
        about.setReadOnly (true);
        aboutText.replaceAllContent (BinaryData::about_text_txt);
        addAndMakeVisible (about);
        logo = Drawable::createFromImageData (BinaryData::ticklogo_svg, BinaryData::ticklogo_svgSize);
    }

    void mouseUp (const juce::MouseEvent&) override
    {
        setVisible (false);
    }

    void resized() override
    {
        auto area = getLocalBounds();
        area.removeFromTop (150);
        area.removeFromBottom (50);
        about.setBounds (area.reduced (10));
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colours::darkgrey.darker (3));

        auto area = getLocalBounds().removeFromTop (150);
        g.fillAll (Colours::black);
        g.setFont (Font (15.0f));
        g.setColour (Colours::white);
        logo->drawWithin (g, getLocalBounds().toFloat(), juce::RectanglePlacement(), 0.5f);
        g.drawFittedText (JucePlugin_Manufacturer ", Copyright 2019-2021", area.removeFromBottom (40), Justification::centredBottom, 1);
        g.setFont (Font (40.0f));
        g.drawFittedText (JucePlugin_Desc, area.removeFromBottom (40), Justification::centredTop, 1);
    }

private:
    std::unique_ptr<juce::Drawable> logo;
    juce::CodeDocument aboutText;
    juce::CodeEditorComponent about;
};
