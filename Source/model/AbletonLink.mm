#include "AbletonLink.h"
#if JUCE_IOS

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import <objc/message.h>
#include <mach/mach_time.h>
#include "../../JUCE/modules/juce_core/native/juce_mac_ObjCHelpers.h"
#include "ABLLinkSettingsViewController.h"

using namespace juce;

class LinkSettings;
static std::unique_ptr<LinkSettings> settingsUI;

AbletonLink::AbletonLink(const double bpm)
{
    ablLink = ABLLinkNew (bpm);
}

AbletonLink::~AbletonLink()
{
    ABLLinkDelete (ablLink);
    settingsUI.reset();
}

@interface StdFunctionToObjCAction  : NSObject
{
@private
    std::function<void()> function;
};

- (id) init: (std::function<void()>) holder;

- (void) handleAction:(id)sender;
@end

@implementation StdFunctionToObjCAction

- (id) init: (std::function<void()>) holder
{
    self = [super init];

    if (self != nil)
    {
        function = holder;
    }
    else
    {
        jassertfalse;
    }

    return self;
}

- (void) handleAction:(id)sender
{
#pragma unused(sender)
    function();
}

@end

static UIViewController* topMostController()
{
    UIViewController *topController = [UIApplication sharedApplication].keyWindow.rootViewController;

    while (topController.presentedViewController) {
        topController = topController.presentedViewController;
    }

    return topController;
}

class LinkSettings
{
public:
    LinkSettings (std::function<void()> onDismiss, ABLLinkRef& ref, juce::Component& source) : onClose (onDismiss)
    {
        topLevelComponent = source.getTopLevelComponent();

        linkSettings = [ABLLinkSettingsViewController instance: ref];
        navController.reset ([[UINavigationController alloc] initWithRootViewController:linkSettings]);
        auto controller = navController.get();

        funcWrapper.reset([[StdFunctionToObjCAction alloc] init: [this]() { close(); }]);

        // this will present a view controller as a popover in iPad and a modal VC on iPhone
        linkSettings.navigationItem.rightBarButtonItem =
            [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemDone
                                                          target:funcWrapper.get()
                                                          action:@selector(handleAction:)];

        // by default, on iPhones/small-factor this will be UIModalPresentationFormSheet, but dragging with JUCE UI seems to require more hooks or it create some visual bugs due to repaint/resized.
        controller.modalPresentationStyle = isIPad() ? UIModalPresentationPopover : UIModalPresentationFullScreen;

        UIPopoverPresentationController *popC = controller.popoverPresentationController;
        popC.permittedArrowDirections = UIPopoverArrowDirectionAny;
        popC.sourceRect = convertToCGRect(source.getScreenBounds());
        popC.sourceView = (UIView*)source.getPeer()->getNativeHandle();

        // we recommend using a size of 320x400 for the display in a popover
        linkSettings.preferredContentSize = CGSizeMake(320., 400.);

        [topMostController() presentViewController:controller animated:YES completion:nil];
    }

private:
    static bool isIPad()
    {
        return [UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPad;
    }

    static CGRect convertToCGRect (juce::Rectangle<int> r)
    {
        return CGRectMake(r.getX(), r.getY(), r.getWidth(), r.getHeight());
    }

    void close()
    {
        [topMostController() dismissViewControllerAnimated:YES completion:^{
            if (topLevelComponent) topLevelComponent->repaint();
            if (onClose) onClose();
        }];
    }

    UIViewController* linkSettings;
    NSUniquePtr<UINavigationController> navController;
    NSUniquePtr<StdFunctionToObjCAction> funcWrapper;
    juce::Component* topLevelComponent;
    std::function<void()> onClose;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LinkSettings)
};

void AbletonLink::showSettings (juce::Component& source, std::function<void()> onDismiss)
{
    settingsUI = std::make_unique<LinkSettings> (std::move(onDismiss), ablLink, source);
}

bool AbletonLink::isLinkEnabled()
{
    return ABLLinkIsEnabled (ablLink);
}

bool AbletonLink::isLinkConnected()
{
    return ABLLinkIsConnected (ablLink);
}

void AbletonLink::linkPosition (AudioPlayHead::PositionInfo& pos, Requests req)
{
    if (!isLinkConnected())
    {
        jassertfalse;
        return;
    }

    const auto stateRef = ABLLinkCaptureAudioSessionState (ablLink);
    const auto ts = pos.getTimeSignature().orFallback (AudioPlayHead::TimeSignature ({4/4}));
    const auto quantum = 4 * ts.numerator / ts.denominator;

    // it seems output latency is already being calculated/counted in code calling this???
    // adding output latency (similar to LinkHut example) seems to break sync on actual device(s).
    uint64_t timeInMicroseconds = mach_absolute_time();

    if (req.isPlaying.has_value())
    {
        const auto reqIsPlaying = *req.isPlaying;
        if (reqIsPlaying && !ABLLinkIsPlaying (stateRef)) {
            // Request starting playback at the beginning of this buffer.
            ABLLinkSetIsPlaying (stateRef, reqIsPlaying, timeInMicroseconds);
        }

        if (!reqIsPlaying && ABLLinkIsPlaying (stateRef)) {
            // Request stopping playback at the beginning of this buffer.
            ABLLinkSetIsPlaying (stateRef, reqIsPlaying, timeInMicroseconds);
        }
    }

    if (!pos.getIsPlaying() && ABLLinkIsPlaying (stateRef)) {
        // Reset the session state's beat timeline so that the requested
        // beat time corresponds to the time the transport will start playing.
        // The returned beat time is the actual beat time mapped to the time
        // playback will start, which therefore may be less than the requested
        // beat time by up to a quantum.
        ABLLinkRequestBeatAtStartPlayingTime (stateRef, 0., quantum);
        pos.setIsPlaying (true);
    }
    else if(pos.getIsPlaying() && !ABLLinkIsPlaying (stateRef)) {
        pos.setIsPlaying (false);
    }

    // Handle a tempo proposal
    if (req.bpm.has_value()) {
        // Propose that the new tempo takes effect at the beginning of
        // this buffer.
        ABLLinkSetTempo (stateRef, *req.bpm, timeInMicroseconds);
    }

    ABLLinkCommitAudioSessionState (ablLink, stateRef);

    pos.setIsPlaying (ABLLinkIsPlaying (stateRef));
    pos.setBpm (ABLLinkGetTempo (stateRef));

    auto beat = ABLLinkBeatAtTime (stateRef, timeInMicroseconds, quantum);
#if JUCE_DEBUG
    auto phase = ABLLinkPhaseAtTime (stateRef, timeInMicroseconds, quantum);
    DBG("BPM " << pos.getBpm().orFallback(0.0));
    DBG("IS PLAYING - " << (pos.getIsPlaying() ? "YES" : "NO"));
    DBG("BEAT " << juce::String(beat) << " " << "PHASE " << juce::String(phase));
    DBG("OUR DATA " << juce::String(pos.getPpqPosition().orFallback(0.0)));
#endif
    pos.setPpqPosition (beat);
}

#endif
