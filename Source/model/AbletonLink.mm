#include "AbletonLink.h"
#if JUCE_IOS

#import <Foundation/Foundation.h>
#import <objc/message.h>
#include "../../JUCE/modules/juce_core/native/juce_mac_ObjCHelpers.h"
#include "ABLLinkSettingsViewController.h"

using namespace juce;

AbletonLink::AbletonLink(const double bpm)
{
    ablLink = ABLLinkNew (bpm);
}

AbletonLink::~AbletonLink()
{
    ABLLinkDelete (ablLink);
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

static std::unique_ptr<LinkSettings> settingsUI;

void AbletonLink::showSettings (juce::Component& source, std::function<void()> onDismiss)
{
    settingsUI = std::make_unique<LinkSettings> (std::move(onDismiss), ablLink, source);
}

#endif
