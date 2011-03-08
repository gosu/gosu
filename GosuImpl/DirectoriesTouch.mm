#import <Gosu/Directories.hpp>
#import <Gosu/Utility.hpp>
#import <GosuImpl/MacUtility.hpp>
#import <Foundation/Foundation.h>

std::wstring Gosu::userSettingsPrefix()
{
    static std::wstring result;
    if (result.empty())
    {
        ObjRef<NSAutoreleasePool> pool([[NSAutoreleasePool alloc] init]);
        NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        NSString* documentsDirectory = [paths objectAtIndex: 0];
        result = utf8ToWstring([documentsDirectory UTF8String]) + L"/";
    }
    return result;
}

std::wstring Gosu::userDocsPrefix()
{
    return userSettingsPrefix();
}

std::wstring Gosu::resourcePrefix()
{
    static std::wstring result;
    if (result.empty())
    {
        ObjRef<NSAutoreleasePool> pool([[NSAutoreleasePool alloc] init]);
        result = utf8ToWstring([[[NSBundle mainBundle] resourcePath] UTF8String]) + L"/";
    }
    return result;
}

std::wstring Gosu::sharedResourcePrefix()
{
    return resourcePrefix();
}
