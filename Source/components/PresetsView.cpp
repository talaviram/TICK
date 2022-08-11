#include "PresetsView.h"
#include "DialogComponent.h"
#include "JUX/components/ListBoxMenu.h"
#include "LookAndFeel.h"
#include "TopBar.h"
#include "utils/UtilityFunctions.h"

static void createFolderCallback (int modalResult, PresetsView* view, juce::Component::SafePointer<DialogComponent> alert)
{
    if (modalResult == 1 && view != nullptr)
    {
        const auto input = alert->getTextEditorContents ("FolderInput");
        if (input.isNotEmpty() && ! input.containsAnyOf ("//\\*"))
        {
            if (! view->getCurrentRoot().getChildFile (input).createDirectory().wasOk())
            {
                juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::AlertIconType::WarningIcon, "Can\'t Create Folder!", "This might be due to invalid folder name or permissions.");
            }
            else
            {
                view->refresh();
            }
        }
        else
        {
            juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::AlertIconType::WarningIcon, "Invalid Input", "Make sure you've used a valid name.");
        }
    }
}

static void savePresetCallback (int modalResult, PresetsView* view, juce::Component::SafePointer<DialogComponent> alert)
{
    if (modalResult == 1 && view != nullptr)
    {
        const auto input = alert->getTextEditorContents ("PresetNameInput");
        auto discardTransport = ! alert->getToggleState ("keepTransport");
        auto newPreset = view->getCurrentRoot().getChildFile (input + TickUtils::kPresetExtension);

        if (newPreset.isDirectory())
        {
            juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::AlertIconType::WarningIcon, "Can\'t Create Preset with that name!", "Preset name conflicts with folder name.");
            return;
        }
        else if (newPreset.existsAsFile())
        {
            DialogComponent::showOkCancelDialog (
                "Overwite Confirmation",
                "Preset exists. Would you like to overwrite it?",
                "Yes",
                "No",
                view->getTopLevelComponent(),
                juce::ModalCallbackFunction::create ([view, newPreset, discardTransport] (int result) {
                    if (result != 0)
                    {
                        // TODO:check it actually overwrite
                        if (newPreset.deleteFile())
                        {
                            view->savePreset (newPreset, discardTransport);
                        }
                        else
                        {
                            jassertfalse;
                            juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::AlertIconType::WarningIcon, "Error", "Failed overwriting preset!");
                        }
                        view->refresh();
                    }
                }));
        }
        else
        {
            view->savePreset (newPreset, discardTransport);
            view->refresh();
        }
    }
}

static void deleteFileCallback (int modalResult, PresetsView* view, juce::Component::SafePointer<DialogComponent> alert)
{
    juce::File file (alert->getProperties().getWithDefault ("filename", {}));
    if (modalResult == 1 && view != nullptr)
    {
        if (file.exists() && ! file.deleteRecursively())
        {
            juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::AlertIconType::WarningIcon, "Can\'t Delete!", "This might be due to permissions. Please retry!");
        }
        else
        {
            view->refresh();
        }
    }
}

