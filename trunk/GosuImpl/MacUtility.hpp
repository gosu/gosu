#ifndef GOSU_MACUTILITY_HPP
#define GOSU_MACUTILITY_HPP

#ifdef __OBJC__
#import <objc/objc.h>
#import <stdexcept>
#import <boost/utility.hpp>
namespace Gosu
{
    template<typename T>
    class ObjRef : boost::noncopyable
    {
        T* ptr;
    public:
        ObjRef(T* ptr = nil)
        : ptr(ptr) 
        {
        }
        
        ~ObjRef() 
        {
            if (ptr)
                [ptr release];
        }
        
        void reset(T* newPtr = nil)
        { 
            if (ptr)
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
                throw std::logic_error("Tried dereferencing nil");
            return ptr;
        }
    };
}
#endif

#endif
