#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_MAC) && !defined(GOSU_IS_IPHONE)

#include <Gosu/Gosu.hpp>
#import <AppKit/AppKit.h>

static NSUInteger style_mask_from_sdl2 = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask;
// Cannot use accessor syntax here without breaking compilation with OS X 10.7/Xcode 4.6.3.
static NSRect available_frame = [[[NSScreen screens] objectAtIndex:0] visibleFrame];
static NSRect available_content_frame = [NSWindow contentRectForFrameRect:available_frame styleMask:style_mask_from_sdl2];

unsigned Gosu::available_width()
{
    return available_content_frame.size.width;
}

unsigned Gosu::available_height()
{
    return available_content_frame.size.height;
}

#endif
