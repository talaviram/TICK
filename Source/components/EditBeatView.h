/*
  ==============================================================================

    EditBeatView.h
    Created: 2 Oct 2020 9:05:55am
    Author:  Tal Aviram

  ==============================================================================
*/

#pragma once

#include "PluginProcessor.h"

namespace jux
{
    class ListBoxMenu;
}

class EditBeatView : public juce::Component, juce::ChangeListener
{
public:
    EditBeatView (TickSettings& state, TicksHolder& ticksRef);
    ~EditBeatView(); // default

    void resized() override;
    void paint (juce::Graphics&) override;
    void visibilityChanged() override;

    void updateSelection (const std::vector<int>& selection);

    std::function<void (std::vector<int>& updateSelection, BeatAssignment newAssignment)> onBeatUpdate {};

private:
    void changeListenerCallback (juce::ChangeBroadcaster*) override;
    juce::PopupMenu getAddSamplesMenu (int indexToReplace = -1);
    void setNewImportedSample (int replaceIndex, std::unique_ptr<Tick> newTick);

    struct SamplesModel : juce::ListBoxModel
    {
        SamplesModel (EditBeatView&);
        int getNumRows() override;
        void paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
        juce::Component* refreshComponentForRow (int rowNumber, bool isRowSelected, Component* existingComponentToUpdate) override;
        void listBoxItemClicked (int row, const juce::MouseEvent&) override;
        bool isLastRow (int row) const;
        EditBeatView& owner;

        class SampleOption : public juce::Component
        {
        public:
            SampleOption (EditBeatView&);
            void mouseDown (const juce::MouseEvent&) override;
            void paint (juce::Graphics&) override;
            bool hitTest (int x, int y) override;
            EditBeatView& owner;
            int row { -1 };
        };
    } model;
    friend class SamplesMode;

    juce::Label hintText;
    juce::FileChooser fileChooser;

    juce::DrawableButton sampleIcon, sampleSelection;
    juce::Label beatLabel;
    juce::Slider beatVolume;
    juce::ListBox samplesList;
    juce::Colour currentColour;
    std::unique_ptr<jux::ListBoxMenu> listboxMenu;

    std::vector<int> selection {};
    TickSettings& state;
    TicksHolder& ticks;
};
