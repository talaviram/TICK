/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"
#include "PluginProcessor.h"

#include "components/AboutView.h"
#include "components/ManageSamplesView.h"
#include "components/PresetsView.h"

#include "JUX/components/ListBoxMenu.h"
#include "utils/UtilityFunctions.h"

static bool didShowSplashOnce { false };
//==============================================================================
TickAudioProcessorEditor::TickAudioProcessorEditor (TickAudioProcessor& p)
    : AudioProcessorEditor (&p), samplesButton ("Sounds", juce::DrawableButton::ButtonStyle::ImageFitted), settingsButton ("settingsButton", juce::DrawableButton::ImageFitted), processor (p)
{
#if JUCE_DEBUG
    didShowSplashOnce = true;
#endif
    auto& state = processor.getState();
    background.setBufferedToImage (true);
    addAndMakeVisible (background);
    juce::LookAndFeel::setDefaultLookAndFeel (&lookAndFeel);

    lookAndFeel.setColour (juce::PopupMenu::ColourIds::highlightedTextColourId, juce::Colours::skyblue);
    lookAndFeel.setColour (juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    lookAndFeel.setColour (juce::AlertWindow::outlineColourId, juce::Colours::white.withAlpha (0.2f));
    lookAndFeel.setColour (juce::AlertWindow::backgroundColourId, juce::Colours::darkgrey.withAlpha (0.9f));
    lookAndFeel.setColour (juce::ListBox::ColourIds::backgroundColourId, juce::Colours::black.withAlpha (0.6f));
    lookAndFeel.setColour (juce::PopupMenu::ColourIds::backgroundColourId, juce::Colours::black.withAlpha (0.6f));
    lookAndFeel.setColour (juce::ResizableWindow::ColourIds::backgroundColourId, TickLookAndFeel::Colours::backgroundColour);
    lookAndFeel.setColour (juce::Slider::trackColourId, juce::Colours::lightgrey);
    lookAndFeel.setColour (juce::Slider::thumbColourId, juce::Colours::white);
    lookAndFeel.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    lookAndFeel.setColour (juce::TextButton::ColourIds::buttonColourId, juce::Colours::darkgrey.darker().darker());
    lookAndFeel.setColour (juce::ScrollBar::ColourIds::thumbColourId, juce::Colours::white.withAlpha (0.4f));

    lookAndFeel.setColour (juce::Slider::ColourIds::rotarySliderFillColourId, TickLookAndFeel::Colours::defaultHighlight);

    addAndMakeVisible (headerArea);
    headerName.setColour (juce::Label::ColourIds::textColourId, juce::Colours::skyblue);

    headerArea.setColour (juce::Label::ColourIds::backgroundColourId, juce::Colours::black.withAlpha (0.3f));
    editModeButton.setButtonText ("Edit");
    editModeButton.setClickingTogglesState (true);
    editModeButton.setColour (juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::transparentBlack);
    editModeButton.setColour (juce::TextButton::ColourIds::buttonColourId, juce::Colours::transparentBlack);
    editModeButton.setColour (juce::TextButton::ColourIds::textColourOffId, juce::Colours::skyblue);
    editModeButton.setColour (juce::TextButton::ColourIds::textColourOnId, juce::Colours::skyblue);
    editModeButton.setColour (juce::TextButton::ColourIds::textColourOffId, juce::Colours::skyblue);
    editModeButton.setToggleState (static_cast<bool> (state.view.isEdit.getValue()), juce::dontSendNotification);
    editModeButton.getToggleStateValue().referTo (state.view.isEdit);
    samplesButton.setClickingTogglesState (true);
    samplesButton.setColour (DrawableButton::backgroundOnColourId, Colours::transparentBlack);
    samplesButton.getToggleStateValue().referTo (state.view.showEditSamples);
    headerArea.addAndMakeVisible (editModeButton);

    auto samplesIcon = juce::DrawablePath::createFromImageData (BinaryData::sampleicon_svg, BinaryData::sampleicon_svgSize);
    samplesIcon->replaceColour (juce::Colours::black, juce::Colours::white);
    auto backIcon = juce::DrawablePath::createFromImageData (BinaryData::arrow_back_ios24px_svg, BinaryData::arrow_back_ios24px_svgSize);
    backIcon->replaceColour (juce::Colours::black, juce::Colours::white);
    samplesButton.setImages (samplesIcon.get(), nullptr, nullptr, nullptr, backIcon.get());
    headerArea.addChildComponent (samplesButton);

    auto settingsOff = juce::Drawable::createFromImageData (BinaryData::cog_fa_svg, BinaryData::cog_fa_svgSize);
    auto settingsOn = settingsOff->createCopy();
    settingsOn->replaceColour (juce::Colours::black, TickLookAndFeel::Colours::peach);
    settingsOff->replaceColour (juce::Colours::black, juce::Colours::white);
    settingsButton.setImages (settingsOff.get(), nullptr, settingsOn.get(), nullptr, settingsOn.get());
    addAndMakeVisible (settingsButton);

    settingsButton.onClick = [this] {
        auto slider = std::make_unique<TickUtils::ParameterSliderItem> (processor.getAPVTS(), IDs::filterCutoff.toString());
        PopupMenu settings;
        settings.addSectionHeader ("Sync");
        auto& transport = processor.getState().useHostTransport;
        settings.addItem ("Internal", true, ! transport.get(), [&transport] {
            transport.setValue (false, nullptr);
        });
        settings.addItem ("External (Host)", true, transport.get(), [&transport] {
            transport.setValue (true, nullptr);
        });
        settings.addSeparator();
        auto& showWaveform = processor.getState().showWaveform;
        settings.addItem ("Always Show Waveform", true, showWaveform.get(), [&showWaveform] {
            showWaveform.setValue (! showWaveform.get(), nullptr);
        });
        settings.addSeparator();
        settings.addSectionHeader ("Low-Pass Filter");
        settings.addCustomItem (222, std::move (slider));
        settings.addSeparator();
        settings.addItem ("About", [this] {
            aboutView->setVisible (true);
        });
        settings.showMenuAsync (PopupMenu::Options().withMinimumWidth (100).withMaximumNumColumns (3).withTargetComponent (&settingsButton));

    };

    addAndMakeVisible (mainArea);

    samplesPaint = std::make_unique<SamplesPaint> (processor.getTicks());

    samplesView = std::make_unique<ManageSamplesView> (*samplesPaint, processor.getTicks());
    samplesView->closeButton.onClick = [this, &state] {
        state.view.showEditSamples = false;
    };
    mainArea.addChildComponent (*samplesView);

    topBar.centerLabel.getTextValue().referTo (state.presetName.getPropertyAsValue());
    topBar.centerLabel.onClick = [this] {
        auto& showPresetValue = processor.getState().view.showPresetsView;
        const bool value = showPresetValue.getValue();
        showPresetValue.setValue (! value);
    };
    mainArea.addAndMakeVisible (topBar);

    bottomBar.transportButton.getToggleStateValue().referTo (state.transport.isPlaying.getPropertyAsValue());
    addAndMakeVisible (bottomBar);

    performView.reset (new PerformView (processor.getState(), processor.getTicks(), *samplesPaint));
    mainArea.addAndMakeVisible (*performView);

    presetsView.reset (new PresetsView (processor.getState(), processor.getTicks()));
    mainArea.addAndMakeVisible (*presetsView);

    aboutView.reset (new AboutView());
    addChildComponent (aboutView.get());
    aboutView->setAlwaysOnTop (true);

    // register view state notifications
    processor.getState().view.isEdit.addListener (this);
    processor.getState().view.showEditSamples.addListener (this);
    processor.getState().view.showPresetsView.addListener (this);

    if (! didShowSplashOnce)
    {
        didShowSplashOnce = true;
        splash_unique.reset (new TickSplash (*this));
        splash = splash_unique.get();
    }

    setResizable (true, true);
    //    375 x 667 iPhone 6
#if JUCE_WINDOWS || JUCE_MAC || JUCE_LINUX
    setResizeLimits (280, 500, 2048, 4096);
    setSize (280, 500);
#else
    setSize (375, 667);
#endif

    startTimerHz (50);
}

TickAudioProcessorEditor::~TickAudioProcessorEditor()
{
    // unregister view state notifications
    processor.getState().view.isEdit.removeListener (this);
    processor.getState().view.showEditSamples.removeListener (this);
    processor.getState().view.showPresetsView.removeListener (this);
    juce::LookAndFeel::setDefaultLookAndFeel (nullptr);
}

//==============================================================================
TickAudioProcessorEditor::Background::Background()
{
    bgImage = juce::Drawable::createFromImageData (BinaryData::background_png, BinaryData::background_pngSize);
    addAndMakeVisible (bgImage.get());
}

void TickAudioProcessorEditor::Background::resized()
{
    bgImage->setBounds (getLocalBounds());
}

void TickAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (TickLookAndFeel::Colours::backgroundColour);
}

