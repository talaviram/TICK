/*
  ==============================================================================

    ManageSamplesView.h
    Created: 2 Oct 2020 12:44:40pm
    Author:  Tal Aviram

  ==============================================================================
*/

#pragma once

#include "PluginProcessor.h"
#include "utils/SamplesPaint.h"

namespace jux
{
    class ListBoxMenu;
}

class ManageSamplesView
    : public juce::Component,
      juce::ChangeListener
{
public:
    ManageSamplesView (SamplesPaint&, TicksHolder& ticks);

    void resized() override;
    void paint (juce::Graphics& g) override;

    struct CloseButton : juce::Button
    {
        CloseButton();
        void paintButton (juce::Graphics&, bool, bool);
    } closeButton;
    // TODO: enforce in model!
    static constexpr auto kMaxSamples = 16;

    juce::Value viewState;

    // single selection, no multiple!
    void updateSelection (int index);

private:
    juce::NormalisableRange<float> heightNorm;
    void changeListenerCallback (juce::ChangeBroadcaster*) override;

    void invalidateSampleCells();

    struct EditSection : juce::Component, juce::Value::Listener
    {
        EditSection();
        void setSelectedSample (Tick*);
        void resized() override;
        void paint (juce::Graphics&) override;

        // Value::Listener
        void valueChanged (juce::Value&) override;

        Tick* selectedTick { nullptr };
        juce::Label name;
        juce::Slider volume;
        juce::Slider range;
    };

    TicksHolder& ticks;
    int lastSelection { 0 };

    EditSection editSection;

    SamplesPaint& samplesPaint;
};
