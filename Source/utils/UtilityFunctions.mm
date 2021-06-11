#import <Foundation/Foundation.h>
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR

#include "UtilityFunctions.h"
// https://forum.juce.com/t/app-group-folder-access/23102/56
juce::File TickUtils::getAppGroupContainerLocation (const juce::String& appGroupID) {
    auto nsGroupID = [NSString stringWithUTF8String: appGroupID.toUTF8()];
    auto fm   = [NSFileManager defaultManager];
    auto path = [fm containerURLForSecurityApplicationGroupIdentifier:nsGroupID].path;
    return juce::File(juce::CharPointer_UTF8 ([path UTF8String]));
}
#endif