void TickAudioProcessorEditor::resized()
{
#if JUCE_IOS
    // Modern iOS/iPadOS device fill 'all screen'...
    // nasty way detecting older devices...
    static const int safeBottom = juce::SystemStats::getDeviceDescription().contains ("iPad") || getParentHeight() < 670 ? 0 : 20;
#else
    static const int safeBottom = 0;
#endif
    const auto availableArea = getLocalBounds().withTrimmedBottom (safeBottom);
    auto topArea = availableArea;
    headerArea.setBounds (topArea.removeFromTop (TickLookAndFeel::toolbarHeight));
    bottomBar.setBounds (topArea.removeFromBottom (TickLookAndFeel::toolbarHeight));
    editModeButton.setBounds (headerArea.getBounds().removeFromRight (60));
    samplesButton.setBounds (headerArea.getBounds().removeFromLeft (headerArea.getHeight()).reduced (5));

    settingsButton.setBounds (samplesButton.getBounds().reduced (5));
    mainArea.setBounds (topArea);

    background.setBounds (getLocalBounds());
    auto performViewArea = mainArea.getLocalBounds();
    topBar.setBounds (mainArea.getLocalBounds().removeFromTop (TickLookAndFeel::barHeight));
    performView->setBounds (performViewArea);
    presetsView->setBounds (mainArea.getLocalBounds().translated (0, (bool) processor.getState().view.showPresetsView.getValue() == true ? 0 : getHeight()));
    aboutView->setBounds (getLocalBounds());
}

