#include <Gosu/Directories.hpp>
#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_MAC)

#import <Foundation/Foundation.h>
#include <filesystem>

std::string Gosu::resource_path(const std::string& relative_filename)
{
    static const std::string resource_prefix = [] {
        @autoreleasepool {
            NSString* resources = [NSBundle mainBundle].resourcePath;
            return std::string(resources.UTF8String ?: ".");
        }
    }();

    return relative_filename.empty() ? resource_prefix : resource_prefix + "/" + relative_filename;
}

std::string Gosu::user_settings_path(const std::string& organization,
                                     const std::string& application,
                                     const std::string& relative_filename)
{
    static const std::string user_settings_prefix = [] {
        @autoreleasepool {
            NSString* library
                = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES)
                      .firstObject;
            NSString* preferences = [library stringByAppendingPathComponent:@"Application Support"];
            return std::string(preferences.UTF8String ?: ".");
        }
    }();

    std::string directory = user_settings_prefix;
    if (!organization.empty()) {
        directory += '/';
        directory += organization;
    }
    if (!application.empty()) {
        directory += '/';
        directory += application;
    }
    if (!std::filesystem::is_directory(directory)) {
        std::filesystem::create_directories(directory);
    }

    return relative_filename.empty() ? directory : directory + '/' + relative_filename;
}

#endif
