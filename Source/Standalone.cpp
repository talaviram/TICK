/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include <JuceHeader.h>
#include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>

#include "PluginProcessor.h"

using namespace juce;

//==============================================================================
class StandaloneFilterApp : public JUCEApplication
{
public:
    StandaloneFilterApp()
    {
        PluginHostType::jucePlugInClientCurrentWrapperType = AudioProcessor::wrapperType_Standalone;

        PropertiesFile::Options options;

        options.applicationName = getApplicationName();
        options.filenameSuffix = ".settings";
        options.osxLibrarySubFolder = "Application Support";
#if JUCE_LINUX || JUCE_BSD
        options.folderName = "~/.config";
#elif JUCE_ANDROID
        options.folderName = File::getSpecialLocation (File::commonApplicationDataDirectory).getFullPathName();
#else
        options.folderName = "";
#endif

        appProperties.setStorageParameters (options);
    }

    const String getApplicationName() override { return JucePlugin_Name; }
    const String getApplicationVersion() override { return JucePlugin_VersionString; }
    bool moreThanOneInstanceAllowed() override { return true; }
    void anotherInstanceStarted (const String&) override {}

    virtual StandaloneFilterWindow* createWindow()
    {
#ifdef JucePlugin_PreferredChannelConfigurations
        StandalonePluginHolder::PluginInOuts channels[] = { JucePlugin_PreferredChannelConfigurations };
#endif

        return new StandaloneFilterWindow (getApplicationName(),
                                           LookAndFeel::getDefaultLookAndFeel().findColour (ResizableWindow::backgroundColourId),
                                           appProperties.getUserSettings(),
                                           false,
                                           {},
                                           nullptr
#ifdef JucePlugin_PreferredChannelConfigurations
                                           ,
                                           juce::Array<StandalonePluginHolder::PluginInOuts> (channels, juce::numElementsInArray (channels))
#else
                                           ,
                                           {}
#endif
#if JUCE_DONT_AUTO_OPEN_MIDI_DEVICES_ON_MOBILE
                                               ,
                                           false
#endif
        );
    }

    //==============================================================================
    void initialise (const String&) override
    {
        mainWindow.reset (createWindow());
        mainWindow->setColour (ResizableWindow::backgroundColourId, Colours::transparentBlack);
        static_cast<TickAudioProcessor*> (mainWindow->getAudioProcessor())->setExternalProps (appProperties.getUserSettings());
#if JUCE_IOS
        mainWindow->getPeer()->setAppStyle (ComponentPeer::Style::dark);
#endif
#if JUCE_STANDALONE_FILTER_WINDOW_USE_KIOSK_MODE
        Desktop::getInstance().setKioskModeComponent (mainWindow.get(), false);
#endif

        mainWindow->setVisible (true);
    }

    void shutdown() override
    {
        mainWindow = nullptr;
        appProperties.saveIfNeeded();
    }

    void suspended() override
    {
        systemSuspended = true;
#if JUCE_IOS || JUCE_ANDROID
        mainWindow->pluginHolder->savePluginState();
        if (static_cast<TickAudioProcessor*> (mainWindow->getAudioProcessor())->playheadPosition_.getIsPlaying())
            return;
        Timer::callAfterDelay (60 * 1000, [this]() {
            // only after 1min
            if (! systemSuspended)
                return;
            if (auto app = JUCEApplicationBase::getInstance())
            {
                app->systemRequestedQuit();
            }
        });
#endif
    }

    void resumed() override
    {
        systemSuspended = false;
    }

    //==============================================================================
    void systemRequestedQuit() override
    {
        if (mainWindow.get() != nullptr)
            mainWindow->pluginHolder->savePluginState();

        if (ModalComponentManager::getInstance()->cancelAllModalComponents())
        {
            Timer::callAfterDelay (100, []() {
                if (auto app = JUCEApplicationBase::getInstance())
                    app->systemRequestedQuit();
            });
        }
        else
        {
            quit();
        }
    }

protected:
    ApplicationProperties appProperties;
    std::unique_ptr<StandaloneFilterWindow> mainWindow;

private:
    bool systemSuspended { false };
};

#if JucePlugin_Build_Standalone && JUCE_IOS

using namespace juce;

bool JUCE_CALLTYPE juce_isInterAppAudioConnected()
{
    if (auto holder = StandalonePluginHolder::getInstance())
        return holder->isInterAppAudioConnected();

    return false;
}

void JUCE_CALLTYPE juce_switchToHostApplication()
{
    if (auto holder = StandalonePluginHolder::getInstance())
        holder->switchToHostApplication();
}

Image JUCE_CALLTYPE juce_getIAAHostIcon (int size)
{
    if (auto holder = StandalonePluginHolder::getInstance())
        return holder->getIAAHostIcon (size);

    return Image();
}
#endif

START_JUCE_APPLICATION (StandaloneFilterApp)