void TickAudioProcessorEditor::valueChanged (juce::Value& value)
{
    auto& state = processor.getState();
#if JUCE_DEBUG
    // Indicate dirty only while debugging
    repaint();
#endif
    if (value.refersToSameSourceAs (state.view.isEdit))
    {
        if (static_cast<bool> (value.getValue()))
        {
            editModeButton.setButtonText ("Done");
            performView->setEditMode (true);
        }
        else
        {
            editModeButton.setButtonText ("Edit");
            performView->setEditMode (false);
            performView->setVisible (true);
            samplesView->setVisible (false);
            samplesButton.setToggleState (false, juce::dontSendNotification);
        }
    }
    else if (value.refersToSameSourceAs (state.view.showEditSamples))
    {
        samplesView->updateSelection (state.selectedEdit);
        if (value.getValue())
            samplesView->toFront (false);
        const auto baseBounds = performView->getBounds().reduced (4, 0).withTrimmedTop (250);
        samplesView->setBounds (value.getValue() ? baseBounds.translated (0, getHeight()) : baseBounds);
        const auto to = value.getValue() ? baseBounds : baseBounds.translated (0, getHeight());
        juce::Desktop::getInstance().getAnimator().animateComponent (samplesView.get(), to, 1.0f, 200, false, 1.0, 1.0);
    }
    else if (value.refersToSameSourceAs (state.view.showPresetsView))
    {
        presetsView->setBounds (value.getValue() ? getLocalBounds().translated (0, getHeight()) : getLocalBounds());
        const auto to = value.getValue() ? getLocalBounds() : getLocalBounds().translated (0, getHeight());
        juce::Desktop::getInstance().getAnimator().animateComponent (presetsView.get(), to, 1.0f, 200, false, 1.0, 1.0);
        presetsView->toFront (false);
        topBar.centerLabel.setColour (juce::Label::ColourIds::textColourId, (bool) value.getValue() == true ? juce::Colours::skyblue : juce::Colours::white);
    }
}

