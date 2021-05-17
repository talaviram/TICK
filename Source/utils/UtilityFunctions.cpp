/*
  ==============================================================================

    UtilityFunctions.cpp
    Created: 3 Jan 2020 5:33:00pm
    Author:  Tal Aviram

  ==============================================================================
*/

#include "UtilityFunctions.h"
#include "components/LookAndFeel.h"
#include "model/JuceState.h"

const char* TickUtils::kPresetExtension = ".preset";

File TickUtils::getUserFolder()
{
    return
#if ! JUCE_IOS
        File::getSpecialLocation(File::commonDocumentsDirectory).getChildFile(String(JucePlugin_Manufacturer)).getChildFile(String(JucePlugin_Name)).getChildFile("Presets")
#else
        File::getSpecialLocation (File::userDocumentsDirectory)
#endif
        ;
}

String TickUtils::getPresetHash (InputStream& zipSource)
{
    ZipFile archive (zipSource);
    auto idx = archive.getIndexOfFileName (INFO_FILE_NAME);
    if (idx >= 0)
    {
        std::unique_ptr<InputStream> stream (archive.createStreamForEntry (idx));
        auto parsed = XmlDocument::parse (stream->readEntireStreamAsString());
        if (parsed)
            return parsed->getStringAttribute (IDs::uuid);
    }
    return String();
}

std::unique_ptr<Drawable> replaceColourAndReturnCopy (Drawable* baseImage, Colour newColour, Colour baseColour = Colours::black)
{
    auto colouredImage = baseImage->createCopy();
    colouredImage->replaceColour (baseColour, newColour);
    return colouredImage;
}

// TODO: REUSE THIS!!!
void TickUtils::setDrawableFromSVG (juce::DrawableButton* btn, juce::Drawable* baseImage)
{
    using namespace juce;
    std::unique_ptr<Drawable> normal = replaceColourAndReturnCopy (baseImage, Colours::white.darker());
    std::unique_ptr<Drawable> over = replaceColourAndReturnCopy (baseImage, Colours::white);
    std::unique_ptr<Drawable> down = replaceColourAndReturnCopy (baseImage, Colours::white.darker().darker());
    std::unique_ptr<Drawable> disabled = replaceColourAndReturnCopy (baseImage, Colours::darkgrey);
    std::unique_ptr<Drawable> normalOn = replaceColourAndReturnCopy (baseImage, TickLookAndFeel::Colours::secondaryColour);
    std::unique_ptr<Drawable> overOn = replaceColourAndReturnCopy (baseImage, TickLookAndFeel::Colours::secondaryColour.brighter());
    std::unique_ptr<Drawable> downOn = replaceColourAndReturnCopy (baseImage, TickLookAndFeel::Colours::secondaryColour.darker());
    btn->setImages (normal.get(), over.get(), down.get(), disabled.get(), normalOn.get(), overOn.get(), downOn.get());
}
