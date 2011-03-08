#ifndef GOSIMPL_ICONV_HPP
#define GOSIMPL_ICONV_HPP

#include <Gosu/Platform.hpp>

#ifdef __APPLE__
#include </usr/include/iconv.h> // We want Apple's iconv
#else
#include <iconv.h>
#endif
//#endif
#include <errno.h>

namespace Gosu
{
    template<typename Out, const char* to, const char* from, typename In>
    Out iconvert(const In& in)
    {
        if (in.empty())
            return Out();
    
        const size_t bufferLen = 128;
        typedef typename In::value_type InElem;
        typedef typename Out::value_type OutElem;

        static iconv_t cd = iconv_open(to, from);
        
        Out result;
        OutElem buffer[bufferLen];
        
        char* inbuf = const_cast<char*>(reinterpret_cast<const char*>(&in[0]));
        size_t inbytesleft = in.size() * sizeof(InElem);
        char* outbuf = reinterpret_cast<char*>(buffer);
        size_t outbytesleft = sizeof buffer;
        
        for (;;)
        {
            size_t ret = ::iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
            if (ret == static_cast<size_t>(-1) && errno == EILSEQ)
            {
                // Skip illegal sequence part, repeat loop.
                // TODO: Or retry w/ different encoding?
                ++inbuf;
                --inbytesleft;
            }
            else if (ret == static_cast<size_t>(-1) && errno == E2BIG)
            {
                // Append new characters, reset out buffer, then repeat loop.
                result.insert(result.end(), buffer, buffer + bufferLen);
                outbuf = reinterpret_cast<char*>(buffer);
                outbytesleft = sizeof buffer;
            }
            else
            {
                // Append what's new in the buffer, then LEAVE loop.
                result.insert(result.end(), buffer, buffer + bufferLen - outbytesleft / sizeof(OutElem));
                return result;
            }
        }        
    }
}


#endif