PresetsView::PresetsView (TickSettings& stateRef, TicksHolder& ticksRef)
    : state (stateRef), ticks (ticksRef), timesliceThread ("PresetScannerThread"), presetModel (*this), fileChooser ("Import Preset", juce::File::getSpecialLocation (juce::File::userDocumentsDirectory), "*" + juce::String (TickUtils::kPresetExtension), TickUtils::usePlatformDialog()
#if JUCE_IOS
                                                                                                                                                                                                                                                                 ,
                                                                                                                     false,
                                                                                                                     getTopLevelComponent() // AUv3 Sandbox requirement
#endif
                                                                                                        )
{
    using namespace juce;

    // TopBar
    topBar.reset (new TopBar());
    addAndMakeVisible (*topBar);

    {
        auto backImage = Drawable::createFromImageData (BinaryData::arrow_back_ios24px_svg, BinaryData::arrow_back_ios24px_svgSize);
        backImage->replaceColour (Colours::black, Colours::white);
        topBar->leftButton.setImages (backImage.get());
        topBar->leftButton.setDescription ("Back");
        topBar->leftButton.onClick = [this] {
            transitionList();
            backToParent();
        };
        topBar->centerLabel.isOpen = true;
        topBar->centerLabel.setColour (juce::Label::textColourId, TickLookAndFeel::Colours::mint);
        topBar->centerLabel.getTextValue().referTo (state.presetName.getPropertyAsValue());
        topBar->centerLabel.onClick = [this] {
            state.view.showPresetsView.setValue (false);
        };
        topBar->centerLabel.setDescription ("Click to close presets view");
        auto moreImage = Drawable::createFromImageData (BinaryData::more_horiz24px_svg, BinaryData::more_horiz24px_svgSize);
        moreImage->replaceColour (Colours::black, Colours::white);
        topBar->rightButton.setImages (moreImage.get());
        topBar->rightButton.setTitle ("More Presets Options");
        topBar->rightButton.onClick = [this] {
            PopupMenu menu;
            if (isRoot())
            {
                menu.addItem ("New Set", [this] {
                    createFolder();
                });
            }
            menu.addItem ("Create Empty Preset", [this] {
                state.clear();
            });
            menu.addItem ("Save as...", [this] {
                const auto fileToSave = directoryContents->getDirectory().getChildFile (state.presetName.get() + TickUtils::kPresetExtension);
                auto* aw = new DialogComponent ("Save Preset",
                                                juce::String(),
                                                this);
                aw->addTextEditor ("PresetNameInput", fileToSave.getFileNameWithoutExtension(), juce::String(), false);
                aw->addToggle ("keepTransport", "Include Tempo & Meter Data", ! state.useHostTransport.get());
                aw->addButton (TRANS ("Save"), 1, juce::KeyPress (juce::KeyPress::returnKey));
                aw->addButton (TRANS ("Cancel"), 0, juce::KeyPress (juce::KeyPress::escapeKey));
                setDialogBounds (*aw, getLocalBounds());
                addAndMakeVisible (aw);
                juce::Timer::callAfterDelay (100, [=] {
                    if (aw != nullptr)
                    {
                        aw->enterModalState (true,
                                             juce::ModalCallbackFunction::forComponent (savePresetCallback,
                                                                                        this,
                                                                                        juce::Component::SafePointer<DialogComponent> (aw)),
                                             true);
                    }
                });
            });
#if JUCE_IOS || JUCE_ANDROID
                menu.addItem ("Import...", TickUtils::canImport(), false, [this] {
                    fileChooser.launchAsync (FileBrowserComponent::FileChooserFlags::openMode | FileBrowserComponent::FileChooserFlags::canSelectFiles,
                                             [this] (const FileChooser& chooser) {
                                                 if (! chooser.getURLResult().isEmpty())
                                                 {
#if JUCE_ANDROID
                                                     auto stream = AndroidDocument::fromDocument (chooser.getURLResult()).createInputStream();
#else
                                                     auto stream (
                                                         chooser.getURLResult().createInputStream (
                                                             URL::InputStreamOptions (
                                                                 URL::ParameterHandling::inAddress)));
#endif
                                                     jassert (stream != nullptr);
                                                     FileOutputStream output (
                                                         TickUtils::getUserFolder().getChildFile (
                                                             chooser.getURLResult().getFileName()));
                                                     if (output.openedOk())
                                                     {
                                                         output.setPosition (0);
                                                         output.truncate();
                                                         output.writeFromInputStream (*stream,
                                                                                      1024 * 512);
                                                         output.flush();
                                                     }
                                                     if (TickUtils::isValidPreset (output.getFile(),
                                                                                   true))
                                                         refresh();
                                                     else
                                                         output.getFile().deleteFile();
                                                 }
                                             });
                });
#else
            String fileManagerName;
#if JUCE_MAC
            fileManagerName = "Show In Finder...";
#else
            fileManagerName = "Show Folder...";
#endif
            menu.addSeparator();
            menu.addItem (fileManagerName, [] {
                TickUtils::getUserFolder().startAsProcess();
            });
#endif
            menu.showMenuAsync (
                juce::PopupMenu::Options().withMinimumWidth (40).withMaximumNumColumns (3).withParentComponent (this).withTargetComponent (&topBar->rightButton));
            // TODO: iOS Import/Export
        };
    }

    auto userDataFolder = TickUtils::getUserFolder();
#if JUCE_IOS
    if (! TickUtils::getUserFolder().getChildFile ("Factory").isDirectory())
    {
        // TODO: background/async? though it's tiny...
        File::getSpecialLocation (File::SpecialLocationType::currentApplicationFile).getChildFile ("Factory").copyDirectoryTo (TickUtils::getUserFolder().getChildFile ("Factory"));
    }
#elif JUCE_ANDROID
    if (! userDataFolder.isDirectory() || ! TickUtils::getUserFolder().getChildFile ("Factory").isDirectory())
    {
        auto result = TickUtils::getUserFolder().createDirectory();
        if (! result.wasOk())
        {
            juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::AlertIconType::WarningIcon, "Failed to create preset folder!", "Try reinstalling TICK.", "Quit", nullptr, ModalCallbackFunction::create ([] (int) {
                                                        juce::JUCEApplicationBase::getInstance()->quit();
                                                        jassertfalse;
                                                    }));
        }

        ZipFile apkZip (File::getSpecialLocation (File::invokedExecutableFile));
        for (int i = 0; i < apkZip.getNumEntries(); i++)
        {
            if (auto* entry = apkZip.getEntry (i))
            {
                if (entry->filename.endsWithIgnoreCase (".tickpreset"))
                {
                    apkZip.uncompressEntry (i, TickUtils::getUserFolder());
                }
            }
        }
        const auto factoryFolder = TickUtils::getUserFolder().getChildFile ("Factory");
        TickUtils::getUserFolder().getChildFile ("assets").moveFileTo (factoryFolder);
    }
