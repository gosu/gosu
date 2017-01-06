#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_MAC)

#import <Gosu/Directories.hpp>
#import <Foundation/Foundation.h>
#import <unistd.h>


void Gosu::use_resource_directory()
{
    chdir(resource_prefix().c_str());
}

std::string Gosu::user_settings_prefix()
{
    static std::string result = [] {
        @autoreleasepool {
            NSString* library =
                NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES)[0];
            NSString* preferences = [library stringByAppendingPathComponent:@"Preferences"];
            
            return std::string(preferences.UTF8String ?: ".") + "/";
        }
    }();
    return result;
}

std::string Gosu::user_documents_prefix()
{
    static std::string result = [] {
        @autoreleasepool {
            NSString* documents =
                NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES)[0];
            
            return std::string(documents.UTF8String ?: ".") + "/";
        }
    }();
    return result;
}

std::string Gosu::resource_prefix()
{
    static std::string result = [] {
        @autoreleasepool {
            NSString* resources = [NSBundle mainBundle].resourcePath;
            return std::string(resources.UTF8String ?: ".") + "/";
        }
    }();
    return result;
}

std::string Gosu::shared_resource_prefix()
{
    #ifdef GOSU_IS_IPHONE
    return resource_prefix();
    #else
    static std::string result = [] {
        @autoreleasepool {
            NSString* bundle_path = [NSBundle mainBundle].bundlePath;
            NSString* containing_path = [bundle_path stringByDeletingLastPathComponent];
            return std::string(containing_path.UTF8String ?: ".");
        }
    }();
    return result;
    #endif
}

#endif
