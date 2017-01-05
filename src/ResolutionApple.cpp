#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_MAC) && !defined(GOSU_IS_IPHONE)

#import <Gosu/Gosu.hpp>
#import <AppKit/AppKit.h>

static const NSUInteger STYLE_MASK_FROM_SDL2 =
    NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask;

static const NSRect available_frame = [[NSScreen screens][0] visibleFrame];

static const NSRect available_content_frame =
    [NSWindow contentRectForFrameRect:available_frame styleMask:STYLE_MASK_FROM_SDL2];

unsigned Gosu::available_width()
{
    return available_content_frame.size.width;
}

unsigned Gosu::available_height()
{
    return available_content_frame.size.height;
}

#endif
