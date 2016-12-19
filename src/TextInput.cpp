#include <Gosu/TextInput.hpp>
#include <Gosu/Input.hpp>
#include <Gosu/Platform.hpp>
#include <Gosu/Utility.hpp>
#include <SDL.h>
#include <cwctype>

struct Gosu::TextInput::Impl
{
    // These two strings contain UTF-8 data.
    // See this Wiki page for an overview of what is going on here:
    // http://wiki.libsdl.org/Tutorials/TextInput#CandidateList
    std::wstring text, composition;
    unsigned caret_pos, selection_start;
    
    Impl()
    : caret_pos(0), selection_start(0)
    {
    }
    
    void insert_text(const std::wstring& new_text)
    {
        // Stop IME composition.
        composition.clear();

        // Delete (overwrite) previous selection.
        if (caret_pos != selection_start) {
            unsigned min = std::min(caret_pos, selection_start);
            unsigned max = std::max(caret_pos, selection_start);
            text.erase(text.begin() + min, text.begin() + max);
            caret_pos = selection_start = min;
        }
        
        text.insert(text.begin() + caret_pos, new_text.begin(), new_text.end());
        caret_pos += new_text.size();
        selection_start = caret_pos;
    }
    
    void move_left(bool modify_selection)
    {
        if (caret_pos > 0)
            caret_pos -= 1;
        
        if (modify_selection)
            selection_start = caret_pos;
    }
    
    void move_right(bool modify_selection)
    {
        if (caret_pos < text.length())
            caret_pos += 1;
        
        if (modify_selection)
            selection_start = caret_pos;
    }
    
    void move_word_left(bool modify_selection)
    {
        if (caret_pos == text.length())
            --caret_pos;
        
        while (caret_pos > 0 && std::iswspace(text.at(caret_pos - 1)))
            --caret_pos;
        
        while (caret_pos > 0 && !std::iswspace(text.at(caret_pos - 1)))
            --caret_pos;
        
        if (modify_selection)
            selection_start = caret_pos;
    }

    void move_word_right(bool modify_selection)
    {
        while (caret_pos < text.length() && std::iswspace(text.at(caret_pos)))
            ++caret_pos;
        
        while (caret_pos < text.length() && !std::iswspace(text.at(caret_pos)))
            ++caret_pos;
        
        if (modify_selection)
            selection_start = caret_pos;
    }
    
    void move_to_beginning_ofLine(bool modify_selection)
    {
        caret_pos = 0;
        
        if (modify_selection)
            selection_start = caret_pos;
    }
    
    void move_to_end_ofLine(bool modify_selection)
    {
        caret_pos = static_cast<unsigned>(text.length());
        
        if (modify_selection)
            selection_start = caret_pos;
    }
    
    void delete_backward()
    {
        if (selection_start != caret_pos) {
            unsigned min = std::min(caret_pos, selection_start);
            unsigned max = std::max(caret_pos, selection_start);
            text.erase(text.begin() + min, text.begin() + max);
            selection_start = caret_pos = min;
        }
        else if (caret_pos > 0) {
            unsigned old_caret = caret_pos;
            caret_pos -= 1;
            text.erase(text.begin() + caret_pos, text.begin() + old_caret);
            selection_start = caret_pos;
        }
    }
    
    void delete_forward()
    {
        if (selection_start != caret_pos) {
            unsigned min = std::min(caret_pos, selection_start);
            unsigned max = std::max(caret_pos, selection_start);
            text.erase(text.begin() + min, text.begin() + max);
            selection_start = caret_pos = min;
        }
        else if (caret_pos < text.length()) {
            unsigned old_caret = caret_pos;
            caret_pos += 1;
            text.erase(text.begin() + old_caret, text.begin() + caret_pos);
            selection_start = caret_pos = old_caret;
        }
        
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
    std::wstring composed_text = pimpl->text;
    if (! pimpl->composition.empty()) {
        composed_text.insert(pimpl->caret_pos, pimpl->composition);
    }
    return composed_text;
}

void Gosu::TextInput::set_text(const std::wstring& text)
{
	pimpl->text = text;
    pimpl->composition.clear();
	pimpl->caret_pos = pimpl->selection_start = static_cast<unsigned>(text.length());
}

unsigned Gosu::TextInput::caret_pos() const
{
    return static_cast<unsigned>(pimpl->caret_pos);
}

void Gosu::TextInput::set_caret_pos(unsigned pos)
{
    pimpl->caret_pos = pos;
}

unsigned Gosu::TextInput::selection_start() const
{
    return static_cast<unsigned>(pimpl->selection_start);
}

void Gosu::TextInput::set_selection_start(unsigned pos)
{
    pimpl->selection_start = pos;
}

bool Gosu::TextInput::feed_sdl_event(void* event)
{
    const SDL_Event* e = static_cast<SDL_Event*>(event);
    
    switch (e->type) {
        // Direct text input, and sent after IME composition completes.
        case SDL_TEXTINPUT: {
            std::wstring text_to_insert = utf8_to_wstring(e->text.text);
            text_to_insert = filter(text_to_insert);
            pimpl->insert_text(text_to_insert);
            return true;
        }
        // IME composition in progress.
        case SDL_TEXTEDITING: {
            pimpl->composition = utf8_to_wstring(e->edit.text);
            return true;
        }
        // Emulate "standard" Windows/X11 keyboard behavior.
        case SDL_KEYDOWN: {
            // ...but not if the IME is currently compositing.
            if (! pimpl->composition.empty()) {
                return false;
            }
            
        #ifdef GOSU_IS_MAC
            bool words = (e->key.keysym.mod & (KMOD_LALT | KMOD_RALT));
            bool command_down = (e->key.keysym.mod & (KMOD_LGUI | KMOD_RGUI));
        #else
            bool words = (e->key.keysym.mod & (KMOD_LCTRL | KMOD_RCTRL));
        #endif
            bool shift_down = (e->key.keysym.mod & (KMOD_LSHIFT | KMOD_RSHIFT));
            SDL_Keycode key = e->key.keysym.sym;
            
            switch (key) {
                case SDLK_LEFT:
                #ifdef GOSU_IS_MAC
                    if (command_down)
                        pimpl->move_to_beginning_ofLine(! shift_down);
                    else
                #endif
                    if (words)
                        pimpl->move_word_left(! shift_down);
                    else
                        pimpl->move_left(! shift_down);
                    return true;
                case SDLK_RIGHT:
                #ifdef GOSU_IS_MAC
                    if (command_down)
                        pimpl->move_to_end_ofLine(! shift_down);
                    else
                #endif
                    if (words)
                        pimpl->move_word_right(! shift_down);
                    else
                        pimpl->move_right(! shift_down);
                    return true;
            #ifdef GOSU_IS_MAC
                case SDLK_UP:
            #endif
                case SDLK_HOME:
                    pimpl->move_to_beginning_ofLine(! shift_down);
                    return true;
            #ifdef GOSU_IS_MAC
                case SDLK_DOWN:
            #endif
                case SDLK_END:
                    pimpl->move_to_end_ofLine(! shift_down);
                    return true;
                case SDLK_BACKSPACE:
                    pimpl->delete_backward();
                    return true;
                case SDLK_DELETE:
                    pimpl->delete_forward();
                    return true;
            }
            break;
        }
            
        // TODO: Handle copy & paste.
    }
    
    return false;
}
