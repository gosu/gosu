//! \file TextInput.hpp
//! Interface of the TextInput class.

#ifndef GOSU_TEXTINPUT_HPP
#define GOSU_TEXTINPUT_HPP

#include <Gosu/Fwd.hpp>
#include <Gosu/Platform.hpp>
#include <Gosu/TR1.hpp>
#include <string>

namespace Gosu
{
    //! TextInput instances are invisible objects that build a text string from input,
    //! using the current operating system's keyboard layout.
    //! At its most basic form, you only need to create a new TextInput instance and
    //! pass it to your window via setTextInput. Until you call this function again,
    //! passing 0, the TextInput object will build a text that can be accessed via
    //! TextInput::text().
    //! A TextInput object is purely abstract, though; drawing the input field is left
    //! to the user. As with most of Gosu, how this is handled is completely left open.
    //! TextInput only aims to provide enough code for your own GUIs to build upon.
    class TextInput
    {
        struct Impl;
        const GOSU_UNIQUE_PTR<Impl> pimpl;
    #if defined(GOSU_CPP11_ENABLED)
        TextInput(TextInput&&) = delete;
        TextInput& operator=(TextInput&&) = delete;
        TextInput(const TextInput&) = delete;
        TextInput& operator=(const TextInput&) = delete;
    #endif
        
    public:
        TextInput();
        virtual ~TextInput();

        std::wstring text() const;

        //! Replaces the current text by the given string and positions the cursor
        //! at the end of the text, with an empty selection.
        void setText(const std::wstring& text);

        //! Position of the caret as the index of the character that it's left to.
        unsigned caretPos() const;
        //! Sets the caret position as returned by caretPos.
        //! You usually also want to use setSelectionStart.
        void setCaretPos(unsigned pos);

        //! If there is a selection, the selectionStart() member yields its beginning,
        //! using the same indexing scheme as caretPos. If there is no selection,
        //! selectionStart() is equal to caretPos().
        unsigned selectionStart() const;
        //! Sets the start of the selection as returned by selectionStart.
        void setSelectionStart(unsigned pos);
        
        // Platform-specific communication with Gosu::Input.
        bool feedSDLEvent(void* event);

        //! Overridable filter that is applied to all new text that is entered.
        //! Allows for context-sensitive filtering/extending/... of the text.
        //! The text will be inserted at caretPos afterwards.
        virtual std::wstring filter(const std::wstring& text) const
        {
            return text;
        }
    };
}

#endif
