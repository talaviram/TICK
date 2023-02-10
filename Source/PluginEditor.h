/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

#include "components/LookAndFeel.h"
#include "components/PerformView.h"
#include "components/TopBar.h"
#include "components/TransportBar.h"
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
    ~TickAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

    void parentHierarchyChanged() override;

    bool keyPressed (const juce::KeyPress&) override;

#if JUCE_DEBUG
    void paintOverChildren (juce::Graphics& g) override
    {
        g.setColour (juce::Colours::green);
        g.drawFittedText (juce::String (tickProcessor.getSampleRate()), getLocalBounds(), juce::Justification::bottom, 1);
        if (tickProcessor.getState().isDirty)
        {
            g.setColour (juce::Colours::red);
            g.drawFittedText ("DIRTY", getLocalBounds(), juce::Justification::centred, 1);
        }
    }
#endif

    // Value::Listener
    void valueChanged (juce::Value&) override;

    juce::PropertySet* standaloneProps { nullptr };

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
    TransportBar bottomBar;
    SidePanel sidePanel;
    struct SidePanelArea : Component
    {
        bool hitTest (int, int) override
        {
            return panel->isPanelShowing();
        }
        void mouseUp (const juce::MouseEvent&) override
        {
            panel->showOrHide (false);
        }
        SidePanel* panel;
    } sidePanelArea;

    // views
    std::unique_ptr<PresetsView> presetsView;
    std::unique_ptr<PerformView> performView;
    std::unique_ptr<ManageSamplesView> samplesView;

    std::unique_ptr<SamplesPaint> samplesPaint;

    std::unique_ptr<Component> aboutView;
    // Settings
    juce::ApplicationProperties appProperties;
    juce::OpenGLContext openglContext;

    TickAudioProcessor& tickProcessor;
    std::unique_ptr<TickSplash> splash;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TickAudioProcessorEditor)
};
