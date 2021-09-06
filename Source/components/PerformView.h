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

    void mouseDrag (const juce::MouseEvent&) override;

    // ChangeListener
    void changeListenerCallback (juce::ChangeBroadcaster*) override;

    void setTapVisibility (bool isVisible) { topBar.tapMode.setVisible (isVisible); }

private:
    void selectionChanged (int index, bool propogateToEditView = true);
    int getEditViewHeight();

    struct DragStep
    {
        void handleDrag (const juce::MouseEvent& e)
        {
            const auto newstep = e.getDistanceFromDragStartY();
            ;
            if (std::abs (newstep - dragStep) > 1)
            {
                if (newstep - dragStep > 0)
                    onStep (false);
                else
                    onStep (true);
                dragStep = newstep;
            }
        }
        std::function<void (bool isUp)> onStep {};
        int dragStep { 0 };
    };

    struct TopBar : juce::Component
    {
        TopBar();
        void resized() override;

        juce::Label tempo, num, denum;
        juce::Label tempoLabel, sigDivider;
        juce::Label tapMode;
    } topBar;

    struct BeatView : juce::Slider, juce::FileDragAndDropTarget
    {
        BeatView (PerformView& parent, int index);
        int index = -1;
        bool isCurrent = false;
        bool isOn = false;
        double relativePos = 0.0;
        bool isSelected = false;
        bool hasDraggedItem = false;

        void mouseDown (const juce::MouseEvent&) override;
        void paint (juce::Graphics&) override;

        PerformView& owner;

        bool isInterestedInFileDrag (const juce::StringArray& files) override;
        void filesDropped (const juce::StringArray& files, int x, int y) override;
        void fileDragEnter (const juce::StringArray& files, int x, int y) override;
        void fileDragExit (const juce::StringArray& files) override;
    };

    TickSettings& state;
    TicksHolder& ticks;
    SamplesPaint& samplesPaint;

    juce::Viewport viewport;
    juce::Component beatsView;
    std::unique_ptr<EditBeatView> editView;
    std::atomic<bool> isAnimatingEditView { false };
    TapModel tapModel;

    std::vector<std::unique_ptr<BeatView>> beats;
    int beatsInRow = 4;
    bool isEditMode = false;
    // used for mouse 'stepping'/dragging musical bpm/signature
    DragStep tempoDrag, numDrag, denumDrag;

    static constexpr int kMargin = 4;
};