#else
    if (! userDataFolder.isDirectory())
    {
        const auto result = userDataFolder.createDirectory();
        if (! result.wasOk())
        {
            juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::AlertIconType::WarningIcon, "Invalid Preset Folder!", "Try reinstalling TICK.");
            return;
        }
    }
#endif

    timesliceThread.startThread();

    const auto preset = String ("*") + String (TickUtils::kPresetExtension) + ";";
    filter.reset (new WildcardFileFilter (preset, String ("*.*"), "Presets"));
    directoryContents.reset (new DirectoryContentsList (filter.get(), timesliceThread));
    directoryContents->addChangeListener (this);
    directoryContents->setDirectory (userDataFolder, true, true);

    list.setModel (&presetModel);
    list.setRowHeight (50);
    addAndMakeVisible (list);
}

PresetsView::~PresetsView()
{
    directoryContents.reset();
    timesliceThread.stopThread (1000);
}

const juce::File& PresetsView::getCurrentRoot() const
{
    jassert (directoryContents != nullptr);
    return directoryContents->getDirectory();
}

juce::File PresetsView::getFileForIndex (const int index) const
{
    jassert (directoryContents != nullptr);
    return directoryContents->getFile (index);
}

void PresetsView::backToParent()
{
    // fail safe...
    if (isRoot())
    {
        state.view.showPresetsView.setValue (false);
    }
    else if (directoryContents != nullptr)
    {
        list.deselectAllRows();
        directoryContents->setDirectory (directoryContents->getDirectory().getParentDirectory(), true, true);
    }
}

void PresetsView::refresh()
{
    directoryContents->refresh();
}

void PresetsView::setDialogBounds (juce::Component& dialog, juce::Rectangle<int> parentBounds)
{
    const auto dialogHeight = std::min<int> (350, juce::roundToInt (parentBounds.getHeight() * 0.3));
    juce::Rectangle<int> bounds = { 0, 0, juce::roundToInt (parentBounds.getWidth() * 0.9), dialogHeight };
    juce::Justification justification (juce::Justification::centredTop);
    dialog.setBounds (justification.appliedToRectangle (bounds, parentBounds));
}

