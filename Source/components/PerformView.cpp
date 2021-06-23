/*
  ==============================================================================

    PerformView.cpp
    Created: 27 Sep 2020 5:53:58pm
    Author:  Tal Aviram

  ==============================================================================
*/

#include "PerformView.h"
#include "LookAndFeel.h"

#include "EditBeatView.h"
#include "utils/SamplesPaint.h"

PerformView::PerformView (TickSettings& stateToLink, TicksHolder& ticksToLink, SamplesPaint& samplesPaint)
    : state (stateToLink), ticks (ticksToLink), samplesPaint (samplesPaint), editView (std::make_unique<EditBeatView> (state, ticks))
{
    using namespace juce;

    setInterceptsMouseClicks (false, true); // needed for top bar to work
    viewport.setScrollBarsShown (true, false);
    viewport.setViewedComponent (&beatsView);
    addAndMakeVisible (viewport);

    topBar.tempo.setText (state.transport.bpm.getPropertyAsValue().toString(), dontSendNotification);
    topBar.tempo.getTextValue().referTo (state.transport.bpm.getPropertyAsValue());
    topBar.num.getTextValue().referTo (state.transport.numerator.getPropertyAsValue());
    topBar.num.setText (juce::String (state.transport.numerator.get()), dontSendNotification);
    topBar.denum.getTextValue().referTo (state.transport.denumerator.getPropertyAsValue());
    topBar.denum.setText (juce::String (state.transport.denumerator.get()), dontSendNotification);
    addAndMakeVisible (topBar);

    beatsView.addMouseListener (this, false);
    topBar.tapMode.addMouseListener (this, true);
    // draggable labels...
    tempoDrag.onStep = [this] (bool isUp) {
        const float bpm = state.transport.bpm.get();
        const int step = isUp ? +1 : -1;
        state.transport.bpm.setValue (bpm + step, nullptr);
    };

    numDrag.onStep = [this] (bool isUp) {
        const int num = state.transport.numerator.get();
        const int step = isUp ? +1 : -1;
        state.transport.numerator.setValue (jlimit (1, TickSettings::kMaxBeatAssignments, num + step), nullptr);
    };

    denumDrag.onStep = [this] (bool isUp) {
        const int denum = state.transport.denumerator.get();
        const int step = isUp ? +1 : -1;
        state.transport.denumerator.setValue (jlimit (1, 128, denum + step), nullptr);
    };

    topBar.tempo.addMouseListener (this, true);
    topBar.num.addMouseListener (this, true);
    topBar.denum.addMouseListener (this, true);

    editView->setAlwaysOnTop (true);
    addChildComponent (editView.get());

    juce::Desktop::getInstance().getAnimator().addChangeListener (this);
}

PerformView::~PerformView()
{
    juce::Desktop::getInstance().getAnimator().removeChangeListener (this);
}

void PerformView::selectionChanged (const int index)
{
    // this actually supports multi-selection but unused for this UX
    std::vector<int> newSelection { index };
    for (auto idx = 0; idx < beats.size(); idx++)
    {
        jassert (idx < 64);
        beats[idx]->isSelected = idx == index;
    }
    editView->updateSelection (newSelection);
}

void PerformView::resized()
{
    constexpr auto editViewHeight = 200;
    auto area = getLocalBounds();

    topBar.setBounds (area.removeFromTop (TickLookAndFeel::barHeight));
    if (isEditMode && editView->isVisible() && editView->getBottom() == getBottom() + editViewHeight)
        area.removeFromBottom (editViewHeight);
    viewport.setBounds (area);

    beatsInRow = juce::jlimit (1, 8, juce::jmin (state.transport.numerator.get(), state.transport.denumerator.get())); //std::floor<int> (area.getWidth() / (beatSize + kMargin));
    const auto beatSize = std::floor<int> ((area.getWidth() - 2 * kMargin) / beatsInRow) - 2 * kMargin;
    const auto numOfBeats = state.transport.numerator.get();
    beatsView.setBounds (area.withHeight (std::max<int> (area.getHeight(), std::ceil (numOfBeats / beatsInRow) * (beatSize + 2 * kMargin) + kMargin)).withWidth (area.getWidth() - viewport.getScrollBarThickness()));

    juce::FlexBox fb (juce::FlexBox::Direction::row, juce::FlexBox::Wrap::wrap, juce::FlexBox::AlignContent::flexStart, juce::FlexBox::AlignItems::center, juce::FlexBox::JustifyContent::flexStart);
    for (auto& beat : beats)
    {
        juce::FlexItem newItem (*beat);
        fb.items.add (newItem.withFlex (1.0f)
                          .withMinHeight (beatSize)
                          .withMinWidth (beatSize)
                          .withMaxHeight (beatSize)
                          .withMaxWidth (beatSize)
                          .withMargin (juce::FlexItem::Margin (kMargin)));
    }
    fb.performLayout (beatsView.getLocalBounds());
}

