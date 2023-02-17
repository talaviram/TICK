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

//==============================================================================
TickAudioProcessorEditor::TickAudioProcessorEditor (TickAudioProcessor& p)
    : AudioProcessorEditor (&p), samplesButton ("Sounds", juce::DrawableButton::ButtonStyle::ImageFitted), settingsButton ("settingsButton", juce::DrawableButton::ImageFitted), sidePanel ("TICK", 280, true), tickProcessor (p)
{
// splash is a 'nicer way' to make JUCE splash requirement for non-GPL builds.
// this is needed for any non-GPL compliant build...
#if ! JUCE_DISPLAY_SPLASH_SCREEN
    TickSplash::didShowSplashOnce = true;
#endif
    initAppProperties();
    auto& state = tickProcessor.getState();
    background.setBufferedToImage (true);
    addAndMakeVisible (background);
    juce::LookAndFeel::setDefaultLookAndFeel (&lookAndFeel);

    lookAndFeel.setColour (juce::Label::outlineWhenEditingColourId, juce::Colours::transparentBlack);
    lookAndFeel.setColour (juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
    lookAndFeel.setColour (juce::PopupMenu::highlightedBackgroundColourId, TickLookAndFeel::Colours::mint.withAlpha (0.6f));
    lookAndFeel.setColour (juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    lookAndFeel.setColour (juce::AlertWindow::outlineColourId, juce::Colours::white.withAlpha (0.2f));
    lookAndFeel.setColour (juce::AlertWindow::backgroundColourId, juce::Colours::darkgrey.withAlpha (0.9f));
    lookAndFeel.setColour (juce::ListBox::ColourIds::backgroundColourId, juce::Colours::black.withAlpha (0.6f));
    lookAndFeel.setColour (juce::PopupMenu::ColourIds::backgroundColourId, TickLookAndFeel::Colours::grey.withAlpha (0.95f));
    lookAndFeel.setColour (juce::ResizableWindow::ColourIds::backgroundColourId, TickLookAndFeel::Colours::backgroundColour);
    lookAndFeel.setColour (juce::Slider::trackColourId, juce::Colours::lightgrey);
    lookAndFeel.setColour (juce::Slider::thumbColourId, juce::Colours::white);
    lookAndFeel.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    lookAndFeel.setColour (juce::TextButton::ColourIds::buttonColourId, juce::Colours::darkgrey.darker().darker());
    lookAndFeel.setColour (juce::ScrollBar::ColourIds::thumbColourId, juce::Colours::white.withAlpha (0.4f));

    lookAndFeel.setColour (juce::Slider::ColourIds::rotarySliderFillColourId, TickLookAndFeel::Colours::defaultHighlight);

    headerArea.setFocusContainerType (FocusContainerType::focusContainer);
    addAndMakeVisible (headerArea);
    headerName.setColour (juce::Label::ColourIds::textColourId, TickLookAndFeel::Colours::mint);

    headerArea.setColour (juce::Label::ColourIds::backgroundColourId, juce::Colours::transparentBlack);
    editModeButton.setButtonText ("Edit");
    editModeButton.setDescription ("Open/Close Samples Editor");
    editModeButton.setClickingTogglesState (true);
    editModeButton.setColour (juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::transparentBlack);
    editModeButton.setColour (juce::TextButton::ColourIds::buttonColourId, juce::Colours::transparentBlack);
    editModeButton.setColour (juce::TextButton::ColourIds::textColourOffId, TickLookAndFeel::Colours::mint);
    editModeButton.setColour (juce::TextButton::ColourIds::textColourOnId, TickLookAndFeel::Colours::mint);
    editModeButton.setColour (juce::TextButton::ColourIds::textColourOffId, TickLookAndFeel::Colours::mint);
    editModeButton.setToggleState (static_cast<bool> (state.view.isEdit.getValue()), juce::dontSendNotification);
    editModeButton.getToggleStateValue().referTo (state.view.isEdit);
    samplesButton.setClickingTogglesState (true);
    samplesButton.setColour (DrawableButton::backgroundOnColourId, Colours::transparentBlack);
    samplesButton.getToggleStateValue().referTo (state.view.showEditSamples);
    addAndMakeVisible (editModeButton);

    auto samplesIcon = juce::DrawablePath::createFromImageData (BinaryData::sampleicon_svg, BinaryData::sampleicon_svgSize);
    samplesIcon->replaceColour (juce::Colours::black, juce::Colours::white);
    auto backIcon = juce::DrawablePath::createFromImageData (BinaryData::arrow_back_ios24px_svg, BinaryData::arrow_back_ios24px_svgSize);
    backIcon->replaceColour (juce::Colours::black, juce::Colours::white);
    samplesButton.setImages (samplesIcon.get(), nullptr, nullptr, nullptr, backIcon.get());
    headerArea.addChildComponent (samplesButton);

    auto settingsOff = juce::Drawable::createFromImageData (BinaryData::settings_black_24dp_svg, BinaryData::settings_black_24dp_svgSize);
    auto settingsOn = settingsOff->createCopy();
    settingsOn->replaceColour (juce::Colours::black, TickLookAndFeel::Colours::peach);
    settingsOff->replaceColour (juce::Colours::black, juce::Colours::white);
    settingsButton.setImages (settingsOff.get(), nullptr, settingsOn.get(), nullptr, settingsOn.get());
    addAndMakeVisible (settingsButton);

    settingsButton.onClick = [this]
    {
        auto slider = std::make_unique<TickUtils::ParameterSliderItem> (tickProcessor.getAPVTS(), IDs::filterCutoff.toString());
        slider->slider.setScrollWheelEnabled (false);
        PopupMenu settings;
        settings.addSectionHeader ("Sync");
        auto& transport = tickProcessor.getState().useHostTransport;
        settings.addItem ("Internal", true, ! transport.get(), [&transport] {
            transport.setValue (false, nullptr);
        });
        const bool canExternal =
#if JUCE_DEBUG
            true
#else
            processor.wrapperType != AudioProcessor::wrapperType_Standalone
#endif
            ;
        settings.addItem ("External (Host)", canExternal, transport.get(), [&transport] {
            transport.setValue (true, nullptr);
        });
#if JUCE_IOS
        settings.addItem ("Ableton Link..", true, tickProcessor.m_link.isLinkConnected(), [this]
                          { tickProcessor.m_link.showSettings (settingsButton, nullptr); });
#endif
        settings.addSeparator();
        PopupMenu preCountMenu;
        auto& preCount = tickProcessor.getState().transport.preCount;
        preCountMenu.addItem ("Off", true, preCount.get() == 0, [&preCount]
                              { preCount.setValue (0, nullptr); });
        preCountMenu.addItem ("1BAR", true, preCount.get() == 1, [&preCount]
                              { preCount.setValue (1, nullptr); });
        preCountMenu.addItem ("2BAR", true, preCount.get() == 2, [&preCount]
                              { preCount.setValue (2, nullptr); });
        preCountMenu.addItem ("3BAR", true, preCount.get() == 3, [&preCount]
                              { preCount.setValue (3, nullptr); });
        settings.addSubMenu ("Pre-Count", preCountMenu, ! tickProcessor.getState().useHostTransport.get());

        PopupMenu viewSubMenu;
        auto& showWaveform = tickProcessor.getState().showWaveform;
        viewSubMenu.addItem ("Always Show Waveform", true, showWaveform.get(), [&showWaveform]
                             { showWaveform.setValue (! showWaveform.get(), nullptr); });
        auto& showBeatNumber = tickProcessor.getState().showBeatNumber;
        viewSubMenu.addItem ("Show Beat Number", true, showBeatNumber.get(), [&showBeatNumber]
                             { showBeatNumber.setValue (! showBeatNumber.get(), nullptr); });
        auto& isVertical = tickProcessor.getState().isVertical;
        jassert (performView);
        auto& performViewRef = *performView;
        viewSubMenu.addItem ("Vertical Layout", true, isVertical.get(), [&isVertical, &performViewRef]
                             {
            isVertical.setValue (! isVertical.get(), nullptr);
            performViewRef.resized(); });
        settings.addSubMenu ("View", viewSubMenu);
        settings.addSeparator();
        settings.addSectionHeader ("Low-Pass Filter");
        settings.addCustomItem (222, std::move (slider));
        auto gainSlider = std::make_unique<TickUtils::ParameterSliderItem> (tickProcessor.getAPVTS(), IDs::masterGain.toString());
        gainSlider->slider.setScrollWheelEnabled (false);
        settings.addSectionHeader ("Master Gain");
        settings.addCustomItem (223, std::move (gainSlider));
        settings.addSeparator();
#if ! JUCE_IOS && ! JUCE_ANDROID
        // for standalone props needs to come from it.
        auto props = standaloneProps != nullptr ? standaloneProps : appProperties.getUserSettings();
        const auto openGLDefault =
#if JUCE_MAC
            false
#else
            true
#endif
            ;
        const auto useOpenGL = props->getBoolValue ("opengl", openGLDefault);
        settings.addItem ("OpenGL Renderer", true, useOpenGL, [this, useOpenGL, props] {
            props->setValue ("opengl", ! useOpenGL);
            // this is redundant / do nothing on standalone.
            appProperties.getUserSettings()->saveIfNeeded();
            juce::NativeMessageBox::showMessageBoxAsync (MessageBoxIconType::InfoIcon, "Graphic Renderer Changed", "Please re-open UI to apply new renderer.");
        });
#endif
        settings.addSeparator();
        settings.addItem ("About", [this] {
            aboutView->setVisible (true);
        });

        auto sideBarContent = std::make_unique<jux::ListBoxMenu>();
        sideBarContent->setMenuFromPopup (std::move (settings));
        sideBarContent->setOnRootBackToParent ([this]
                                               { sidePanel.showOrHide (false); });
        sideBarContent->setShouldCloseOnItemClick (true, [this]
                                                   { sidePanel.showOrHide (false); });
        sidePanel.setContent (sideBarContent.release());
        sidePanel.showOrHide (! sidePanel.isPanelShowing());
    };

    addAndMakeVisible (mainArea);

    samplesPaint = std::make_unique<SamplesPaint> (tickProcessor.getTicks());

    samplesView = std::make_unique<ManageSamplesView> (*samplesPaint, tickProcessor.getTicks());
    samplesView->closeButton.onClick = [&state]
    {
        state.view.showEditSamples = false;
    };
    mainArea.addChildComponent (*samplesView);

    topBar.leftButton.setAccessible (false);
    topBar.rightButton.setAccessible (false);
    topBar.centerLabel.getTextValue().referTo (state.presetName.getPropertyAsValue());
    topBar.centerLabel.onClick = [this]
    {
        auto& showPresetValue = tickProcessor.getState().view.showPresetsView;
        const bool value = showPresetValue.getValue();
        showPresetValue.setValue (! value);
    };
    addAndMakeVisible (topBar);
    settingsButton.setTitle ("Settings");
    settingsButton.toFront (false);
    editModeButton.toFront (false);

    bottomBar.transportButton.getToggleStateValue().referTo (state.transport.isPlaying.getPropertyAsValue());
    // bottomBar add on resize

    performView.reset (new PerformView (tickProcessor.getState(), tickProcessor.getTicks(), *samplesPaint));
    mainArea.addAndMakeVisible (*performView);

    presetsView.reset (new PresetsView (tickProcessor.getState(), tickProcessor.getTicks()));
#if JUCE_IOS
    presetsView->canSharePresets = tickProcessor.wrapperType != tickProcessor.wrapperType_AudioUnitv3;
#endif
    addAndMakeVisible (*presetsView);

    sidePanelArea.setAlwaysOnTop (true);
    sidePanel.setTitleBarHeight (0);
    sidePanelArea.setWantsKeyboardFocus (false);
    sidePanelArea.setFocusContainerType (FocusContainerType::focusContainer);
    sidePanelArea.addAndMakeVisible (sidePanel);
    sidePanelArea.panel = &sidePanel;
    addAndMakeVisible (sidePanelArea);

    aboutView.reset (new AboutView (AudioProcessor::getWrapperTypeDescription (tickProcessor.wrapperType)));
    addChildComponent (aboutView.get());
    aboutView->setAlwaysOnTop (true);

    // register view state notifications
    tickProcessor.getState().view.isEdit.addListener (this);
    tickProcessor.getState().view.showEditSamples.addListener (this);
    tickProcessor.getState().view.showPresetsView.addListener (this);

#if ! JUCE_IOS
    auto useOpenGL = appProperties.getUserSettings()->getBoolValue ("opengl", true);
    if (useOpenGL)
        openglContext.attachTo (*this);
#endif

#if JUCE_ANDROID
    RuntimePermissions::request (
        TickUtils::canUseNewerAndroidFileAPI() ? RuntimePermissions::readExternalStorage : RuntimePermissions::writeExternalStorage,
        [this] (bool wasGranted)
        {
            if (! wasGranted)
            {
                juce::AlertWindow::showMessageBoxAsync (
                    juce::AlertWindow::AlertIconType::WarningIcon,
                    "No Read/Write Permission",
                    JucePlugin_Desc " Requires Read/Write Permission in order to import/export presets and samples.\nUntil enabled all import/export features will be disabled.");
            }
        });
#endif

    if (! TickSplash::didShowSplashOnce)
        splash.reset (new TickSplash (*this));

    setResizable (true, true);
    //    375 x 667 iPhone 6
#if JUCE_WINDOWS || JUCE_MAC || JUCE_LINUX
    const auto size = VariantConverter<ViewDiemensions>::fromVar (state.view.windowSize.getValue());
    setSize (size.x, size.y);
    setResizeLimits (280, 260, 2048, 4096);
#else
    Desktop::getInstance().setScreenSaverEnabled (false);
    setSize (375, 667);
#endif

    startTimerHz (50);
}

TickAudioProcessorEditor::~TickAudioProcessorEditor()
{
    // unregister view state notifications
    tickProcessor.getState().view.isEdit.removeListener (this);
    tickProcessor.getState().view.showEditSamples.removeListener (this);
    tickProcessor.getState().view.showPresetsView.removeListener (this);
    juce::LookAndFeel::setDefaultLookAndFeel (nullptr);
}

//==============================================================================
TickAudioProcessorEditor::Background::Background()
{
    bgImage = juce::Drawable::createFromImageData (BinaryData::background_png, BinaryData::background_pngSize);
    bgImage->setAlpha (0.6f);
    addAndMakeVisible (bgImage.get());
}

void TickAudioProcessorEditor::Background::paintOverChildren (juce::Graphics& g)
{
    g.setColour (Colours::white.withAlpha (0.1f));
    g.drawHorizontalLine (separatorLineY, 0, getWidth());
}

void TickAudioProcessorEditor::Background::resized()
{
    bgImage->setTransformToFit (getLocalBounds().toFloat(), RectanglePlacement::stretchToFit);
}

void TickAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (TickLookAndFeel::Colours::backgroundColour);
}

void TickAudioProcessorEditor::parentHierarchyChanged()
{
#if JUCE_IOS
    // safe area only valid after view is visible.
    resized();
#endif
}

void TickAudioProcessorEditor::resized()
{
    const bool transportOnTopBar = tickProcessor.wrapperType == TickAudioProcessor::wrapperType_AudioUnitv3 || (getWidth() >= 375 && getHeight() <= 260);
    // brute force removal before re-adding
    topBar.removeChildComponent (&bottomBar);
    removeChildComponent (&bottomBar);

#if ! JUCE_IOS || ! JUCE_ANDROID
    tickProcessor.getState().view.windowSize.setValue (String (getWidth()) + "," + String (getHeight()));
#endif
    auto safeArea = Desktop::getInstance().getDisplays().getPrimaryDisplay()->safeAreaInsets;
    constexpr auto notchSafeSides = 32; // we are always being safe...
    const auto isRotatedAndNeeded =
#if JUCE_IOS || JUCE_ANDROID
        (Desktop::getInstance().getCurrentOrientation() & (Desktop::DisplayOrientation::rotatedClockwise | Desktop::DisplayOrientation::rotatedAntiClockwise)) && ! SystemStats::getDeviceDescription().contains ("iPad")
#else
        false
#endif
        ;
    const auto safeTop = safeArea.getTop();
    const auto safeBottom = safeArea.getBottom() + safeTop > 20 ? 40 : 0;
    const auto safeLeft = isRotatedAndNeeded ? notchSafeSides : 0;
    const auto safeRight = safeLeft;
    const auto availableArea = getLocalBounds().withTrimmedBottom (safeBottom).withTrimmedTop (safeTop).withTrimmedLeft (safeLeft).withTrimmedRight (safeRight);
    auto topArea = availableArea;
    headerArea.setBounds (topArea.removeFromTop (TickLookAndFeel::toolbarHeight));
    background.separatorLineY = headerArea.getBottom() + 1;

    sidePanelArea.setBounds (availableArea);

    if (! transportOnTopBar)
    {
        addAndMakeVisible (bottomBar);
        bottomBar.setBounds (topArea.removeFromBottom (TickLookAndFeel::toolbarHeight));
    }
    editModeButton.setBounds (headerArea.getBounds().removeFromRight (60));
    samplesButton.setBounds (headerArea.getBounds().removeFromLeft (headerArea.getHeight()).reduced (TickLookAndFeel::reducePixels));

    settingsButton.setBounds (samplesButton.getBounds().reduced (TickLookAndFeel::reducePixels));
    mainArea.setBounds (topArea);

    background.setBounds (getLocalBounds());
    auto performViewArea = mainArea.getLocalBounds();
    topBar.extendedTopBar = transportOnTopBar;
    topBar.setBounds (headerArea.getBounds());
    if (transportOnTopBar)
    {
        topBar.addAndMakeVisible (bottomBar);
        bottomBar.setBounds (topBar.extendedBarArea);
    }
    performView->setBounds (performViewArea);
    if (samplesView->isVisible())
        samplesView->setBounds (performView->getBounds());
    presetsView->setBounds (mainArea.getLocalBounds().translated (0, (bool) tickProcessor.getState().view.showPresetsView.getValue() == true ? 0 : getHeight()));
    aboutView->setBounds (getLocalBounds());
}

bool TickAudioProcessorEditor::keyPressed (const juce::KeyPress& key)
{
    if (tickProcessor.wrapperType == AudioProcessor::wrapperType_Standalone && ! tickProcessor.getState().useHostTransport.get())
    {
        if (key.getKeyCode() == juce::KeyPress::spaceKey)
        {
            auto& transport = bottomBar.transportButton;
            transport.setToggleState (! transport.getToggleState(), dontSendNotification);
            return true;
        }
        if (key.getKeyCode() == juce::KeyPress::upKey)
        {
            auto& bpmVal = tickProcessor.getState().transport.bpm;
            bpmVal.setValue (bpmVal.get() + 1, nullptr);
            return true;
        }
        if (key.getKeyCode() == juce::KeyPress::downKey)
        {
            auto& bpmVal = tickProcessor.getState().transport.bpm;
            bpmVal.setValue (bpmVal.get() - 1, nullptr);
            return true;
        }
    }
    return false;
}

void TickAudioProcessorEditor::valueChanged (juce::Value& value)
{
    auto& state = tickProcessor.getState();
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
            if (static_cast<bool> (state.view.showPresetsView.getValue()) == true)
                state.view.showPresetsView.setValue (false);
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
        const auto baseBounds = performView->getBounds();
        samplesView->setBounds (value.getValue() ? baseBounds.translated (0, getHeight()) : baseBounds);
        const auto to = value.getValue() ? baseBounds : baseBounds.translated (0, mainArea.getHeight());
        juce::Desktop::getInstance().getAnimator().animateComponent (samplesView.get(), to, 1.0f, 200, false, 1.0, 1.0);
    }
    else if (value.refersToSameSourceAs (state.view.showPresetsView))
    {
        settingsButton.setAccessible (! value.getValue());
        editModeButton.setAccessible (! value.getValue());
        samplesView->setAccessible (! value.getValue());
        performView->setAccessible (! value.getValue());
        const auto safeBounds = topBar.getBounds().withBottom (getLocalBounds().getBottom() - (topBar.extendedTopBar ? 0 : bottomBar.getHeight()));
        presetsView->setBounds (value.getValue() ? getLocalBounds().translated (0, getHeight()) : safeBounds);
        const auto to = value.getValue() ? safeBounds : getLocalBounds().translated (0, getHeight());
        juce::Desktop::getInstance().getAnimator().animateComponent (presetsView.get(), to, 1.0f, 200, false, 1.0, 1.0);
        presetsView->toFront (false);
        topBar.centerLabel.setColour (juce::Label::ColourIds::textColourId, (bool) value.getValue() == true ? TickLookAndFeel::Colours::mint : juce::Colours::white);
    }
}

