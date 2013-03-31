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

void Gosu::TextInput::setCaretPos(unsigned pos)
{
    pimpl->caretPos = pos;
}

unsigned Gosu::TextInput::selectionStart() const
{
    return pimpl->selectionStart;
}

void Gosu::TextInput::setSelectionStart(unsigned pos)
{
    pimpl->selectionStart = pos;
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
    int keysyms_per_keycode_return;
    KeySym keysym = *XGetKeyboardMapping((Display*)display, ev->xkey.keycode, 1, &keysyms_per_keycode_return);
    XConvertCase(keysym, &lower, &upper);

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
        
        wchar_t text[] = { ch, 0 };
        std::wstring filteredText = filter(text);
        pimpl->text.insert(pimpl->text.begin() + CARET_POS, filteredText.begin(), filteredText.end());
        CARET_POS += filteredText.length();
        SEL_START = CARET_POS;
        return true;
    }

    Button btn(ev->xkey.keycode-8);

    // Char left
    if (btn == kbLeft && !ctrlDown)
    {
        if (CARET_POS > 0)
            CARET_POS -= 1;

        if (!shiftDown)
            SEL_START = CARET_POS;
        
        return true;
    }

    // Char right
    if (btn == kbRight && !ctrlDown)
    {
        if (CARET_POS < pimpl->text.length())
            CARET_POS += 1;

        if (!shiftDown)
            SEL_START = CARET_POS;
        
        return true;
    }
    
    // Home
    if (btn == kbHome)
    {
        CARET_POS = 0;

        if (!shiftDown)
            SEL_START = CARET_POS;
        
        return true;
    }

    // End
    if (btn == kbEnd)
    {
        CARET_POS = pimpl->text.length();

        if (!shiftDown)
            SEL_START = CARET_POS;
        
        return true;
    }

    // Word left
    if (btn == kbLeft && ctrlDown)
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
    if (btn == kbRight && ctrlDown)
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
    if (btn == kbBackspace)
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
    if (btn == kbDelete)
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