void PresetsView::createFolder()
{
    auto* aw = new DialogComponent ("Create New Set",
                                    juce::String(),
                                    this);
    aw->addTextEditor ("FolderInput", juce::String(), juce::String(), false);
    aw->addButton (TRANS ("Create"), 1, juce::KeyPress (juce::KeyPress::returnKey));
    aw->addButton (TRANS ("Cancel"), 0, juce::KeyPress (juce::KeyPress::escapeKey));
    setDialogBounds (*aw, getLocalBounds());
    addAndMakeVisible (aw);
    juce::Timer::callAfterDelay (100, [=] {
        if (aw != nullptr)
        {
            aw->enterModalState (true,
                                 juce::ModalCallbackFunction::forComponent (createFolderCallback,
                                                                            this,
                                                                            juce::Component::SafePointer<DialogComponent> (aw)),
                                 true);
        }
    });
}

void PresetsView::deleteFileWithConfirmation (juce::File file)
{
    juce::String message = "Are you sure you want to delete ";
    if (file.isDirectory())
        message += "entire set?";
    else
        message += "preset?";
    auto* aw = new DialogComponent ("Delete \"" + file.getFileNameWithoutExtension() + "\"",
                                    message,
                                    this);
    aw->addButton (TRANS ("Delete"), 1, juce::KeyPress (juce::KeyPress::returnKey));
    aw->addButton (TRANS ("Cancel"), 0, juce::KeyPress (juce::KeyPress::escapeKey));
    aw->getProperties().set ("filename", file.getFullPathName());
    setDialogBounds (*aw, getLocalBounds());
    addAndMakeVisible (aw);
    juce::Timer::callAfterDelay (100, [=] {
        if (aw != nullptr)
        {
            aw->enterModalState (true,
                                 juce::ModalCallbackFunction::forComponent (deleteFileCallback,
                                                                            this,
                                                                            juce::Component::SafePointer<DialogComponent> (aw)),
                                 true);
        }
    });
}

void PresetsView::renamePreset (juce::File& file, const juce::String& newName, const bool discardTransport)
{
    jassert (file.exists());
    auto renamedFile = file.getParentDirectory().getChildFile (newName + TickUtils::kPresetExtension);
    if (! file.moveFileTo (renamedFile))
    {
        juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::AlertIconType::WarningIcon, "Can\'t Rename!", "Please try again or check permissions.");
        return;
    }
    savePreset (renamedFile, discardTransport);
}

void PresetsView::loadPreset (juce::File presetFile)
{
    juce::ZipFile preset (new juce::FileInputStream (presetFile), true);
    state.loadFromArchive (preset, ticks);
}

void PresetsView::savePreset (juce::File presetNameToSave, const bool discardTransport)
{
    state.presetName = presetNameToSave.getFileNameWithoutExtension();
    juce::FileOutputStream outStream (presetNameToSave);
    if (outStream.openedOk())
        state.saveToArchive (outStream, ticks, discardTransport);
    else
        juce::NativeMessageBox::showMessageBoxAsync (juce::AlertWindow::WarningIcon, "Saving Failed", "Cannot write preset!\nMake sure permissions are correct.");
}

void PresetsView::paint (juce::Graphics& g)
{
    g.fillAll (Colour (19, 67, 62));
}

void PresetsView::resized()
{
    auto bounds = getLocalBounds();
    topBar->setBounds (bounds.removeFromTop (TickLookAndFeel::toolbarHeight));
    list.setBounds (bounds);
}

bool PresetsView::isRoot()
{
    return directoryContents && directoryContents->getDirectory() == TickUtils::getUserFolder();
}

void PresetsView::changeListenerCallback (juce::ChangeBroadcaster* source)
{
    // DirectoyContent
    if (source == directoryContents.get())
    {
        if (! directoryContents->isStillLoading())
            list.updateContent();
        return;
    }
    auto& animator = juce::Desktop::getInstance().getAnimator();
    if (source == &animator && ! animator.isAnimating (&list))
    {
        transitionBackground.reset();
        juce::Desktop::getInstance().getAnimator().removeChangeListener (this);
        repaint();
    }
}

void PresetsView::transitionList()
{
    transitionBackground = std::make_unique<juce::ImageComponent>();
    transitionBackground->setImage (list.createComponentSnapshot (list.getLocalBounds()));
    transitionBackground->setBounds (list.getBounds());
    addAndMakeVisible (transitionBackground.get());
    //    getToolbar()->backButton.setVisible (false);
    auto& animator = juce::Desktop::getInstance().getAnimator();
    animator.addChangeListener (this);
    auto finalBounds = list.getBounds();
    list.setBounds (finalBounds.translated (getWidth(), 0));
    animator.animateComponent (&list, finalBounds, 1.0, 300, false, 0.3, 0.0);
}

