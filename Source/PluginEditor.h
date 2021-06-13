/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

#include "components/LookAndFeel.h"
#include "components/PerformView.h"
#include "components/TopBar.h"
#include "utils/TickSplash.h"

//==============================================================================
/**
*/
class PresetsView;
class ManageSamplesView;
class SamplesPaint;

namespace jux
{
    class ListBoxMenu;
}

class TickAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer, private juce::Value::Listener
{
public:
    TickAudioProcessorEditor (TickAudioProcessor&);
    ~TickAudioProcessorEditor();

    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

    void parentHierarchyChanged() override;

    bool keyPressed (const juce::KeyPress&) override;

#if JUCE_DEBUG
    void paintOverChildren (juce::Graphics& g) override
    {
        g.setColour (juce::Colours::green);
        g.drawFittedText (juce::String (processor.getSampleRate()), getLocalBounds(), juce::Justification::bottom, 1);
        if (processor.getState().isDirty)
        {
            g.setColour (juce::Colours::red);
            g.drawFittedText ("DIRTY", getLocalBounds(), juce::Justification::centred, 1);
        }
    }
#endif

    // Value::Listener
    void valueChanged (juce::Value&) override;

private:
    void initAppProperties();

    class Background : public juce::Component
    {
    public:
        Background();
        void paintOverChildren (juce::Graphics&) override;
        void resized() override;
        int separatorLineY { 0 };

    private:
        std::unique_ptr<juce::Drawable> bgImage;
    } background;

    juce::Label headerName;

    TickLookAndFeel lookAndFeel;

    Component mainArea;
    juce::TextButton editModeButton;
    juce::Label headerArea, tabBar;
    juce::DrawableButton samplesButton;
    juce::DrawableButton settingsButton;

    juce::LookAndFeel_V4 plainLaF;

    TopBar topBar;

    struct BottomBar : juce::Component
    {
        BottomBar();
        void resized() override;
        juce::DrawableButton transportButton;
        juce::DrawableButton syncIndicator;
        juce::Label transportPosition;
    } bottomBar;

    // views
    std::unique_ptr<PresetsView> presetsView;
    std::unique_ptr<PerformView> performView;
    std::unique_ptr<ManageSamplesView> samplesView;

    std::unique_ptr<SamplesPaint> samplesPaint;

    std::unique_ptr<Component> aboutView;
    // Settings
    juce::ApplicationProperties appProperties;
    juce::OpenGLContext openglContext;

    TickAudioProcessor& processor;
    std::unique_ptr<TickSplash> splash;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TickAudioProcessorEditor)
};
