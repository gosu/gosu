#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_MAC)

#import <Gosu/Utility.hpp>
#import <Foundation/Foundation.h>
using namespace std;

string Gosu::language()
{
    @autoreleasepool {
        NSString* language = [NSLocale preferredLanguages][0];
        return language.UTF8String ?: "en";
    }
}

#endif