void TickAudioProcessorEditor::timerCallback()
{
    bool useHostTransport = processor.getState().useHostTransport.get();
    performView->update (processor.getCurrentBeatPos());
    bottomBar.syncIndicator.setVisible (useHostTransport);
    bottomBar.transportButton.setVisible (! useHostTransport);
    bottomBar.transportPosition.setVisible (useHostTransport);
    if (useHostTransport)
    {
        bottomBar.transportPosition.setText (TickUtils::generateTimecodeDisplay (processor.lastKnownPosition_), dontSendNotification);
    }
}

TickAudioProcessorEditor::BottomBar::BottomBar()
    : transportButton ("transportButton", juce::DrawableButton::ImageFitted), syncIndicator ("syncIndicator", juce::DrawableButton::ImageAboveTextLabel)
{
    auto syncOn = juce::Drawable::createFromImageData (BinaryData::lock_clock24px_svg, BinaryData::lock_clock24px_svgSize);
    syncOn->replaceColour (Colours::black, Colours::white);
    syncIndicator.setTooltip ("Transport Controlled from Host");
    syncIndicator.setButtonText ("EXT SYNC");
    syncIndicator.setImages (syncOn.get(), nullptr, nullptr, nullptr);
    syncIndicator.setInterceptsMouseClicks (false, false);
    addChildComponent (syncIndicator);

    auto transportIconPlay = Drawable::createFromImageData (BinaryData::playbutton_svg, BinaryData::playbutton_svgSize);
    auto transportIconStop = Drawable::createFromImageData (BinaryData::stopbutton_svg, BinaryData::stopbutton_svgSize);
    transportIconPlay->replaceColour (Colours::black, Colours::white);
    transportIconStop->replaceColour (Colours::black, Colours::white);
    transportButton.setImages (transportIconPlay.get(), nullptr, nullptr, nullptr, transportIconStop.get());
    transportButton.setColour (juce::DrawableButton::ColourIds::backgroundColourId, juce::Colours::transparentBlack);
    transportButton.setColour (juce::DrawableButton::ColourIds::backgroundOnColourId, juce::Colours::transparentBlack);
    transportButton.setClickingTogglesState (true);
    addAndMakeVisible (transportButton);

    transportPosition.setInterceptsMouseClicks (false, false);
    transportPosition.setJustificationType (juce::Justification::centred);
    transportPosition.setText ("0|0|000", juce::dontSendNotification);
    addAndMakeVisible (transportPosition);
}

void TickAudioProcessorEditor::BottomBar::paint (Graphics& g)
{
    ColourGradient gradient (Colours::grey.withAlpha (0.2f), 0, 0, Colours::darkgrey.withAlpha (0.4f), 0, getLocalBounds().getBottom(), false);
    g.setGradientFill (gradient);
    g.fillAll();
}

void TickAudioProcessorEditor::BottomBar::resized()
{
    auto area = getLocalBounds();
    transportPosition.setBounds (area);
    transportButton.setBounds (area.reduced (6, 0));
    syncIndicator.setBounds (transportButton.getBounds().removeFromLeft (getHeight()).reduced (4));
}