PresetsView::PresetModel::PresetModel (PresetsView& parent)
    : owner (parent)
{
}

int PresetsView::PresetModel::getNumRows()
{
    return owner.directoryContents->isStillLoading() ? 0 : owner.directoryContents->getNumFiles();
}

void PresetsView::PresetModel::paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool selected)
{
}

void PresetsView::PresetModel::listBoxItemClicked (int row, const juce::MouseEvent& e)
{
    auto* item = static_cast<PresetView*> (owner.list.getComponentForRowNumber (row));
    if (item->data.isFolder)
    {
        owner.transitionList();
        owner.directoryContents->setDirectory (owner.directoryContents->getFile (row), true, true);
        owner.list.deselectAllRows();
    }
    else
    {
        owner.loadPreset (owner.directoryContents->getFile (row));
    }
}

juce::Component* PresetsView::PresetModel::refreshComponentForRow (int rowNumber, bool isRowSelected, juce::Component* existingComponentToUpdate)
{
    auto* component = dynamic_cast<PresetView*> (existingComponentToUpdate);
    if (component == nullptr)
        component = new PresetView();

    component->isReady = false;
    if (! owner.directoryContents->isStillLoading() && rowNumber < owner.directoryContents->getNumFiles())
    {
        component->index = rowNumber;
        component->isSelected = isRowSelected;
        component->presetsView = &owner;

        // TODO: this should be async!
        owner.queryPreset (owner.directoryContents->getFile (rowNumber), component->data);
        const auto& data = component->data;
        auto& props = component->name.getProperties();
        props.clear();
        props.set ("filename", data.name);
        juce::String time;
        if (data.containsTime)
        {
            time = " [" + String (data.bpm) + " " + String (data.numerator) + "/" + String (data.denumerator) + "]";
            props.set ("time", time);
        }
        component->name.setText (data.name + time, juce::dontSendNotification);
        component->name.setDescription ((data.isFolder ? "Folder " : "Preset " + component->name.getText()));
        component->isReady = true;
    }

    return component;
}

PresetsView::PresetView::PresetView()
    : moreOptions ("More Options...", juce::DrawableButton::ImageFitted)
{
    setFocusContainerType (FocusContainerType::focusContainer);
    auto more = juce::Drawable::createFromImageData (BinaryData::more_vert24px_svg, BinaryData::more_vert24px_svgSize);
    auto moreInverted = more->createCopy();
    more->replaceColour (juce::Colours::black, juce::Colours::white);
    moreOptions.setColour (juce::DrawableButton::backgroundOnColourId, juce::Colours::transparentBlack);
    moreOptions.setImages (more.get(), nullptr, nullptr, nullptr, moreInverted.get());
    addAndMakeVisible (moreOptions);
    name.setInterceptsMouseClicks (false, true);
    name.setFont (16.0f);
    addAndMakeVisible (name);
    setInterceptsMouseClicks (false, true);

    moreOptions.onClick = [this] {
        juce::PopupMenu p;
        auto editIcon = juce::Drawable::createFromImageData (BinaryData::edit24px_svg, BinaryData::edit24px_svgSize);
        editIcon->replaceColour (juce::Colours::black, juce::Colours::white);
        auto deleteIcon = juce::Drawable::createFromImageData (BinaryData::delete24px_svg, BinaryData::delete24px_svgSize);
        deleteIcon->replaceColour (juce::Colours::black, juce::Colours::white);
        auto shareIcon = juce::Drawable::createFromImageData (BinaryData::ios_share_black_24dp_svg, BinaryData::ios_share_black_24dp_svgSize);
        shareIcon->replaceColour (juce::Colours::black, juce::Colours::white);
        p.addItem (1, "Rename..", true, false, std::move (editIcon));
        p.addItem (2, "Delete", true, false, std::move (deleteIcon));
        p.addSeparator();
#if JUCE_IOS || JUCE_ANDROID
        p.addItem (3, "Share", true, false, std::move (shareIcon));
#endif
        auto options = juce::PopupMenu::Options().withParentComponent (getParentComponent()->getParentComponent()->getParentComponent()).withTargetComponent (moreOptions);
        p.showMenuAsync (options, [this] (int value) {
            switch (value)
            {
                case 1:
                    name.setText (name.getProperties().getWithDefault ("filename", ""), juce::dontSendNotification);
                    name.showEditor();
                    name.onEditorHide = [this]() {
                        auto file = presetsView->getFileForIndex (index);
                        if (name.getText() == file.getFileNameWithoutExtension())
                            return;
                        presetsView->renamePreset (file, name.getText(), name.getProperties().getVarPointer ("time") == nullptr);
                    };
                    break;
                case 2:
                    jassert (presetsView != nullptr);
                    presetsView->deleteFileWithConfirmation (presetsView->getFileForIndex (index));
                    break;
                case 3:
#if JUCE_IOS || JUCE_ANDROID
                    juce::ContentSharer::getInstance()->shareFiles ({ presetsView->getFileForIndex (index) }, [] (bool, String) {});
#endif
                    break;
                default:
                    break;
            };
        });
    };
}

