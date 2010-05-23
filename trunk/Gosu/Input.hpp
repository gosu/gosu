//! \file Input.hpp
//! Interface of the Input class.

#ifndef GOSU_INPUT_HPP
#define GOSU_INPUT_HPP

#include <Gosu/Fwd.hpp>
#include <Gosu/Platform.hpp>

#ifdef GOSU_IS_WIN
#include <Gosu/ButtonsWin.hpp>
#include <windows.h>
#endif

#ifdef GOSU_IS_MAC
#include <Gosu/ButtonsMac.hpp>
#endif

#ifdef GOSU_IS_X
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <Gosu/ButtonsX.hpp>
#endif

#include <Gosu/Platform.hpp>
#include <Gosu/Fwd.hpp>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <vector>

namespace Gosu
{
	//! Very lightweight class that identifies a button (keyboard, mouse or other device).
	class Button
	{
		unsigned id_;
		
	public:
		//! For internal use.
		explicit Button(unsigned id) : id_(id) {}
		//! For internal use.
		unsigned id() const { return id_; }

		//! Default constructor; == noButton.
		Button() : id_(noButton) {}

		//! Conversion from ButtonName constants.
		Button(ButtonName name) : id_(name) {}
	};
    
	// Available even on non-iPhone platforms to make it easier to compile the
	// same source for multiple platforms.
    
    //! Struct that saves information about a touch on the surface of a multi-
    //! touch device.
    struct Touch
    {
        //! Allows for identification of a touch across calls.
        void* id;
        //! Position of a touch on the touch screen.
        double x, y;
    };
    typedef std::vector<Touch> Touches;
    
	//! Tests whether two Buttons identify the same physical button.
	inline bool operator==(Button lhs, Button rhs)
	{
		return lhs.id() == rhs.id();
	}
	inline bool operator!=(Button lhs, Button rhs)
	{
		return !(lhs == rhs);
	}
    inline bool operator<(Button lhs, Button rhs)
    {
        return lhs.id() < rhs.id();
    }
	
    //! Manages initialization and shutdown of the input system. Only one Input
	//! instance can exist per application.
    class Input
    {
        struct Impl;
        boost::scoped_ptr<Impl> pimpl;

	public:
        #ifdef GOSU_IS_WIN
        Input(HWND window);
        #endif
        
        #ifdef GOSU_IS_MAC
        #ifdef GOSU_IS_IPHONE
        Input();
        #else
        Input(void* nswindow);
        bool feedNSEvent(void* event);
        #endif
        #endif
        
        #ifdef GOSU_IS_X
        Input(::Display* display, ::Window window);
        bool feedXEvent(::XEvent& event);
        #endif
        
        ~Input();
        
        //! Returns the character a button usually produces, or 0.
        static wchar_t idToChar(Button btn);
        //! Returns the button that has to be pressed to produce the
        //! given character, or noButton.
        static Button charToId(wchar_t ch);
        
		//! Returns true if a button is currently pressed.
        //! Updated every tick.
		bool down(Button btn) const;
        
        //! Returns the horizontal position of the mouse relative to the top
        //! left corner of the window given to Input's constructor.
        double mouseX() const;
        //! See mouseX.
        double mouseY() const;

        //! Immediately moves the mouse as far towards the desired position
        //! as possible. x and y are relativ to the window just as in the mouse
        //! position accessors.
        void setMousePosition(double x, double y);

        // Undocumented for the moment.
        void setMouseFactors(double factorX, double factorY);
        
        //! Collects new information about which buttons are pressed, where the
        //! mouse is and calls onButtonUp/onButtonDown, if assigned.
        void update();
        
		//! Assignable events that are called by update. You can bind these to your own functions.
		//! If you use the Window class, it will assign forward these to its own methods.
        boost::function<void (Button)> onButtonDown, onButtonUp;
        
        //! Returns the currently active TextInput instance, or 0.
        TextInput* textInput() const;
        //! Sets the currently active TextInput, or clears it (input = 0).
        void setTextInput(TextInput* input);
    };
}

#endif
