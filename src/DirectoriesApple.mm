#import <Gosu/Directories.hpp>
#import <Gosu/Utility.hpp>
#import "AppleUtility.hpp"
#import <Foundation/Foundation.h>
#import <unistd.h>


static std::wstring stringFromNSString(NSString* string, const std::wstring& def)
{
    return string ? Gosu::utf8ToWstring([string UTF8String]) : def;
}

void Gosu::useResourceDirectory()
{
    chdir(Gosu::wstringToUTF8(resourcePrefix()).c_str());
}

std::wstring Gosu::userSettingsPrefix()
{
    static std::wstring result;
    if (result.empty())
    {
        ObjCRef<NSAutoreleasePool> pool([NSAutoreleasePool new]);
        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
        NSString *libraryDirectory = [paths objectAtIndex:0];
        NSString *preferencesDirectory = [libraryDirectory stringByAppendingPathComponent:@"Preferences"];
        
        result = stringFromNSString(preferencesDirectory, L".") + L"/";
    }
    return result;
}

std::wstring Gosu::userDocsPrefix()
{
    static std::wstring result;
    if (result.empty())
    {
        ObjCRef<NSAutoreleasePool> pool([NSAutoreleasePool new]);
        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        NSString *documentsDirectory = [paths objectAtIndex:0];
        
        result = stringFromNSString(documentsDirectory, L".") + L"/";
    }
    return result;
}

std::wstring Gosu::resourcePrefix()
{
    static std::wstring result;
    if (result.empty())
    {
        ObjCRef<NSAutoreleasePool> pool([NSAutoreleasePool new]);
        NSString *resourcePath = [[NSBundle mainBundle] resourcePath];
        
        result = stringFromNSString(resourcePath, L".") + L"/";
    }
    return result;
}

std::wstring Gosu::sharedResourcePrefix()
{
    #ifdef GOSU_IS_IPHONE
    return resourcePrefix();
    #else
    static std::wstring result;
    if (result.empty())
    {
        ObjCRef<NSAutoreleasePool> pool([NSAutoreleasePool new]);
        NSString *bundlePath = [[NSBundle mainBundle] bundlePath];
        NSString *containingPath = [bundlePath stringByDeletingLastPathComponent];
        
        result = stringFromNSString(containingPath, L".");
    }
    return result;
    #endif
}
