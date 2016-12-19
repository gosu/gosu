#import <Gosu/Directories.hpp>
#import <Gosu/Utility.hpp>
#import "AppleUtility.hpp"
#import <Foundation/Foundation.h>
#import <unistd.h>


static std::wstring string_from_ns_string(NSString *string, const wchar_t *fallback)
{
    return string ? Gosu::utf8_to_wstring([string UTF8String]) : fallback;
}

void Gosu::use_resource_directory()
{
    chdir(Gosu::wstring_to_utf8(resource_prefix()).c_str());
}

std::wstring Gosu::user_settings_prefix()
{
    static std::wstring result = [] {
        @autoreleasepool {
            NSString *library =
                NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES)[0];
            NSString *preferences = [library stringByAppendingPathComponent:@"Preferences"];
            
            return string_from_ns_string(preferences, L".") + L"/";
        }
    }();
    return result;
}

std::wstring Gosu::user_documents_prefix()
{
    static std::wstring result = [] {
        @autoreleasepool {
            NSString *documents =
                NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES)[0];
            
            return string_from_ns_string(documents, L".") + L"/";
        }
    }();
    return result;
}

std::wstring Gosu::resource_prefix()
{
    static std::wstring result = [] {
        @autoreleasepool {
            NSString *resources = [NSBundle mainBundle].resourcePath;
            return string_from_ns_string(resources, L".") + L"/";
        }
    }();
    return result;
}

std::wstring Gosu::shared_resource_prefix()
{
#ifdef GOSU_IS_IPHONE
    return resource_prefix();
#else
    static std::wstring result = [] {
        @autoreleasepool {
            NSString *bundle_path = [NSBundle mainBundle].bundlePath;
            NSString *containing_path = [bundle_path stringByDeletingLastPathComponent];
            return string_from_ns_string(containing_path, L".");
        }
    }();
    return result;
#endif
}
