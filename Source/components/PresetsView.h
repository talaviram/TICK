#pragma once

#include "PluginProcessor.h"

namespace jux
{
    class ListBoxMenu;
}

class TopBar;

class PresetsView : public juce::Component, private juce::ChangeListener
{
public:
    PresetsView (TickSettings& state, TicksHolder& ticksRef);
    ~PresetsView() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void backToParent();
    void refresh();
    const juce::File& getCurrentRoot() const;
    juce::File getFileForIndex (int) const;
    void savePreset (juce::File, bool discardTransport);

    // Used to avoid share on iOS AUv3
    // Due to JUCE Share not properly supporting sandbox
    bool canSharePresets = true;

private:
    TickSettings& state;
    TicksHolder& ticks;

    std::unique_ptr<juce::DirectoryContentsList> directoryContents;
    struct PresetData
    {
        bool isFolder;
        bool containsTime;
        juce::String name;
        float bpm;
        juce::String uuid;
        int numerator, denumerator;
        juce::DirectoryContentsList::FileInfo info;
    };

    std::unique_ptr<juce::WildcardFileFilter> filter;
    juce::TimeSliceThread timesliceThread;
    std::unique_ptr<juce::ImageComponent> transitionBackground;

    void transitionList();

    class PresetModel : public juce::ListBoxModel
    {
    public:
        PresetModel (PresetsView& parent);

        int getNumRows() override;

        void listBoxItemClicked (int row, const juce::MouseEvent&) override;
        void paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;

        juce::Component* refreshComponentForRow (int rowNumber, bool isRowSelected, juce::Component* existingComponentToUpdate) override;

    private:
        PresetsView& owner;
    } presetModel;

    struct PresetView : public juce::Component
    {
        PresetView();
        int index { -1 };
        bool isSelected { false };
        bool isReady { false };
        void paint (juce::Graphics& g) override;
        void resized() override;

        juce::DrawableButton moreOptions;
        juce::Label name;
        PresetsView* presetsView { nullptr };
        PresetData data;
    };

    juce::ListBox list;
    juce::FileChooser fileChooser;

    std::unique_ptr<TopBar> topBar;

    bool isRoot();

    void setDialogBounds (juce::Component& dialog, juce::Rectangle<int> parentBounds);

    void queryPreset (juce::File, PresetData& dataToFill);

    void createFolder();
    void deleteFileWithConfirmation (juce::File);
    void renamePreset (juce::File& file, const juce::String& newName, bool discardTransport);
    void loadPreset (juce::File);

    // juce::ChangeListener
    void changeListenerCallback (juce::ChangeBroadcaster*) override;
};
