#ifndef GOSU_APPLEUTILITY_HPP
#define GOSU_APPLEUTILITY_HPP

#include <Gosu/Platform.hpp>
#include <CoreFoundation/CoreFoundation.h>

#ifdef __OBJC__

#include <string>
#include <stdexcept>
#import <Foundation/Foundation.h>

inline static void throwOSError(OSStatus status, unsigned line)
{
    @autoreleasepool {
        NSError *error = [NSError errorWithDomain:NSOSStatusErrorDomain code:status userInfo:nil];
        throw std::runtime_error("Error on line " + std::to_string(line) +
                                 " (Code " + std::to_string(status) + "): " +
                                 error.localizedDescription.UTF8String +
                                 " (" + error.description.UTF8String + ")");
    }
}

#define CHECK_OS(status) do { if (status) throwOSError(status, __LINE__); } while (0)

#endif

namespace Gosu
{
    template<typename CFRefType = CFTypeRef>
    class CFRef
    {
        CFRefType ref;
        
    public:
        CFRef(CFRefType ref)
        : ref(ref)
        {
        }
        
        ~CFRef()
        {
            if (ref) {
                CFRelease(ref);
            }
        }

        CFRef(const CFRef&) = delete;
        CFRef& operator=(const CFRef&) = delete;

        CFRefType get() const
        {
            return ref;
        }
        
        CFRefType obj() const
        {
            if (!ref) {
                throw std::logic_error("CoreFoundation reference is NULL");
            }
            return ref;
        }
    };
}

#endif
