#include <Gosu/TextInput.hpp>
#include <Gosu/Input.hpp>
#include <Gosu/Platform.hpp>

#ifndef GOSU_IS_IPHONE
#include <SDL.h>
#include <cctype>
#endif

using namespace std;

struct Gosu::TextInput::Impl
{
    string text;
    
    // This is the current IME composition (not used on iOS).
    // http://wiki.libsdl.org/Tutorials/TextInput#CandidateList
    string composition;
    
    // Indices into the UTF-8 encoded text.
    unsigned caret_pos = 0, selection_start = 0;
    
    // Skip continuation characters, see: https://en.wikipedia.org/wiki/UTF-8#Description
    // (0xc0 = 11'000000, 0x80 = 10'000000)
    bool should_skip(char ch)
    {
        return (static_cast<unsigned char>(ch) & 0xc0) == 0x80;
    }
    
    void insert_text(const string& new_text)
    {
        // Stop IME composition.
        composition.clear();

        // Delete (overwrite) previous selection.
        if (caret_pos != selection_start) {
            unsigned from = min(caret_pos, selection_start);
            unsigned to = max(caret_pos, selection_start);
            text.erase(text.begin() + from, text.begin() + to);
            caret_pos = selection_start = from;
        }
        
        text.insert(text.begin() + caret_pos, new_text.begin(), new_text.end());
        caret_pos += new_text.size();
        selection_start = caret_pos;
    }
    
    void move_left(bool modify_selection)
    {
        if (caret_pos > 0) {
            caret_pos -= 1;
            
            // Skip UTF-8 continuation bytes.
            while (caret_pos > 0 && should_skip(text[caret_pos])) {
                caret_pos -= 1;
            }
        }
        
        if (modify_selection) {
            selection_start = caret_pos;
        }
    }
    
    void move_right(bool modify_selection)
    {
        if (caret_pos < text.length()) {
            caret_pos += 1;
            
            // Skip UTF-8 continuation bytes.
            while (caret_pos < text.length() && should_skip(text[caret_pos])) {
                caret_pos += 1;
            }
        }
        
        if (modify_selection) {
            selection_start = caret_pos;
        }
    }
    
    void move_word_left(bool modify_selection)
    {
        while (caret_pos > 0 && isspace(text[caret_pos - 1])) {
            move_left(modify_selection);
        }
        while (caret_pos > 0 && !isspace(text[caret_pos - 1])) {
            move_left(modify_selection);
        }
    }

    void move_word_right(bool modify_selection)
    {
        while (caret_pos < text.length() && isspace(text.at(caret_pos))) {
            move_right(modify_selection);
        }
        while (caret_pos < text.length() && !isspace(text.at(caret_pos))) {
            move_right(modify_selection);
        }
    }
    
    void move_to_beginning_of_line(bool modify_selection)
    {
        caret_pos = 0;
        
        if (modify_selection) {
            selection_start = caret_pos;
        }
    }
    
    void move_to_end_of_line(bool modify_selection)
    {
        caret_pos = static_cast<unsigned>(text.length());
        
        if (modify_selection) {
            selection_start = caret_pos;
        }
    }
    
    void delete_backward()
    {
        if (selection_start != caret_pos) {
            unsigned from = min(caret_pos, selection_start);
            unsigned to = max(caret_pos, selection_start);
            text.erase(text.begin() + from, text.begin() + to);
            selection_start = caret_pos = from;
        }
        else if (caret_pos > 0) {
            move_left(false);
            text.erase(text.begin() + caret_pos, text.begin() + selection_start);
            selection_start = caret_pos;
        }
    }
    
    void delete_forward()
    {
        if (selection_start != caret_pos) {
            unsigned from = min(caret_pos, selection_start);
            unsigned to = max(caret_pos, selection_start);
            text.erase(text.begin() + from, text.begin() + to);
            selection_start = caret_pos = from;
        }
        else if (caret_pos < text.length()) {
            move_right(false);
            text.erase(text.begin() + selection_start, text.begin() + caret_pos);
            caret_pos = selection_start;
        }
    }
};

Gosu::TextInput::TextInput()
: pimpl(new Impl)
{
}

Gosu::TextInput::~TextInput()
{
    // TODO: Unset Input::text_input to avoid stale pointers?
}

string Gosu::TextInput::text() const
{
    string composed_text = pimpl->text;
    if (!pimpl->composition.empty()) {
        composed_text.insert(pimpl->caret_pos, pimpl->composition);
    }
    return composed_text;
}

void Gosu::TextInput::set_text(const string& text)
{
    pimpl->text = text;
    pimpl->composition.clear();
    pimpl->caret_pos = pimpl->selection_start = static_cast<unsigned>(pimpl->text.length());
}

unsigned Gosu::TextInput::caret_pos() const
{
    return pimpl->caret_pos;
}

void Gosu::TextInput::set_caret_pos(unsigned caret_pos)
{
    pimpl->caret_pos = caret_pos;
}

unsigned Gosu::TextInput::selection_start() const
{
    return pimpl->selection_start;
}

void Gosu::TextInput::set_selection_start(unsigned selection_start)
{
    pimpl->selection_start = selection_start;
}

#ifndef GOSU_IS_IPHONE
bool Gosu::TextInput::feed_sdl_event(void* event)
{
    const SDL_Event* e = static_cast<SDL_Event*>(event);
    
    switch (e->type) {
    // Direct text input or completed IME composition.
    case SDL_TEXTINPUT: {
        pimpl->insert_text(filter(e->text.text));
        return true;
    }
    // IME composition in progress.
    case SDL_TEXTEDITING: {
        pimpl->composition = e->edit.text;
        return true;
    }
    // Emulate "standard" Windows/Linux keyboard behavior.
    case SDL_KEYDOWN: {
        // ...but not if the IME is currently compositing.
        if (!pimpl->composition.empty()) return false;

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
            if (command_down) {
                pimpl->move_to_beginning_of_line(!shift_down);
                return true;
            }
        #endif
            if (words) {
                pimpl->move_word_left(!shift_down);
            }
            else {
                pimpl->move_left(!shift_down);
            }
            return true;
        case SDLK_RIGHT:
        #ifdef GOSU_IS_MAC
            if (command_down) {
                pimpl->move_to_end_of_line(!shift_down);
                return true;
            }
        #endif
            if (words) {
                pimpl->move_word_right(!shift_down);
            }
            else {
                pimpl->move_right(!shift_down);
            }
            return true;
        #ifdef GOSU_IS_MAC
        case SDLK_UP:
        #endif
        case SDLK_HOME:
            pimpl->move_to_beginning_of_line(!shift_down);
            return true;
        #ifdef GOSU_IS_MAC
        case SDLK_DOWN:
        #endif
        case SDLK_END:
            pimpl->move_to_end_of_line(!shift_down);
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
#endif

void Gosu::TextInput::insert_text(string text)
{
    pimpl->insert_text(text);
}

void Gosu::TextInput::delete_forward()
{
    pimpl->delete_forward();
}

void Gosu::TextInput::delete_backward()
{
    pimpl->delete_backward();
}