void PerformView::update (double currentPos)
{
    const auto numOfBeats = state.transport.numerator.get();
    const auto denumrator = state.transport.denumerator.get();
    if (numOfBeats != beats.size() || beatsInRow != denumrator)
    {
        beats.clear();
        for (auto beat = 0; beat < numOfBeats; beat++)
        {
            auto beatView = std::make_unique<BeatView> (*this, beat);
            beatView->setEnabled (isEditMode);
            beats.push_back (std::move (beatView));
            beatsView.addAndMakeVisible (beats.back().get());
        }
        resized();
    }
    const auto beatPosExclusive = std::max<double> (0, currentPos - 1.0);
    const auto currentBeat = floor (beatPosExclusive);
    const auto barPos = beatPosExclusive - currentBeat;
    for (auto num = 0; num < numOfBeats; num++)
    {
        auto& beat = *beats[num];
        beat.isOn = num < currentBeat;
        beat.isCurrent = num == currentBeat;
        beat.relativePos = beat.isCurrent ? barPos : 0.0;
        beat.repaint();
        if (state.transport.isPlaying.get() && ! isEditMode && beat.isCurrent && (viewport.getViewArea().getBottom() < beat.getY() || viewport.getViewArea().getY() > beat.getY()))
        {
            viewport.setViewPosition (0, beat.getY());
        }
    }
    const bool isStandalone = ! state.useHostTransport.get();
    topBar.tempo.setEnabled (isStandalone);
    topBar.num.setEnabled (isStandalone);
    topBar.denum.setEnabled (isStandalone);
}

void PerformView::setEditMode (const bool newMode)
{
    isEditMode = newMode;
    for (auto& beat : beats)
    {
        // clears selection when not in edit mode
        if (! isEditMode)
        {
            beat->isSelected = false;
            beat->setEnabled (false);
        }

        beat->setEnabled (isEditMode);
    }
    editView->updateSelection ({});
    editView->setVisible (isEditMode);
    juce::Rectangle<int> hidden (0, getBottom(), getWidth(), 200);
    juce::Rectangle<int> showing (0, getBottom() - 200, getWidth(), 200);
    editView->setBounds (isEditMode ? hidden : showing);
    resized();
    juce::Desktop::getInstance().getAnimator().animateComponent (editView.get(), isEditMode ? showing : hidden, 1.0f, 150, true, 1.0, 1.0);
}

PerformView::BeatView::BeatView (PerformView& parent, const int idx)
    : index (idx), owner (parent)
{
    setValue (owner.state.beatAssignments[idx].gain.get());
    getValueObject().referTo (owner.state.beatAssignments[idx].gain.getPropertyAsValue());
    setTextBoxStyle (TextBoxBelow, false, 0, 0);
    setRange (0.0f, 1.0f);
    setSliderStyle (SliderStyle::RotaryHorizontalVerticalDrag);
}

void PerformView::BeatView::paint (juce::Graphics& g)
{
    jassert (index > -1);
    constexpr auto cornerSize = 3.0f;
    auto& assignment = owner.state.beatAssignments[index];
    const auto tickIndex = assignment.tickIdx.get();
    const auto gain = assignment.gain.get();
    const auto gainedColour = TickLookAndFeel::sampleColourPallete[tickIndex].withAlpha (gain);
    const auto bounds = getLocalBounds().toFloat();
    g.setColour (juce::Colours::white.withAlpha (0.3f));
    g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (1.0f), cornerSize, 1.0f);
    g.setColour (juce::Colours::black.withAlpha (0.3f));
    g.fillRoundedRectangle (bounds, cornerSize);

    if (isOn)
    {
        g.setColour (gainedColour);
        g.fillRoundedRectangle (bounds, cornerSize);
    }
    if (isCurrent)
    {
        g.setColour (gainedColour);
        g.fillRoundedRectangle (0, bounds.getY(), getWidth() * relativePos, bounds.getHeight(), cornerSize);
        g.setColour (juce::Colours::white.withAlpha (0.5f));
        g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (1.0f), cornerSize, cornerSize);
    }
    if (isEnabled() && isSelected)
    {
        g.setColour (TickLookAndFeel::sampleColourPallete[tickIndex]);
        g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (1.0f), cornerSize, cornerSize);
        g.setColour (juce::Colours::white);
        g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (1.0f), cornerSize, cornerSize);
    }
    if ((owner.isEditMode || owner.state.showWaveform.get()) && tickIndex < owner.ticks.getNumOfTicks())
        owner.samplesPaint.drawTick (g, getLocalBounds().reduced (10), tickIndex, assignment.gain.get(), juce::Colours::white.withAlpha (isCurrent ? 1.0f : isSelected ? 0.7f : 0.3f));
}

