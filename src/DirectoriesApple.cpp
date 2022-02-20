#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_MAC)

#import <Gosu/Directories.hpp>
#import <Foundation/Foundation.h>
#import <unistd.h>

void Gosu::use_resource_directory()
{
    chdir(resource_prefix().c_str());
}

const std::string& Gosu::user_settings_prefix()
{
    static const std::string user_settings_prefix = [] {
        @autoreleasepool {
            NSString* library =
                    NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES)
                            .firstObject;
            NSString* preferences = [library stringByAppendingPathComponent:@"Preferences"];

            return std::string{preferences.UTF8String ?: "."} + "/";
        }
    }();
    return user_settings_prefix;
}

const std::string& Gosu::user_documents_prefix()
{
    static const std::string user_documents_prefix = [] {
        @autoreleasepool {
            NSString* documents =
                    NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES)
                            .firstObject;

            return std::string{documents.UTF8String ?: "."} + "/";
        }
    }();
    return user_documents_prefix;
}

const std::string& Gosu::resource_prefix()
{
    static const std::string resource_prefix = [] {
        @autoreleasepool {
            NSString* resources = [NSBundle mainBundle].resourcePath;
            return std::string{resources.UTF8String ?: "."} + "/";
        }
    }();
    return resource_prefix;
}

const std::string& Gosu::shared_resource_prefix()
{
#ifdef GOSU_IS_IPHONE
    return resource_prefix();
#else
    static const std::string shared_resource_prefix = [] {
        @autoreleasepool {
            NSString* bundle_path = [NSBundle mainBundle].bundlePath;
            NSString* containing_path = [bundle_path stringByDeletingLastPathComponent];
            return std::string{containing_path.UTF8String ?: "."};
        }
    }();
    return shared_resource_prefix;
#endif
}

#endif
