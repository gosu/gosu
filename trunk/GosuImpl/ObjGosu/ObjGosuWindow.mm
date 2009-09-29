//
//  Gosu_Window.m
//  Gosu
//
//  Created by Julian Raschke on 25.09.09.
//  Copyright 2009 Raschke & Ludwig GbR. All rights reserved.
//

#import "ObjGosuWindow.h"
#import <Carbon/Carbon.h>
#import <MacRuby/MacRuby.h>

@implementation ObjGosuWindow
- (id) initWithWidth:(int)width height:(int)height inFullscreen:(BOOL)fullscreen
{
    if (not (self = [super init]))
        return NULL;
    window = new Gosu::Window(width, height, fullscreen);
    return self;
}

- (void) show
{
    window->show();
}
@end

extern "C" void Init_gosu()
{
    [[MacRuby sharedRuntime] evaluateString:
        @"module Gosu\n"
        @"  class Window < ::ObjGosuWindow\n"
        @"    def self.new(width, height, fullscreen)\n"
        @"      self.alloc.initWithWidth(width, height: height, inFullscreen: fullscreen)\n"
        @"    end\n"
        @"  end\n"
        @"end\n"
    ];
}
