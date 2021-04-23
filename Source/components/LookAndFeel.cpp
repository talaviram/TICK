/*
  ==============================================================================

    LookAndFeel.cpp
    Created: 1 Nov 2019 6:13:31pm
    Author:  Tal Aviram

  ==============================================================================
*/

#include "LookAndFeel.h"
#include "DialogComponent.h"

const juce::Colour TickLookAndFeel::Colours::defaultHighlight = juce::Colours::skyblue;
const juce::Colour TickLookAndFeel::Colours::secondaryColour = juce::Colours::skyblue;
const juce::Colour TickLookAndFeel::Colours::buttonSoftBackground = juce::Colours::darkgrey.withAlpha (0.4f);
const juce::Colour TickLookAndFeel::Colours::tickNameColour = juce::Colours::transparentBlack.darker();
const juce::Colour TickLookAndFeel::Colours::backgroundColour = juce::Colours::black;

const juce::Colour TickLookAndFeel::Colours::peach = juce::Colour::fromString ("ffF2A099");
const juce::Colour TickLookAndFeel::Colours::clay = juce::Colour::fromString ("ffBF6B63");
const juce::Colour TickLookAndFeel::Colours::wood = juce::Colour::fromString ("ffC0A08E");
const juce::Colour TickLookAndFeel::Colours::mint = juce::Colour::fromString ("ffA8BFB2");
const juce::Colour TickLookAndFeel::Colours::grey = juce::Colour::fromString ("ff373640");

// should be greater or equal to kMaxTick...
const juce::Colour TickLookAndFeel::sampleColourPallete[] = {
    Colours::peach,
    Colours::mint,
    Colours::clay,
    Colours::wood,
    Colours::peach.brighter(),
    Colours::mint.brighter(),
    Colours::clay.brighter(),
    Colours::wood.brighter(),
    Colours::peach.brighter (2),
    Colours::mint.brighter (2),
    Colours::clay.brighter (2),
    Colours::wood.brighter (2),
    Colours::peach.brighter (3),
    Colours::mint.brighter (3),
    Colours::clay.brighter (3),
    Colours::wood.brighter (3)
};

juce::Path TickLookAndFeel::createTransportIcon (juce::Rectangle<int> b, const bool isPlaying, const float proportion)
{
    juce::Path p;
    auto halfLen = std::min<int> (b.getHeight(), b.getWidth()) * proportion;
    if (isPlaying)
    {
        p.addTriangle (b.getCentreX() + halfLen, b.getCentreY(), b.getCentreX() - halfLen, b.getCentreY() - halfLen, b.getCentreX() - halfLen, b.getCentreY() + halfLen);
    }
    else
    {
        p.addRectangle (b.getCentreX() - halfLen, b.getCentreY() - halfLen, halfLen * 2, halfLen * 2);
    }
    return p;
}

void TickLookAndFeel::drawOpenCloseArrow (juce::Graphics& g, juce::Rectangle<float> area, juce::Colour colour, bool isOpened)
{
    g.strokePath (getArrowPath (area, isOpened ? 2 : 1, false, juce::Justification::centredRight), juce::PathStrokeType (1.0f));
}

juce::Path TickLookAndFeel::getArrowPath (juce::Rectangle<float> arrowZone, const int direction, bool filled, const juce::Justification justification)
{
    auto w = std::min<int> (arrowZone.getWidth(), (direction == 0 || direction == 2) ? 8.0f : filled ? 5.0f : 8.0f);
    auto h = std::min<int> (arrowZone.getHeight(), (direction == 0 || direction == 2) ? 5.0f : filled ? 8.0f : 5.0f);

    if (justification == juce::Justification::centred)
    {
        arrowZone.reduce ((arrowZone.getWidth() - w) / 2, (arrowZone.getHeight() - h) / 2);
    }
    else if (justification == juce::Justification::centredRight)
    {
        arrowZone.removeFromLeft (arrowZone.getWidth() - w);
        arrowZone.reduce (0, (arrowZone.getHeight() - h) / 2);
    }
    else if (justification == juce::Justification::centredLeft)
    {
        arrowZone.removeFromRight (arrowZone.getWidth() - w);
        arrowZone.reduce (0, (arrowZone.getHeight() - h) / 2);
    }
    else
    {
        jassertfalse; // currently only supports centred justifications
    }

    juce::Path path;
    path.startNewSubPath (arrowZone.getX(), arrowZone.getBottom());
    path.lineTo (arrowZone.getCentreX(), arrowZone.getY());
    path.lineTo (arrowZone.getRight(), arrowZone.getBottom());

    if (filled)
        path.closeSubPath();

    path.applyTransform (juce::AffineTransform::rotation (direction * juce::MathConstants<float>::halfPi,
                                                          arrowZone.getCentreX(),
                                                          arrowZone.getCentreY()));

    return path;
}

