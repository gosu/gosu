#ifndef GOSU_APPLEUTILITY_HPP
#define GOSU_APPLEUTILITY_HPP

#include <Gosu/Platform.hpp>

#ifdef __OBJC__

#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <objc/objc.h>
#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>

namespace Gosu
{
    inline void throwOSError(OSStatus status, unsigned line)
    {
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
        NSError *error = [NSError errorWithDomain:NSOSStatusErrorDomain code:status userInfo:nil];
        std::ostringstream str;
        str << "Error on line " << line << " (Code " << status << "): "
        << [[error localizedDescription] UTF8String]
        << " (" << [[error description] UTF8String] << ")";
        [pool release];
        throw std::runtime_error(str.str());
    }
    
    #define CHECK_OS(status) if (!(status)) {} else Gosu::throwOSError(status, __LINE__)

    template<typename T>
    class ObjCRef
    {
        ObjCRef(const ObjCRef&);
        ObjCRef& operator=(const ObjCRef&);
        
        T* ptr;
        
    public:
        ObjCRef(T* ptr = nil)
        : ptr(ptr) 
        {
        }
        
        ~ObjCRef() 
        {
            [ptr release];
        }
        
        void reset(T* newPtr = nil)
        { 
            [ptr release]; 
            ptr = newPtr;
        }
        
        T* get() const 
        {
            return ptr; 
        }
        
        T* obj() const
        {
            if (!ptr)
                throw std::logic_error("Objective-C reference is nil");
            return ptr;
        }
    };
}
#endif

namespace Gosu
{
    template<typename CFRefType = CFTypeRef>
    class CFRef
    {
        CFRef(const CFRef&);
        CFRef& operator=(const CFRef&);
        
        CFRefType ref;
        
    public:
        CFRef(CFRefType ref)
        :   ref(ref)
        {
        }
        
        ~CFRef()
        {
            if (ref)
                CFRelease(ref);
        }
        
        CFRefType get() const
        {
            return ref;
        }
        
        CFRefType obj() const
        {
            if (!ref)
                throw std::logic_error("CoreFoundation reference is NULL");
            return ref;
        }
    };
}

#endif