void TickAudioProcessorEditor::timerCallback()
{
    const bool useHostTransport = tickProcessor.getState().useHostTransport.get();
    const int preCount = tickProcessor.getState().transport.preCount.get();
    performView->update (tickProcessor.getCurrentBeatPos());
    bottomBar.transportButton.setVisible (! useHostTransport);
    bottomBar.preCountIndicator.setVisible (! useHostTransport && preCount > 0);
    bottomBar.preCountIndicator.setButtonText (String (preCount) + "BAR");
#if JUCE_IOS
    const auto link = tickProcessor.m_link.isLinkConnected();
    bottomBar.transportButton.setColour (juce::DrawableButton::backgroundColourId, link ? TickLookAndFeel::Colours::mint : Colours::transparentBlack);
    bottomBar.transportButton.setColour (juce::DrawableButton::backgroundOnColourId, link ? TickLookAndFeel::Colours::mint : Colours::transparentBlack);
#endif
    bottomBar.transportPosition.setVisible (useHostTransport);
    performView->setTapVisibility (! useHostTransport);
    if (useHostTransport)
    {
        bottomBar.transportPosition.setText (TickUtils::generateTimecodeDisplay (tickProcessor.playheadPosition_), dontSendNotification);
    }
    else if (isVisible() && isShowing())
    {
        // When plug-in is self controlled, we want spacebar to work
        grabKeyboardFocus();
    }
}

void TickAudioProcessorEditor::initAppProperties()
{
    // this is copied from juce standalone filter
    // we 'reuse' this for our opengl toggle.
    PropertiesFile::Options options;

    options.applicationName = JucePlugin_Name;
    options.filenameSuffix = ".settings";
    options.osxLibrarySubFolder = "Application Support";
#if JUCE_LINUX || JUCE_BSD
    options.folderName = "~/.config";
#else
    options.folderName = "";
#endif

    appProperties.setStorageParameters (options);
}