TickLookAndFeel::TickLookAndFeel()
{
    LookAndFeel::setDefaultSansSerifTypefaceName ("KlokanTechNotoSans");
}

juce::Typeface::Ptr TickLookAndFeel::getTypefaceForFont (const juce::Font& f)
{
    if (f.getTypefaceName() == "<Sans-Serif>")
        return f.isBold() ? getTypeface ("KlokanTechNotoSans Bold") : getTypeface ("KlokanTechNotoSans");
    auto typeface = getTypeface (f.getTypefaceName());
    if (typeface != nullptr)
        return typeface;

    return LookAndFeel::getTypefaceForFont (f);
}

juce::Typeface::Ptr TickLookAndFeel::getTypeface (const juce::String& name)
{
    static auto regular = juce::Typeface::createSystemTypefaceFor (BinaryData::KlokanTechNotoSansRegular_ttf, BinaryData::KlokanTechNotoSansRegular_ttfSize);
    static auto bold = juce::Typeface::createSystemTypefaceFor (BinaryData::KlokanTechNotoSansBold_ttf, BinaryData::KlokanTechNotoSansBold_ttfSize);

    if (name == "KlokanTechNotoSans")
        return regular;
    else if (name == "KlokanTechNotoSans Bold")
        return bold;

    return nullptr;
}

juce::Font TickLookAndFeel::getPopupMenuFont()
{
#if JUCE_IOS || JUCE_ANDROID
    return juce::Font (35.0);
#else
    return juce::Font (20.0);
#endif
}

void TickLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const juce::Slider::SliderStyle style, juce::Slider& s)
{
    if (s.isBar())
    {
        const auto corner = std::min<int> (width, height) * 0.3f;
        const auto bounds = s.getLocalBounds().toFloat();
        juce::Path sliderPath;
        sliderPath.addRoundedRectangle (bounds, corner);
        g.setColour (juce::Colours::white.withAlpha (0.1f));
        g.fillPath (sliderPath);
        g.setColour (juce::Colours::whitesmoke);
        const auto fill = s.getPositionOfValue (s.getValue());
        juce::Rectangle<float> rect;
        if (style == juce::Slider::SliderStyle::LinearBar)
        {
            rect = juce::Rectangle<float> (0, 0, juce::roundToInt (fill), bounds.getHeight());
        }
        else if (style == juce::Slider::SliderStyle::LinearBarVertical)
        {
            rect = juce::Rectangle<float> (0, juce::roundToInt (fill), bounds.getWidth(), juce::roundToInt (bounds.getHeight() - fill));
        }

        {
            juce::Graphics::ScopedSaveState state (g);
            g.reduceClipRegion (sliderPath);
            g.fillRect (rect);
            g.setColour (juce::Colours::black);
        }
    }
    else
        LookAndFeel_V4::drawLinearSlider (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, s);
}

void TickLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider)
{
    auto outline = slider.findColour (juce::Slider::rotarySliderOutlineColourId);
    auto fill = slider.findColour (juce::Slider::rotarySliderFillColourId);

    auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (10);

    auto radius = std::min<int> (bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    auto lineW = std::min<float> (8.0f, radius * 0.5f);
    auto arcRadius = radius - lineW * 0.5f;

    juce::Path backgroundArc;
    backgroundArc.addCentredArc (bounds.getCentreX(),
                                 bounds.getCentreY(),
                                 arcRadius,
                                 arcRadius,
                                 0.0f,
                                 rotaryStartAngle,
                                 rotaryEndAngle,
                                 true);

    g.setColour (outline);
    g.strokePath (backgroundArc, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    if (slider.isEnabled())
    {
        juce::Path valueArc;
        valueArc.addCentredArc (bounds.getCentreX(),
                                bounds.getCentreY(),
                                arcRadius,
                                arcRadius,
                                0.0f,
                                rotaryStartAngle,
                                toAngle,
                                true);

        g.setColour (fill);
        g.strokePath (valueArc, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
}

void TickLookAndFeel::fillTextEditorBackground (juce::Graphics& g, int width, int height, juce::TextEditor& textEditor)
{
    if ((dynamic_cast<juce::AlertWindow*> (textEditor.getParentComponent()) == nullptr) || (dynamic_cast<DialogComponent*> (textEditor.getParentComponent()) == nullptr))
    {
        g.setColour (textEditor.findColour (juce::TextEditor::backgroundColourId));
        g.fillRect (0, 0, width, height);

        g.setColour (textEditor.findColour (juce::TextEditor::outlineColourId));
        g.drawHorizontalLine (height - 1, 0.0f, static_cast<float> (width));
    }
    else
    {
        LookAndFeel_V2::fillTextEditorBackground (g, width, height, textEditor);
    }
}

void TickLookAndFeel::drawTextEditorOutline (juce::Graphics& g, int width, int height, juce::TextEditor& textEditor)
{
    if ((dynamic_cast<juce::AlertWindow*> (textEditor.getParentComponent()) == nullptr) && (dynamic_cast<DialogComponent*> (textEditor.getParentComponent()) == nullptr))
    {
        if (textEditor.isEnabled())
        {
            if (textEditor.hasKeyboardFocus (true) && ! textEditor.isReadOnly())
            {
                g.setColour (textEditor.findColour (juce::TextEditor::focusedOutlineColourId));
                g.drawRect (0, 0, width, height, 2);
            }
            else
            {
                g.setColour (textEditor.findColour (juce::TextEditor::outlineColourId));
                g.drawRect (0, 0, width, height);
            }
        }
    }
}

void TickLookAndFeel::drawDialogComponent (juce::Graphics& g, DialogComponent& alert, const juce::Rectangle<int>& textArea, juce::TextLayout& textLayout)
{
    auto bounds = alert.getLocalBounds().reduced (1);
    g.reduceClipRegion (bounds);

    g.setColour (alert.findColour (juce::AlertWindow::backgroundColourId));
    g.fillRect (bounds.toFloat());
    g.setColour (juce::Colours::white.withAlpha (0.1f));
    g.drawRect (bounds.toFloat());

    auto iconSpaceUsed = 0;

    auto iconWidth = 80;
    auto iconSize = std::min<int> (iconWidth + 50, bounds.getHeight() + 20);

    if (alert.containsAnyExtraComponents() || alert.getNumButtons() > 2)
        iconSize = std::min<int> (iconSize, textArea.getHeight() + 50);

    juce::Rectangle<int> iconRect (iconSize / -10, iconSize / -10, iconSize, iconSize);

    g.setColour (alert.findColour (juce::AlertWindow::textColourId));

    juce::Rectangle<int> alertBounds (bounds.getX() + iconSpaceUsed, 30, bounds.getWidth(), bounds.getHeight() - getAlertWindowButtonHeight() - 20);

    textLayout.draw (g, alertBounds.toFloat());
}

juce::Array<int> TickLookAndFeel::getWidthsForTextButtons (const juce::Array<juce::TextButton*>& buttons)
{
    const int n = buttons.size();
    juce::Array<int> buttonWidths;

    const int buttonHeight = getAlertWindowButtonHeight();

    for (int i = 0; i < n; ++i)
        buttonWidths.add (getTextButtonWidthToFitText (*buttons.getReference (i), buttonHeight));

    return buttonWidths;
}

juce::Path TickLookAndFeel::getTickShape (float height)
{
    auto check = juce::Drawable::createFromImageData (BinaryData::check_circle24px_svg, BinaryData::check_circle24px_svgSize);
    return check->getOutlineAsPath();
}
