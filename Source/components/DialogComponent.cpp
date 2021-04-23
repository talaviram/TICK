#include "DialogComponent.h"
#include "LookAndFeel.h"

using namespace juce;

static juce_wchar getDefaultPasswordChar() noexcept
{
#if JUCE_LINUX
    return 0x2022;
#else
    return 0x25cf;
#endif
}

DialogComponent::DialogComponent (const String& title, const String& message, Component* associatedComponent)
    : justification (Justification::centred), associatedComponent (associatedComponent)
{
    setName (title);
    setAlwaysOnTop (true);
    if (message.isEmpty())
        text = " "; // to force an update if the message is empty
    updateLayout();
    setMessage (message);
}

void DialogComponent::addButton (const String& name,
                                 const int returnValue,
                                 const KeyPress& shortcutKey1,
                                 const KeyPress& shortcutKey2)
{
    auto* b = new TextButton (name, {});
    buttons.add (b);

    b->setWantsKeyboardFocus (true);
    b->setMouseClickGrabsKeyboardFocus (false);
    b->setCommandToTrigger (nullptr, returnValue, false);
    b->addShortcut (shortcutKey1);
    b->addShortcut (shortcutKey2);
    b->onClick = [this, b] { exitAlert (b); };

    Array<TextButton*> buttonsArray (buttons.begin(), buttons.size());
    // TODO: make this less dirty
    auto& lf = dynamic_cast<TickLookAndFeel&> (getLookAndFeel());

    auto buttonHeight = lf.getAlertWindowButtonHeight();
    auto buttonWidths = lf.getWidthsForTextButtons (buttonsArray);

    jassert (buttonWidths.size() == buttons.size());
    int i = 0;

    for (auto* button : buttons)
        button->setSize (buttonWidths[i++], buttonHeight);

    addAndMakeVisible (b, 0);
    updateLayout();
}

void DialogComponent::addToggle (const String& name, const String& text, const bool initialState)
{
    auto* b = new ToggleButton (text);
    b->setName (name);
    b->setToggleState (initialState, dontSendNotification);
    toggles.add (b);
    allComps.add (b);

    b->setWantsKeyboardFocus (true);
    b->setMouseClickGrabsKeyboardFocus (false);

    addAndMakeVisible (b, 0);
    updateLayout();
}

bool DialogComponent::getToggleState (const String& name)
{
    for (auto* t : toggles)
        if (t->getName() == name)
            return t->getToggleState();

    jassertfalse;
    return false;
}

int DialogComponent::getNumButtons() const
{
    return buttons.size();
}

void DialogComponent::exitAlert (Button* button)
{
    if (auto* parent = button->getParentComponent())
        parent->exitModalState (button->getCommandID());
}

void DialogComponent::setMessage (const String& message)
{
    auto newMessage = message.substring (0, 2048);

    if (text != newMessage)
    {
        text = newMessage;
        updateLayout();
        repaint();
    }
}

void DialogComponent::resized()
{
    updateLayout();
}

void DialogComponent::paint (Graphics& g)
{
    auto& lf = dynamic_cast<TickLookAndFeel&> (getLookAndFeel());
    lf.drawDialogComponent (g, *this, textArea, textLayout);

    g.setColour (findColour (AlertWindow::ColourIds::textColourId));
    g.setFont (lf.getAlertWindowFont());

    for (int i = textBoxes.size(); --i >= 0;)
    {
        auto* te = textBoxes.getUnchecked (i);

        g.drawFittedText (textboxNames[i],
                          te->getX(),
                          te->getY() - 14,
                          te->getWidth(),
                          14,
                          Justification::centredLeft,
                          1);
    }
}

void DialogComponent::addTextEditor (const String& name,
                                     const String& initialContents,
                                     const String& onScreenLabel,
                                     const bool isPasswordBox)
{
    auto* ed = new TextEditor (name, isPasswordBox ? getDefaultPasswordChar() : 0);
    ed->setSelectAllWhenFocused (true);
    ed->setEscapeAndReturnKeysConsumed (false);
    textBoxes.add (ed);
    allComps.add (ed);

    ed->setColour (TextEditor::outlineColourId, findColour (ComboBox::outlineColourId));
    ed->setFont (getLookAndFeel().getAlertWindowMessageFont());
    addAndMakeVisible (ed);
    ed->setText (initialContents);
    ed->setCaretPosition (initialContents.length());
    textboxNames.add (onScreenLabel);

    updateLayout();
}

TextEditor* DialogComponent::getTextEditor (const String& nameOfTextEditor) const
{
    for (auto* tb : textBoxes)
        if (tb->getName() == nameOfTextEditor)
            return tb;

    return nullptr;
}

String DialogComponent::getTextEditorContents (const String& nameOfTextEditor) const
{
    if (auto* t = getTextEditor (nameOfTextEditor))
        return t->getText();

    return {};
}

