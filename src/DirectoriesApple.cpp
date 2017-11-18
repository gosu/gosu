#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_MAC)

#import <Gosu/Directories.hpp>
#import <Foundation/Foundation.h>
#import <unistd.h>
using namespace std;

void Gosu::use_resource_directory()
{
    chdir(resource_prefix().c_str());
}

string Gosu::user_settings_prefix()
{
    static string result = [] {
        @autoreleasepool {
            NSString* library =
                NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES)[0];
            NSString* preferences = [library stringByAppendingPathComponent:@"Preferences"];
            
            return string(preferences.UTF8String ?: ".") + "/";
        }
    }();
    return result;
}

string Gosu::user_documents_prefix()
{
    static string result = [] {
        @autoreleasepool {
            NSString* documents =
                NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES)[0];
            
            return string(documents.UTF8String ?: ".") + "/";
        }
    }();
    return result;
}

string Gosu::resource_prefix()
{
    static string result = [] {
        @autoreleasepool {
            NSString* resources = [NSBundle mainBundle].resourcePath;
            return string(resources.UTF8String ?: ".") + "/";
        }
    }();
    return result;
}

string Gosu::shared_resource_prefix()
{
    #ifdef GOSU_IS_IPHONE
    return resource_prefix();
    #else
    static string result = [] {
        @autoreleasepool {
            NSString* bundle_path = [NSBundle mainBundle].bundlePath;
            NSString* containing_path = [bundle_path stringByDeletingLastPathComponent];
            return string(containing_path.UTF8String ?: ".");
        }
    }();
    return result;
    #endif
}

#endif
