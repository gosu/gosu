//! \file TR1.hpp
//! Includes all parts of C++03 (TR1) that are relevant for Gosu. It makes available the following members of the std::tr1 namespace: array, bind, function, shared_ptr, uint*_t and int*_t.

#ifndef GOSU_TR1_HPP
#define GOSU_TR1_HPP

#include <memory>

#if defined(_MSC_VER) || defined(_LIBCPP_MEMORY)
    #include <array>
    #include <functional>
    namespace std
    {
        namespace tr1
        {
            typedef unsigned char uint8_t;
            typedef unsigned short uint16_t;
            typedef unsigned int uint32_t;
            typedef unsigned long long uint64_t;
            typedef signed char int8_t;
            typedef signed short int16_t;
            typedef signed int int32_t;
            typedef signed long long int64_t;
            
            #ifdef _LIBCPP_MEMORY
            namespace placeholders
            {
                using namespace std::placeholders;
            }
            using std::array;
            using std::bind;
            using std::function;
            using std::shared_ptr;
            #endif
        }        
    }
#else
    #include <tr1/array>
    #include <tr1/memory>
    #include <tr1/functional>
    #if defined(__GNUC__) && (__GNUC__ < 4 || __GNUC_MINOR__ < 2)
        #include <stdint.h>
        namespace std
        {
            namespace tr1
            {
                using ::int8_t; using ::int16_t; using ::int32_t; using ::int64_t;
                using ::uint8_t; using ::uint16_t; using ::uint32_t; using ::uint64_t;
            }
        }
    #else
        #include <tr1/cstdint>
    #endif
#endif

#endif