void DialogComponent::updateLayout()
{
    const int titleH = 24;

    auto& lf = getLookAndFeel();
    auto messageFont (lf.getAlertWindowMessageFont());

    auto wid = jmax (messageFont.getStringWidth (text),
                     messageFont.getStringWidth (getName()));

    auto sw = (int) std::sqrt (messageFont.getHeight() * wid);
    auto w = jmin (300 + sw * 2, (int) (getParentWidth() * 0.7f));
    const int edgeGap = 10;
    const int labelHeight = 18;
    int iconSpace = 0;

    AttributedString attributedText;
    attributedText.append (getName(), lf.getAlertWindowTitleFont());

    if (text.isNotEmpty())
        attributedText.append ("\n\n" + text, messageFont);

    attributedText.setColour (findColour (AlertWindow::ColourIds::textColourId));

    {
        attributedText.setJustification (Justification::centredTop);
        textLayout.createLayoutWithBalancedLineLengths (attributedText, (float) w);
    }

    w = jmax (350, (int) textLayout.getWidth() + iconSpace + edgeGap * 4);
    w = jmin (w, (int) (getParentWidth() * 0.7f));

    auto textLayoutH = (int) textLayout.getHeight();
    auto textBottom = 16 + titleH + textLayoutH;
    int h = textBottom;

    int buttonW = 40;

    for (auto* b : buttons)
        buttonW += 16 + b->getWidth();

    w = jmax (buttonW, w);

    h += (textBoxes.size()) * 50;

    if (auto* b = buttons[0])
        h += 20 + b->getHeight();

    const auto tw = getWidth();
    for (auto* t : toggles)
    {
        t->setBounds (0, h, tw, titleH);
        h += titleH;
    }

    for (auto* c : customComps)
    {
        w = jmax (w, (c->getWidth() * 100) / 80);
        h += 10 + c->getHeight();

        if (c->getName().isNotEmpty())
            h += labelHeight;
    }

    w = jmin (w, (int) (getParentWidth() * 0.7f));

    setBounds (getBounds().withHeight (h));

    textArea.setBounds (edgeGap, edgeGap, w - (edgeGap * 2), h - edgeGap);

    auto y = (int) (getHeight() * 0.95f);
    if (buttons.size())
    {
        auto idealBtnWidth = roundToInt (getWidth() / buttons.size());
        const auto h = lf.getAlertWindowButtonHeight();
        auto buttonsArea = getLocalBounds().withY (getHeight() - h - 1).withHeight (h);
        for (auto* c : buttons)
        {
            const auto ideal = jmax (lf.getTextButtonWidthToFitText (*c, buttonsArea.getHeight()), idealBtnWidth);
            const auto leftWidth = jmin (ideal, buttonsArea.getWidth());
            c->setBounds (buttonsArea.removeFromLeft (leftWidth));
            if (buttons.getLast() != c)
                buttonsArea.removeFromLeft (1);

            c->toFront (false);
        }
    }
    y = textBottom;

    for (auto* c : allComps)
    {
        h = 22;

        const int tbIndex = textBoxes.indexOf (dynamic_cast<TextEditor*> (c));
        if (tbIndex >= 0 && textboxNames[tbIndex].isNotEmpty())
            y += labelHeight;
        if (customComps.contains (c))
        {
            if (c->getName().isNotEmpty())
                y += labelHeight;

            c->setTopLeftPosition (proportionOfWidth (0.1f), y);
            h = c->getHeight();
        }
        else if (textBlocks.contains (c))
        {
            c->setTopLeftPosition ((getWidth() - c->getWidth()) / 2, y);
            h = c->getHeight();
        }
        else
        {
            c->setBounds (proportionOfWidth (0.1f), y, proportionOfWidth (0.8f), h);
        }

        y += h + 10;
    }

    setWantsKeyboardFocus (getNumChildComponents() == 0);
}

bool DialogComponent::containsAnyExtraComponents() const
{
    return allComps.size() > 0;
}

void DialogComponent::showOkCancelDialog (
    const String& title,
    const String& message,
    const String& button1Text,
    const String& button2Text,
    Component* associatedComponent,
    ModalComponentManager::Callback* callback)
{
    jassert (associatedComponent);
    auto* aw = new DialogComponent (title,
                                    message,
                                    associatedComponent);
    aw->addButton (TRANS ("Ok"), 1, KeyPress (KeyPress::returnKey));
    aw->addButton (TRANS ("Cancel"), 0, KeyPress (KeyPress::escapeKey));
    const auto dialogHeight = jmin (350, roundToInt (associatedComponent->getHeight() * 0.3));
    Rectangle<int> bounds = { 0, 0, roundToInt (associatedComponent->getWidth() * 0.8), dialogHeight };

    Justification justification (Justification::centredTop);
    aw->setBounds (justification.appliedToRectangle (bounds, associatedComponent->getLocalBounds()));
    associatedComponent->addAndMakeVisible (aw);

    aw->enterModalState (true, callback, true);
}