void PerformView::mouseDown (const juce::MouseEvent& e)
{
    if (e.originalComponent == &topBar.tapMode)
    {
        topBar.tapMode.setColour (juce::Label::backgroundColourId, TickLookAndFeel::Colours::wood);
        tapModel.pushTap (e.eventTime);
        if (tapModel.getLastDetectedBPM() > 0)
            state.transport.bpm.setValue (tapModel.getLastDetectedBPM(), nullptr);
        return;
    }

    if (e.originalComponent == &topBar.tempo)
    {
        tempoDrag.dragStep = 0;
        return;
    }

    if (e.originalComponent == &topBar.num)
    {
        numDrag.dragStep = 0;
        return;
    }

    if (e.originalComponent == &topBar.denum)
    {
        denumDrag.dragStep = 0;
        return;
    }
}

void PerformView::mouseUp (const juce::MouseEvent& e)
{
    if (e.originalComponent == &beatsView && isEditMode)
    {
        state.view.isEdit.setValue (false);
        return;
    }

    if (e.originalComponent == &topBar.tapMode)
    {
        topBar.tapMode.setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    }
}

void PerformView::mouseDrag (const juce::MouseEvent& e)
{
    if (e.originalComponent == &topBar.tempo)
    {
        tempoDrag.handleDrag (e);
        return;
    }
    if (e.originalComponent == &topBar.num)
    {
        numDrag.handleDrag (e);
        return;
    }
    if (e.originalComponent == &topBar.denum)
    {
        denumDrag.handleDrag (e);
        return;
    }
}

void PerformView::BeatView::mouseDown (const juce::MouseEvent& e)
{
    if (! isEnabled())
        return;
    owner.selectionChanged (index);
    repaint();
#if ! JUCE_IOS || ! JUCE_ANDROID
    juce::Slider::mouseDown (e);
#endif
}

void setupSigLabel (juce::Label& l)
{
    l.setFont (juce::Font (30.0));
    l.setKeyboardType (juce::TextEditor::VirtualKeyboardType::phoneNumberKeyboard);
    l.setJustificationType (juce::Justification::centred);
    l.onEditorShow = [&l] {
        l.getCurrentTextEditor()->setInputRestrictions (5, "0123456789");
    };
}

PerformView::TopBar::TopBar()
{
    setInterceptsMouseClicks (false, true);
    tempoLabel.setText ("BPM", juce::dontSendNotification);
    tempoLabel.setFont (juce::Font (30.0));
    tempoLabel.setJustificationType (juce::Justification::centred);
    tempo.setKeyboardType (juce::TextEditor::VirtualKeyboardType::numericKeyboard);
    tempo.setFont (juce::Font (30.0));
    tempo.setJustificationType (juce::Justification::centred);
    tempo.onEditorShow = [this] {
        tempo.getCurrentTextEditor()->setInputRestrictions (6, "0123456789.");
        tempo.getCurrentTextEditor()->onTextChange = [this] {
            auto* editor = tempo.getCurrentTextEditor();
            if (editor->getText().length() > 3 && ! editor->getText().contains ("."))
                editor->deleteBackwards (false);
        };
    };
    setupSigLabel (sigDivider);
    setupSigLabel (num);
    setupSigLabel (denum);
    sigDivider.setText ("/", juce::dontSendNotification);

    tapMode.setFont (juce::Font (30.0));
    tapMode.setJustificationType (juce::Justification::centred);
    tapMode.setText ("TAP", juce::dontSendNotification);

    tempo.setEditable (true);
    num.setEditable (true);
    denum.setEditable (true);

    addAndMakeVisible (tempo);
    addAndMakeVisible (tempoLabel);
    addAndMakeVisible (sigDivider);
    addAndMakeVisible (num);
    addAndMakeVisible (denum);
    addAndMakeVisible (tapMode);
}

void PerformView::TopBar::resized()
{
    auto area = getLocalBounds();
    const auto bpmWidth = tempo.getFont().getStringWidth ("999.99") + 10;
    tempo.setBounds (area.removeFromLeft (bpmWidth));
    auto signatureArea = area.removeFromRight (bpmWidth);
    auto sigWidth = juce::roundToInt (bpmWidth * 0.5);
    num.setBounds (signatureArea.removeFromLeft (sigWidth));
    denum.setBounds (signatureArea.removeFromRight (sigWidth));
    sigDivider.setBounds (signatureArea.expanded (30, 0));
    constexpr auto kWidth = 100;
    auto tapBounds = juce::Rectangle<int> (getLocalBounds().getCentreX() - kWidth / 2, 0, kWidth, getHeight());
    tapMode.setBounds (tapBounds.reduced (0, 4));
}

void PerformView::changeListenerCallback (juce::ChangeBroadcaster*)
{
    if (! juce::Desktop::getInstance().getAnimator().isAnimating (editView.get()) && getParentComponent())
    {
        editView->setVisible (isEditMode);
        resized();
    }
}
