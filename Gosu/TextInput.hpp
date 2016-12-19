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
    //! passing 0, the TextInput object will build a text that can be accessed via
    //! TextInput::text().
    //! A TextInput object is purely abstract, though; drawing the input field is left
    //! to the user. As with most of Gosu, how this is handled is completely left open.
    //! TextInput only aims to provide enough code for your own GUIs to build upon.
    class TextInput
    {
        struct Impl;
        // Non-movable (const) to avoid dangling internal references.
        const std::unique_ptr<Impl> pimpl;
        
    public:
        TextInput();
        virtual ~TextInput();

        std::wstring text() const;

        //! Replaces the current text by the given string and positions the cursor
        //! at the end of the text, with an empty selection.
        void set_text(const std::wstring& text);

        //! Position of the caret as the index of the character that it's left to.
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
        
        // Platform-specific communication with Gosu::Input.
        bool feed_sdl_event(void* event);

        //! Overridable filter that is applied to all new text that is entered.
        //! Allows for context-sensitive filtering/extending/... of the text.
        //! The text will be inserted at caret_pos afterwards.
        virtual std::wstring filter(const std::wstring& text) const
        {
            return text;
        }
    };
}
