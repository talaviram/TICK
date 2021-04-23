/*
  ==============================================================================

    LookAndFeel.h
    Created: 1 Nov 2019 6:13:31pm
    Author:  Tal Aviram

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"

class DialogComponent;

class TickLookAndFeel : public juce::LookAndFeel_V4
{
public:
#if JUCE_IOS || JUCE_ANDROID
    static constexpr int barHeight = 40;
    static constexpr int tabBarHeight = 40;
    static constexpr int toolbarHeight = 60;
#else
    static constexpr int barHeight = 40;
    static constexpr int tabBarHeight = 40;
    static constexpr int toolbarHeight = 40;
#endif

    struct Colours
    {
        static const juce::Colour defaultHighlight;
        static const juce::Colour secondaryColour;
        static const juce::Colour buttonSoftBackground;
        static const juce::Colour tickNameColour;
        static const juce::Colour backgroundColour;

        static const juce::Colour peach;
        static const juce::Colour clay;
        static const juce::Colour wood;
        static const juce::Colour mint;
        static const juce::Colour grey;
    };

    static void drawOpenCloseArrow (juce::Graphics& g, juce::Rectangle<float> area, juce::Colour colour, bool isOpened);
    static juce::Path createTransportIcon (juce::Rectangle<int> bounds, bool isPlaying, float proportion = 0.1f);
    static juce::Path getArrowPath (juce::Rectangle<float> arrowZone, const int direction, bool filled, const juce::Justification justification);

    static const juce::Colour& getBackgroundColour()
    {
        static const juce::Colour bg (50, 50, 50);
        return bg;
    }

    static const juce::Colour sampleColourPallete[];

    TickLookAndFeel();

    static juce::Typeface::Ptr getTypeface (const juce::String& name);
    juce::Typeface::Ptr getTypefaceForFont (const juce::Font&) override;

    juce::Font getPopupMenuFont() override;

    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const juce::Slider::SliderStyle style, juce::Slider& s) override;

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override;

    void drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown, int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& combo) override
    {
        g.setColour (combo.findColour (juce::ComboBox::ColourIds::backgroundColourId));
        g.fillRoundedRectangle (0, 0, width, height, 4);
    }

    void drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour& backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        if (shouldDrawButtonAsDown)
            g.setColour (backgroundColour.darker());
        else if (shouldDrawButtonAsHighlighted)
            g.setColour (backgroundColour.brighter());
        else
            g.setColour (backgroundColour);
        g.fillRect (b.getLocalBounds());
    }

    void fillTextEditorBackground (juce::Graphics&, int width, int height, juce::TextEditor&) override;
    void drawTextEditorOutline (juce::Graphics&, int width, int height, juce::TextEditor&) override;
    void drawDialogComponent (juce::Graphics& g, DialogComponent& alert, const juce::Rectangle<int>& textArea, juce::TextLayout& textLayout);
    juce::Array<int> getWidthsForTextButtons (const juce::Array<juce::TextButton*>& buttons);

    juce::Path getTickShape (float height) override;
};
