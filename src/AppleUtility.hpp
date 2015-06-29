#ifndef GOSU_APPLEUTILITY_HPP
#define GOSU_APPLEUTILITY_HPP

#include <Gosu/Platform.hpp>

#import <CoreFoundation/CoreFoundation.h>

#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <stdexcept>

namespace Gosu
{
    inline void throwOSError(OSStatus status, unsigned line)
    {
        std::ostringstream str;
        #ifdef GOSU_IS_IPHONE
        str << "Error on line " << line << " (Code " << status << ")";
        #else
        str << "Error on line " << line << " (Code " << status << "): "
            << GetMacOSStatusErrorString(status)
            << " (" << GetMacOSStatusCommentString(status) << ")";
        #endif
        throw std::runtime_error(str.str());
    }
    
    #define CHECK_OS(status) if (!(status)) {} else Gosu::throwOSError(status, __LINE__)
}

#ifdef __OBJC__
#import <objc/objc.h>
#import <stdexcept>
namespace Gosu
{
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
