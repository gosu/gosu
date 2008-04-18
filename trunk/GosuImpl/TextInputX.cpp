#include <Gosu/TextInput.hpp>
#include <Gosu/Input.hpp>
#include <algorithm>
#include <vector>
#include <wctype.h>

struct Gosu::TextInput::Impl
{
    std::wstring text;
    unsigned caretPos, selectionStart;
    Impl() : caretPos(0), selectionStart(0) {}
};

Gosu::TextInput::TextInput()
: pimpl(new Impl)
{
}

Gosu::TextInput::~TextInput()
{
}

std::wstring Gosu::TextInput::text() const
{
    return pimpl->text;
}

void Gosu::TextInput::setText(const std::wstring& text)
{
	pimpl->text = text;
	pimpl->caretPos = pimpl->selectionStart = text.length();
}

unsigned Gosu::TextInput::caretPos() const
{
    return pimpl->caretPos;
}

unsigned Gosu::TextInput::selectionStart() const
{
    return pimpl->selectionStart;
}

#define CARET_POS (pimpl->caretPos)
#define SEL_START (pimpl->selectionStart)

bool Gosu::TextInput::feedXEvent(void* display, void* event)
{
    XEvent* ev = static_cast<XEvent*>(event);

    if (ev->type != KeyPress)
        return false;

    bool ctrlDown	= (ev->xkey.state & ControlMask);
    bool shiftDown	= (ev->xkey.state & ShiftMask);

    KeySym lower, upper;
    XConvertCase(XKeycodeToKeysym((Display*)display, ev->xkey.keycode, 0), &lower, &upper);

    wchar_t ch = static_cast<wchar_t>(shiftDown ? upper : lower);

    if (ch >= 32 && ch != 127 && ch <= 255)
    {
        // Delete (overwrite) previous selection.
        if (CARET_POS != SEL_START)
        {
            unsigned min = std::min(CARET_POS, SEL_START);
            unsigned max = std::max(CARET_POS, SEL_START);
            pimpl->text.erase(pimpl->text.begin() + min, pimpl->text.begin() + max);
            CARET_POS = SEL_START = min;
        }
        
        pimpl->text.insert(pimpl->text.begin() + CARET_POS, ch);
        CARET_POS += 1;
        SEL_START = CARET_POS;
        return true;
    }

    // Char left
    if (ch == kbLeft && !ctrlDown)
    {
        if (CARET_POS > 0)
            CARET_POS -= 1;

        if (!shiftDown)
            SEL_START = CARET_POS;
        
        return true;
    }

    // Char right
    if (ch == kbRight && !ctrlDown)
    {
        if (CARET_POS < pimpl->text.length())
            CARET_POS += 1;

        if (!shiftDown)
            SEL_START = CARET_POS;
        
        return true;
    }
    
    // Home
    if (ch == kbHome)
    {
        CARET_POS = 0;

        if (!shiftDown)
            SEL_START = CARET_POS;
        
        return true;
    }

    // End
    if (ch == kbEnd)
    {
        CARET_POS = pimpl->text.length();

        if (!shiftDown)
            SEL_START = CARET_POS;
        
        return true;
    }

    // Word left
    if (ch == kbLeft && ctrlDown)
    {
        if (CARET_POS == pimpl->text.length())
            --CARET_POS;
        
        while (CARET_POS > 0 && iswspace(pimpl->text.at(CARET_POS - 1)))
            --CARET_POS;

        while (CARET_POS > 0 && !iswspace(pimpl->text.at(CARET_POS - 1)))
            --CARET_POS;

        if (!shiftDown)
            SEL_START = CARET_POS;
        
        return true;
    }

    // Word right
    if (ch == kbRight && ctrlDown)
    {
        while (CARET_POS < pimpl->text.length() && iswspace(pimpl->text.at(CARET_POS)))
            ++CARET_POS;

        while (CARET_POS < pimpl->text.length() && !iswspace(pimpl->text.at(CARET_POS)))
            ++CARET_POS;

        if (!shiftDown)
            SEL_START = CARET_POS;

        return true;
    }

    // Delete existant selection
    if (ch == kbBackspace)
    {
        if (SEL_START != CARET_POS)
        {
            unsigned min = std::min(CARET_POS, SEL_START);
            unsigned max = std::max(CARET_POS, SEL_START);
            pimpl->text.erase(pimpl->text.begin() + min, pimpl->text.begin() + max);
            SEL_START = CARET_POS = min;
        }
        else if (CARET_POS > 0)
        {
            unsigned oldCaret = CARET_POS;
            CARET_POS -= 1;
            pimpl->text.erase(pimpl->text.begin() + CARET_POS, pimpl->text.begin() + oldCaret);
            SEL_START = CARET_POS;
        }

        return true;
    }

    // Delete existant selection
    if (ch == kbDelete)
    {
        if (SEL_START != CARET_POS)
        {
            unsigned min = std::min(CARET_POS, SEL_START);
            unsigned max = std::max(CARET_POS, SEL_START);
            pimpl->text.erase(pimpl->text.begin() + min, pimpl->text.begin() + max);
            SEL_START = CARET_POS = min;
        }
        else if (CARET_POS < pimpl->text.length())
        {
            unsigned oldCaret = CARET_POS;
            CARET_POS += 1;
            pimpl->text.erase(pimpl->text.begin() + oldCaret, pimpl->text.begin() + CARET_POS);
            SEL_START = CARET_POS = oldCaret;
        }

        return true;
    }

    return false;
}
