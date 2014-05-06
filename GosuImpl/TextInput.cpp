#include <Gosu/TextInput.hpp>
#include <Gosu/Utility.hpp>
#include <SDL.h>

struct Gosu::TextInput::Impl
{
    // These two strings contain UTF-8 data.
    // See this Wiki page for an idea of what is going on here:
    // http://wiki.libsdl.org/Tutorials/TextInput#CandidateList
    std::wstring text, composition;
    unsigned caretPos, selectionStart;
    
    Impl()
    : caretPos(0), selectionStart(0)
    {
    }
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
    std::wstring composedText = pimpl->text;
    if (! pimpl->composition.empty()) {
        composedText.insert(pimpl->caretPos, pimpl->composition);
    }
    return composedText;
}

void Gosu::TextInput::setText(const std::wstring& text)
{
	pimpl->text = text;
    pimpl->composition.clear();
	pimpl->caretPos = pimpl->selectionStart = static_cast<unsigned>(text.length());
}

unsigned Gosu::TextInput::caretPos() const
{
    return static_cast<unsigned>(pimpl->caretPos);
}

void Gosu::TextInput::setCaretPos(unsigned pos)
{
    pimpl->caretPos = pos;
}

unsigned Gosu::TextInput::selectionStart() const
{
    return static_cast<unsigned>(pimpl->selectionStart);
}

void Gosu::TextInput::setSelectionStart(unsigned pos)
{
    pimpl->selectionStart = pos;
}

bool Gosu::TextInput::feedSDLEvent(void* event)
{
    const SDL_Event* e = static_cast<SDL_Event*>(event);
    
    switch (e->type) {
        case SDL_TEXTINPUT: {
            pimpl->composition.clear();
            std::wstring insertedText = utf8ToWstring(e->text.text);
            pimpl->text.insert(pimpl->caretPos, insertedText);
            pimpl->caretPos += insertedText.length();
            return true;
        }
        case SDL_TEXTEDITING: {
            pimpl->composition = utf8ToWstring(e->edit.text);
            return true;
        }
    }
    
    return false;
}
