//! \file TextInput.hpp
//! Interface of the TextInput class.

#pragma once

#include <Gosu/Fwd.hpp>
#include <Gosu/Platform.hpp>
#include <memory>
#include <string>

namespace Gosu
{
    //! TextInput instances are invisible objects that build a text string from input,
    //! using the current operating system's keyboard layout.
    //! At its most basic form, you only need to create a new TextInput instance and
    //! pass it to your window via set_text_input. Until you call this function again,
    //! passing nullptr, the TextInput object will build a text that can be accessed via
    //! TextInput::text().
    //! A TextInput object is purely abstract. It is up to you to actually draw a text field.
    //! TextInput only provides a portable base for your own GUI to build upon.
    class TextInput
    {
        struct Impl;
        // Non-movable (const) to avoid dangling references to TextInput instances.
        const std::unique_ptr<Impl> pimpl;
        
    public:
        TextInput();
        virtual ~TextInput();

        std::string text() const;

        //! Replaces the current text by the given string and positions the cursor
        //! at the end of the text, with an empty selection.
        void set_text(const std::string& text);

        //! Index of the character that comes after the caret. When the text is empty, this is
        //! always 0. For ease of use in C++, this counts bytes, not codepoints in the UTF-8 encoded
        //! string.
        unsigned caret_pos() const;
        //! Sets the caret position as returned by caret_pos.
        //! You usually also want to use set_selection_start.
        void set_caret_pos(unsigned pos);

        //! If there is a selection, the selection_start() member yields its beginning,
        //! using the same indexing scheme as caret_pos. If there is no selection,
        //! selection_start() is equal to caret_pos().
        unsigned selection_start() const;
        //! Sets the start of the selection as returned by selection_start.
        void set_selection_start(unsigned pos);
        
#ifndef GOSU_IS_IPHONE
        // Platform-specific communication with Gosu::Input.
        bool feed_sdl_event(void* event);
#endif

        //! Overridable filter that is applied to all new text that is entered.
        //! Allows for context-sensitive filtering/extending/... of new characters.
        //! The return value will be inserted at caret_pos.
        virtual std::string filter(std::string text) const
        {
            return text;
        }
        
        //! Replaces the current selection (if any) and inserts the given string at the current
        //! caret position.
        //! The filter method will not be applied before appending the string.
        void insert_text(std::string text);
        
        //! Deletes the current selection, if any, or the next character.
        void delete_forward();
        
        //! Deletes the current selection, if any, or the previous character.
        void delete_backward();
    };
}
