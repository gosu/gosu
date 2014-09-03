#include <Gosu/Gosu.hpp>
#import <AppKit/AppKit.h>

namespace Gosu
{
    NSUInteger styleMaskFromSDL2 = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask;
    NSRect availableFrame = [[[NSScreen screens] objectAtIndex:0] visibleFrame];
    NSRect availableContentFrame = [NSWindow contentRectForFrameRect:availableFrame styleMask:styleMaskFromSDL2];
}

unsigned Gosu::availableWidth()
{
    return availableContentFrame.size.width;
}

unsigned Gosu::availableHeight()
{
    return availableContentFrame.size.height;
}
