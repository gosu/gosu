//! \file TextInput.hpp
//! Interface of the Image class and helper functions.

#ifndef GOSU_TEXTINPUT_HPP
#define GOSU_TEXTINPUT_HPP

#include <Gosu/Fwd.hpp>
#include <Gosu/Platform.hpp>
#include <boost/scoped_ptr.hpp>

namespace Gosu
{
    class TextInput
    {
        struct Impl;
        boost::scoped_ptr<Impl> pimpl;
        
    public:
        TextInput();
        ~TextInput();

        std::wstring text() const;
        unsigned caretPos() const;
        unsigned selectionStart() const;

        #ifdef GOSU_IS_MAC
        void feedNSEvent(void* event);
        #endif
    };
}

#endif
