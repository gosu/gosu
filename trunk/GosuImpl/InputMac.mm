#import <AppKit/AppKit.h>
#import <Carbon/Carbon.h>
#include <Gosu/Input.hpp>
#include <Gosu/Utility.hpp>
#include <IOKit/hidsystem/IOLLEvent.h>
#include <boost/array.hpp>
#include <boost/cstdint.hpp>
#include <map>
#include <string>
#include <vector>

namespace {
    // This is just a wild assumption. For Apple ADB keyboards, I read something
    // about 127 being the max, but checking more than that will not hurt, so...
    const unsigned numScancodes = 256;
    
    boost::array<wchar_t, numScancodes> idChars;
    std::map<wchar_t, unsigned> charIds;
    
    void initCharTranslation()
    {
        static bool initializedCharData = false;
        if (initializedCharData)
            return;
        initializedCharData = true;
        
        idChars.assign(0);

		const void* KCHR = reinterpret_cast<const void*>(GetScriptManagerVariable(smKCHRCache));
		if (!KCHR)
			return;
        
        for (unsigned code = 0; code < numScancodes; ++code)
        {
            UInt32 deadKeyState = 0;
			UInt32 value = KeyTranslate(KCHR, code, &deadKeyState);
			// If this key triggered a dead key, hit it again to obtain the actual value.
			if (deadKeyState != 0)
				value = KeyTranslate(KCHR, code, &deadKeyState);
			
			// No character! Pity.
			if (value == 0)
				continue;
			
			// Now we have a character which is *not* limited to the ASCII range. To correctly
			// translate this into a wchar_t, we need to convert it based on the current locale.
			// TODO: That locale stuff should be explicit. Locales always cause trouble.
			
			std::string str(1, char(value));
			wchar_t ch = Gosu::widen(str).at(0);
			
            idChars[code] = ch;
            charIds[ch] = code;
        }
    }
	 
    boost::array<bool, Gosu::numButtons> buttonStates;
}

bool Gosu::Button::isDown() const
{
    return buttonStates.at(id);
}

struct Gosu::Input::Impl
{
    Input& input;
    NSWindow* window;
    double mouseX, mouseY;
    double mouseFactorX, mouseFactorY;
    
    unsigned currentMods;
    
    struct WaitingButton
    {
        Button btn;
        bool down;
        WaitingButton(unsigned btnId, bool down) : btn(btnId), down(down) {}
    };
    std::vector<WaitingButton> queue;

    Impl(Input& input)
    : input(input), mouseFactorX(1), mouseFactorY(1), currentMods(0)
    {
    }
    
    void enqueue(unsigned btnId, bool down)
    {
        queue.push_back(WaitingButton(btnId, down));
    }

    void updateMods(unsigned newMods)
    {
        const unsigned ids[8] = { kbLeftShift, kbLeftControl,
                                  kbLeftAlt, kbLeftMeta,
                                  kbRightShift, kbRightControl,
                                  kbRightAlt, kbRightMeta };
        const unsigned bits[8] = { NX_DEVICELSHIFTKEYMASK, NX_DEVICELCTLKEYMASK,
                                   NX_DEVICELALTKEYMASK, NX_DEVICELCMDKEYMASK,
                                   NX_DEVICERSHIFTKEYMASK, NX_DEVICERCTLKEYMASK,
                                   NX_DEVICERALTKEYMASK, NX_DEVICERCMDKEYMASK };
    
        for (unsigned i = 0; i < 8; ++i)
            if ((currentMods & bits[i]) != (newMods & bits[i]))
                enqueue(ids[i], (newMods & bits[i]) != 0);
        
        currentMods = newMods;
    }
};

Gosu::Input::Input(void* window)
: pimpl(new Impl(*this))
{
    pimpl->window = static_cast<NSWindow*>(window);
    buttonStates.assign(false);
    initCharTranslation();
}

Gosu::Input::~Input()
{
}

bool Gosu::Input::feedNSEvent(void* event)
{
    NSEvent* ev = (NSEvent*)event;
    
    // Process modifier keys.
    unsigned mods = [ev modifierFlags];
    if (mods != pimpl->currentMods)
        pimpl->updateMods(mods);
    
    // Handle mouse input.
    unsigned type = [ev type];
    switch (type)
    {
        case NSLeftMouseDown:
            pimpl->enqueue(msLeft, true);
            return true;
        case NSLeftMouseUp:
            pimpl->enqueue(msLeft, false);
            return true;
        case NSRightMouseDown:
            pimpl->enqueue(msRight, true);
            return true;
        case NSRightMouseUp:
            pimpl->enqueue(msRight, false);
            return true;
    }
        
    // Handle other keys.
    if (type == NSKeyDown || type == NSKeyUp)
    {
        pimpl->enqueue([ev keyCode], type == NSKeyDown);
        return true;
    }

    return false;
}

wchar_t Gosu::Input::idToChar(Button btn) const
{
    if (btn.getId() < numScancodes)
        return idChars[btn.getId()];
    else
        return 0;
}

Gosu::Button Gosu::Input::charToId(wchar_t ch) const
{
    std::map<wchar_t, unsigned>::const_iterator iter = charIds.find(ch);
    if (iter == charIds.end())
        return noButton;
    return Gosu::Button(iter->second);
}

double Gosu::Input::mouseX() const
{
    return pimpl->mouseX * pimpl->mouseFactorX;
}

double Gosu::Input::mouseY() const
{
    return pimpl->mouseY * pimpl->mouseFactorY;
}

void Gosu::Input::setMouseResolution(double width, double height)
{
    NSSize size;
    if (pimpl->window)
        size = [[pimpl->window contentView] frame].size;
    else
    {
        CGSize cgSize = CGDisplayBounds(CGMainDisplayID()).size;
        size.width = cgSize.width, size.height = cgSize.height;
    }

    pimpl->mouseFactorX = width / size.width;
    pimpl->mouseFactorY = height / size.height;
}

void Gosu::Input::update()
{
    NSPoint mousePos = [NSEvent mouseLocation];
    if (pimpl->window)
    {
        mousePos = [pimpl->window convertScreenToBase: mousePos];
        mousePos.y = [[pimpl->window contentView] frame].size.height - mousePos.y;
    }
    else
        mousePos.y = CGDisplayBounds(CGMainDisplayID()).size.height - mousePos.y;
    pimpl->mouseX = mousePos.x;
    pimpl->mouseY = mousePos.y;
    
    for (unsigned i = 0; i < pimpl->queue.size(); ++i)
    {
        Impl::WaitingButton& wb = pimpl->queue[i];
        buttonStates.at(wb.btn.getId()) = wb.down;
        if (wb.down && onButtonDown)
            onButtonDown(wb.btn);
        else if (!wb.down && onButtonUp)
            onButtonUp(wb.btn);
    }
    pimpl->queue.clear();
}
