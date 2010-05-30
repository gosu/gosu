#include <Gosu/TextInput.hpp>
#include <Gosu/ButtonsWin.hpp>
#include <algorithm>
#include <vector>
#include <wctype.h>

// OPT-IN, have you heard of it?!
#define NOMINMAX
#include <windows.h>

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

bool Gosu::TextInput::feedMessage(unsigned long message, unsigned long wparam, unsigned long lparam)
{
    if (message == WM_CHAR && wparam >= 32 && wparam != 127)
    {
        // Delete (overwrite) previous selection.
        if (CARET_POS != SEL_START)
        {
            unsigned min = std::min(CARET_POS, SEL_START);
            unsigned max = std::max(CARET_POS, SEL_START);
            pimpl->text.erase(pimpl->text.begin() + min, pimpl->text.begin() + max);
            CARET_POS = SEL_START = min;
        }
        
        wchar_t text[] = { static_cast<wchar_t>(wparam), 0 };
        std::wstring filteredText = filter(text);
        pimpl->text.insert(pimpl->text.begin() + CARET_POS, filteredText.begin(), filteredText.end());
        CARET_POS += filteredText.length();
        SEL_START = CARET_POS;
        return true;
    }

    bool ctrlDown	= (GetKeyState(VK_CONTROL) < 0);
	bool shiftDown	= (GetKeyState(VK_SHIFT) < 0);

    // Char left
    if (message == WM_KEYDOWN && wparam == VK_LEFT && !ctrlDown)
    {
        if (CARET_POS > 0)
            CARET_POS -= 1;

        if (!shiftDown)
            SEL_START = CARET_POS;
        
        return true;
    }

    // Char right
    if (message == WM_KEYDOWN && wparam == VK_RIGHT && !ctrlDown)
    {
        if (CARET_POS < pimpl->text.length())
            CARET_POS += 1;

        if (!shiftDown)
            SEL_START = CARET_POS;
        
        return true;
    }
    
    // Home
    if (message == WM_KEYDOWN && wparam == VK_HOME)
    {
        CARET_POS = 0;

        if (!shiftDown)
            SEL_START = CARET_POS;
        
        return true;
    }

    // End
    if (message == WM_KEYDOWN && wparam == VK_END)
    {
        CARET_POS = pimpl->text.length();

        if (!shiftDown)
            SEL_START = CARET_POS;
        
        return true;
    }

    // Word left
    if (message == WM_KEYDOWN && wparam == VK_LEFT && ctrlDown)
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
    if (message == WM_KEYDOWN && wparam == VK_RIGHT && ctrlDown)
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
    if (message == WM_KEYDOWN && wparam == VK_BACK)
    {
        if (SEL_START != CARET_POS)
        {
            unsigned min = std::min(CARET_POS, SEL_START);
            unsigned max = std::max(CARET_POS, SEL_START);
            pimpl->text.erase(pimpl->text.begin() + min, pimpl->text.begin() + max);
            SEL_START = CARET_POS = min;
        }
        else
        {
            unsigned oldCaret = CARET_POS;
            // Move left - either char or word
            feedMessage(WM_KEYDOWN, VK_LEFT, lparam);
            pimpl->text.erase(pimpl->text.begin() + CARET_POS, pimpl->text.begin() + oldCaret);
            SEL_START = CARET_POS;
        }

        return true;
    }

    // Delete existant selection
    if (message == WM_KEYDOWN && wparam == VK_DELETE)
    {
        if (SEL_START != CARET_POS)
        {
            unsigned min = std::min(CARET_POS, SEL_START);
            unsigned max = std::max(CARET_POS, SEL_START);
            pimpl->text.erase(pimpl->text.begin() + min, pimpl->text.begin() + max);
            SEL_START = CARET_POS = min;
        }
        else
        {
            unsigned oldCaret = CARET_POS;
            // Move right - either char or word
            feedMessage(WM_KEYDOWN, VK_RIGHT, lparam);
            pimpl->text.erase(pimpl->text.begin() + oldCaret, pimpl->text.begin() + CARET_POS);
            SEL_START = CARET_POS = oldCaret;
        }

        return true;
    }

    return false;
}
