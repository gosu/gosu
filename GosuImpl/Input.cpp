#include <Gosu/Input.hpp>
#include <SDL.h>

Gosu::Input::Input()
{
    SDL_InitSubSystem(SDL_INIT_JOYSTICK);
}

Gosu::Input::~Input()
{
    // TODO should be ref-counted or moved out of here.
    SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
}

bool Gosu::Input::feedSDLEvent(void* event)
{
    const SDL_Event* e = static_cast<SDL_Event *>(event);
    switch (e->type) {
    case SDL_KEYDOWN:
        return true;
    default:
        return false;
    }
}

wchar_t Gosu::Input::idToChar(Button btn)
{
    return 0;
}

Gosu::Button Gosu::Input::charToId(wchar_t ch)
{
    return Button(Gosu::noButton);
}

bool Gosu::Input::down(Button btn) const
{
    return false;
}

double Gosu::Input::mouseX() const
{
    return 0;
}

double Gosu::Input::mouseY() const
{
    return 0;
}

void Gosu::Input::setMousePosition(double x, double y)
{
    
}

void setMouseFactors(double factorX, double factorY)
{
}

const Gosu::Touches& Gosu::Input::currentTouches() const
{
    static Gosu::Touches empty;
    return empty;
}

double Gosu::Input::accelerometerX() const
{
    return 0;
}

double Gosu::Input::accelerometerY() const
{
    return 0;
}

double Gosu::Input::accelerometerZ() const
{
    return 0;
}

void Gosu::Input::update()
{
    
}

Gosu::TextInput* Gosu::Input::textInput() const
{
    return nullptr;
}

void Gosu::Input::setTextInput(TextInput* input)
{
}