void PresetsView::queryPreset (juce::File fileToQuery, PresetData& dataToFill)
{
    dataToFill.containsTime = false;
    dataToFill.isFolder = fileToQuery.isDirectory();
    if (dataToFill.isFolder)
    {
        dataToFill.name = fileToQuery.getFileName();
        return;
    }

    jassert (fileToQuery.existsAsFile());

    ZipFile archive (fileToQuery);
    const auto idx = archive.getIndexOfFileName (INFO_FILE_NAME);
    if (idx >= 0)
    {
        if (archive.getEntry (idx)->uncompressedSize > 512 * 1000)
        {
            // Too big preset?!?
            jassertfalse;
        }
        else
        {
            auto data = std::unique_ptr<InputStream> (archive.createStreamForEntry (idx));
            const auto stateDataToLoad = ValueTree::fromXml (data->readString());
            dataToFill.name = stateDataToLoad.getProperty (IDs::presetName);
            auto transport = stateDataToLoad.getChildWithName (IDs::TRANSPORT);
            dataToFill.uuid = stateDataToLoad.getProperty (IDs::uuid);
            if (transport.isValid())
            {
                dataToFill.containsTime = true;
                dataToFill.bpm = transport.getProperty (IDs::bpm, -1);
                dataToFill.numerator = transport.getProperty (IDs::numerator, -1);
                dataToFill.denumerator = transport.getProperty (IDs::denumerator, -1);
            }
        }
    }
}

void PresetsView::PresetView::paint (juce::Graphics& g)
{
    if (! isReady)
        return;

    auto image = juce::Drawable::createFromImageData (data.isFolder ? BinaryData::folder_open24px_svg : BinaryData::metro_tick_icon_svg, data.isFolder ? BinaryData::folder_open24px_svgSize : BinaryData::metro_tick_icon_svgSize);

    moreOptions.setToggleState (isSelected, juce::dontSendNotification);
    const auto isCurrent = presetsView->state.presetHash == data.uuid;

    if (isSelected)
        g.fillAll (isCurrent ? TickLookAndFeel::Colours::mint : juce::Colours::white);

    const auto textColor = isSelected ? juce::Colours::black : isCurrent ? TickLookAndFeel::Colours::mint : juce::Colours::white;
    image->replaceColour (juce::Colours::black, textColor);
    image->drawWithin (g, getLocalBounds().removeFromLeft (getHeight()).reduced (12).toFloat(), juce::RectanglePlacement(), 1.0f);
    auto& nameLabel = name;
    MessageManager::callAsync ([&nameLabel, textColor]
                               {
        nameLabel.setColour (juce::Label::textColourId, textColor);
        nameLabel.setColour (juce::Label::textWhenEditingColourId, textColor); });
}

void PresetsView::PresetView::resized()
{
    name.setBounds (getHeight(), 0, getWidth() - 2 * getHeight(), getHeight());
    moreOptions.setBounds (getLocalBounds().removeFromRight (getHeight()).reduced (14));
}
