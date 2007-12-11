#include <Gosu/Input.hpp>

/*

Not meant to be compiled, taken out until 0.7.9.

Relies on addition to Gosu/Input.hpp:

    // One TextInput instance can be selected per Input which will then translate
    // events into a string.
    class TextInput
    {
        struct Impl;
        boost::scoped_ptr<Impl> pimpl;
        
    public:
        TextInput(class Input& input);
        virtual ~TextInput();
        
        // Called by Input. Type of events depends on platform.
        bool feedEvent(void* event);
        
        std::wstring text() const;
        unsigned caretPos() const;
        unsigned selStartPos() const;
    };

Also on the fact that     

*/

// This is a very stupid dummy implementation that
// will hopefully only be used for prototyping.

// It does not allow for cursor movement, selection or
// anything else than appending and deleting characters.

struct Gosu::TextInput:Impl
{
    Input* input;
    std::wstring text;
};

Gosu::TextInput::TextInput(Input& input)
: pimpl(new Impl)
{
    pimpl->input = &input;
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
    
    wchar_t chr = pimpl->input->idToChar(Button(buttonId));
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

std::wstring Gosu::TextInput::caretPos() const
{
    return pimpl->text.length();
}

std::wstring Gosu::TextInput::selStartPos() const
{
    return pimpl->text.length();
}
