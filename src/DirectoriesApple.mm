#import <Gosu/Directories.hpp>
#import <Gosu/Utility.hpp>
#import "AppleUtility.hpp"
#import <Foundation/Foundation.h>
#import <unistd.h>


static std::wstring stringFromNSString(NSString *string, const wchar_t *fallback)
{
    return string ? Gosu::utf8ToWstring([string UTF8String]) : fallback;
}

void Gosu::useResourceDirectory()
{
    chdir(Gosu::wstringToUTF8(resourcePrefix()).c_str());
}

std::wstring Gosu::userSettingsPrefix()
{
    static std::wstring result = [] {
        @autoreleasepool {
            NSString *library =
                NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES)[0];
            NSString *preferences = [library stringByAppendingPathComponent:@"Preferences"];
            
            return stringFromNSString(preferences, L".") + L"/";
        }
    }();
    return result;
}

std::wstring Gosu::userDocsPrefix()
{
    static std::wstring result = [] {
        @autoreleasepool {
            NSString *documents =
                NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES)[0];
            
            return stringFromNSString(documents, L".") + L"/";
        }
    }();
    return result;
}

std::wstring Gosu::resourcePrefix()
{
    static std::wstring result = [] {
        @autoreleasepool {
            NSString *resources = [NSBundle mainBundle].resourcePath;
            return stringFromNSString(resources, L".") + L"/";
        }
    }();
    return result;
}

std::wstring Gosu::sharedResourcePrefix()
{
#ifdef GOSU_IS_IPHONE
    return resourcePrefix();
#else
    static std::wstring result = [] {
        @autoreleasepool {
            NSString *bundlePath = [NSBundle mainBundle].bundlePath;
            NSString *containingPath = [bundlePath stringByDeletingLastPathComponent];
            
            return stringFromNSString(containingPath, L".");
        }
    }();
    return result;
#endif
}
