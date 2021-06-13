/*
  ==============================================================================

    PerformView.h
    Created: 27 Sep 2020 5:53:58pm
    Author:  Tal Aviram

  ==============================================================================
*/

#pragma once

#include "PluginProcessor.h"
#include "model/TapModel.h"

class EditBeatView;
class SamplesPaint;

class PerformView
    : public juce::Component,
      public juce::ChangeListener
{
public:
    PerformView (TickSettings& stateToLink, TicksHolder& ticks, SamplesPaint& samplesPaint);
    ~PerformView();
    void resized() override;
    void setEditMode (bool);
    void update (double currentPos);

    void mouseDown (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;

    // ChangeListener
    void changeListenerCallback (juce::ChangeBroadcaster*) override;

private:
    void selectionChanged (int index);

    struct TopBar : juce::Component
    {
        TopBar();
        void resized() override;

        juce::Label tempo, num, denum;
        juce::Label tempoLabel, sigDivider;
        juce::Label tapMode;
    } topBar;

    struct BeatView : juce::Slider
    {
        BeatView (PerformView& parent, int index);
        int index = -1;
        bool isCurrent = false;
        bool isOn = false;
        double relativePos = 0.0;
        bool isSelected = false;

        void mouseDown (const juce::MouseEvent&) override;
        void paint (juce::Graphics&) override;

        PerformView& owner;
    };

    TickSettings& state;
    TicksHolder& ticks;
    SamplesPaint& samplesPaint;

    juce::Viewport viewport;
    juce::Component beatsView;
    std::unique_ptr<EditBeatView> editView;
    TapModel tapModel;

    std::vector<std::unique_ptr<BeatView>> beats;
    int beatsInRow = 4;
    bool isEditMode = false;

    static constexpr int kMargin = 4;
};
