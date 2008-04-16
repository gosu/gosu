#include <Gosu/Input.hpp>

// This is a very stupid dummy implementation that
// will hopefully only be used for prototyping.

// It does not allow for cursor movement, selection or
// anything else than appending and deleting characters.

struct Gosu::TextInput:Impl
{
    std::wstring text;
};

Gosu::TextInput::TextInput()
: pimpl(new Impl)
{
}

Gosu::TextInput::~TextInput()
{
}

bool feedEvent(void* event)
{
    unsigned buttonId = reinterpret_cast<unsigned>(event);
    
    if (buttonId == KbBackspace)
    {
        pimpl->text.resize(pimpl->text.size() - 1);
        return true;
    }
    
    wchar_t chr = Input::idToChar(Button(buttonId));
    if (chr != 0)
    {
        pimpl->text += chr;
        return true;
    }
    
    return false;
}

std::wstring Gosu::TextInput::text() const
{
    return pimpl->text;
}

void Gosu::TextInput::setText(const std::wstring& text)
{
    pimpl->text = text;
}

std::wstring Gosu::TextInput::caretPos() const
{
    return pimpl->text.length();
}

std::wstring Gosu::TextInput::selStart() const
{
    return pimpl->text.length();
}
