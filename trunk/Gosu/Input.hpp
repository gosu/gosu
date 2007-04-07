//! \file Input.hpp
//! Interface of the Input class.

#ifndef GOSU_INPUT_HPP
#define GOSU_INPUT_HPP

#include <Gosu/Platform.hpp>

#ifdef GOSU_IS_WIN
#include <Gosu/ButtonsWin.hpp>
#include <windows.h>
#endif

#ifdef GOSU_IS_MAC
#include <Gosu/ButtonsMac.hpp>
#endif

#ifdef GOSU_IS_X
#include <Gosu/Fwd.hpp>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <Gosu/ButtonsX.hpp>
#endif

#include <Gosu/Platform.hpp>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>

namespace Gosu
{
    //! Offers information about which buttons are currently pressed,
    //! translates between button ids and characters and call events
    //! when buttons are pushed or released.
    class Input
    {
        struct Impl;
        boost::scoped_ptr<Impl> pimpl;

#ifdef SWIG
        Input();
#endif

public:
#ifdef GOSU_IS_WIN
        Input(HWND window);
#endif
#ifdef GOSU_IS_MAC
        Input(void* nswindow);
        bool feedNSEvent(void* event);
#endif
#ifdef GOSU_IS_X
        Input(::Display* dpy);
        bool feedXEvent(::XEvent& event, Window* window);
#endif
        ~Input();

        //! Returns whether a key was pressed when update() was called for the
        //! last time.
        bool down(unsigned id) const;

        //! Returns the character a button id usually produces, or 0.
        wchar_t idToChar(unsigned id) const;
        //! Returns the id of the button that has to be pressed to produce the
        //! given character, or 0.
        unsigned charToId(wchar_t ch) const;

        //! Returns the horizontal position of the mouse relative to the top
        //! left corner of the window given to Input's constructor.
        double mouseX() const;
        //! See mouseX.
        double mouseY() const;
        // See comments on virtual resolutions in Graphics.hpp.
        void setMouseResolution(double width, double height);

        //! Collects new information about which buttons are pressed, where the
        //! mouse is and calls onButtonUp/onButtonDown, if assigned.
        void update();

        boost::function<void (unsigned)> onButtonDown, onButtonUp;
    };
}

#endif
