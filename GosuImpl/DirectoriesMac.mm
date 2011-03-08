#import <Gosu/Directories.hpp>
#import <GosuImpl/MacUtility.hpp>
#import <Gosu/Utility.hpp>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSString.h>
#import <Foundation/NSPathUtilities.h>

std::wstring Gosu::resourcePrefix()
{
    static std::wstring result;
    if (result.empty())
    {
        ObjRef<NSAutoreleasePool> pool([[NSAutoreleasePool alloc] init]);
        NSBundle* bundle = [NSBundle mainBundle];
        if (bundle == nil)
            return L"";
        NSString* str = [bundle resourcePath];
        result = Gosu::widen([str fileSystemRepresentation]) + L"/";
    }
    return result;
}

std::wstring Gosu::sharedResourcePrefix()
{
    static std::wstring result;
    if (result.empty())
    {
        ObjRef<NSAutoreleasePool> pool([[NSAutoreleasePool alloc] init]);
        NSBundle* bundle = [NSBundle mainBundle];
        if (bundle == nil)
            return L"";
        NSString* str = [bundle bundlePath];
        str = [str stringByDeletingLastPathComponent];
        result = Gosu::widen([str fileSystemRepresentation]) + L"/";
    }
    return result;
}
