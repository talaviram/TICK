// DialogComponent - takeoff from juce Dialog to allow Tick use cases.
#pragma once

#include "JuceHeader.h"

class DialogComponent : public juce::Component
{
public:
    DialogComponent (const juce::String& title, const juce::String& message, Component* associatedComponent = nullptr);

    void addTextEditor (const juce::String& name,
                        const juce::String& initialContents,
                        const juce::String& onScreenLabel,
                        const bool isPasswordBox);

    juce::TextEditor* getTextEditor (const juce::String& nameOfTextEditor) const;
    juce::String getTextEditorContents (const juce::String& nameOfTextEditor) const;

    void addToggle (const juce::String& name, const juce::String& text, bool initialState);
    bool getToggleState (const juce::String& name);

    struct DialogResult
    {
        int clickedButton;
        juce::String textBoxContent;
    };

    void addButton (const juce::String& name, const int returnValue, const juce::KeyPress& shortcutKey1 = juce::KeyPress(), const juce::KeyPress& shortcutKey2 = juce::KeyPress());

    void setMessage (const juce::String& message);
    void setJustification (juce::Justification);

    void paint (juce::Graphics& g) override;
    void resized() override;

    /** Returns the number of buttons that the window currently has. */
    int getNumButtons() const;

    //==============================================================================
    /** Returns true if the window contains any components other than just buttons.*/
    bool containsAnyExtraComponents() const;

    static void showOkCancelDialog (
        const juce::String& title,
        const juce::String& message,
        const juce::String& button1Text,
        const juce::String& button2Text,
        Component* associatedComponent,
        juce::ModalComponentManager::Callback* callback);

private:
    void updateLayout();

    void exitAlert (juce::Button* button);
    juce::Rectangle<int> textArea;
    juce::TextLayout textLayout;

    juce::String text;
    juce::Justification justification;

    juce::ComponentBoundsConstrainer constrainer;
    juce::OwnedArray<juce::TextButton> buttons;
    juce::OwnedArray<juce::ToggleButton> toggles;
    juce::OwnedArray<juce::TextEditor> textBoxes;

    Component* const associatedComponent;
    juce::Array<juce::Component*> customComps;
    juce::OwnedArray<juce::Component> textBlocks;
    juce::Array<juce::Component*> allComps;
    juce::StringArray textboxNames, comboBoxNames;
    bool escapeKeyCancels = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DialogComponent)
};
