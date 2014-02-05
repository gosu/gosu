#include <Gosu/Input.hpp>
#include <Gosu/TR1.hpp>
#include <SDL.h>

namespace {
    std::tr1::array<bool, Gosu::numButtons> buttonStates = { false };
}

struct Gosu::Input::Impl
{
    Input& input;
    TextInput* textInput;
    double mouseX, mouseY;
    double mouseFactorX, mouseFactorY;
    
    struct WaitingButton
    {
        Button btn;
        bool down;
        WaitingButton(unsigned btnId, bool down) : btn(btnId), down(down) {}
    };
    std::vector<WaitingButton> queue;
    
    Impl(Input& input)
    : input(input), textInput(nullptr), mouseFactorX(1), mouseFactorY(1)
    {
    }
    
    void enqueue(unsigned btnId, bool down)
    {
        queue.push_back(WaitingButton(btnId, down));
    }
    
    void refreshMousePosition()
    {
        int x, y;
        SDL_GetMouseState(&x, &y);
        mouseX = x, mouseY = y;
    }
};

Gosu::Input::Input()
: pimpl(new Impl(*this))
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
    const SDL_Event* e = static_cast<SDL_Event*>(event);
    switch (e->type) {
        case SDL_KEYDOWN:
        case SDL_KEYUP: {
            if (e->key.repeat == 0 && e->key.keysym.scancode <= kbRangeEnd) {
                pimpl->enqueue(e->key.keysym.scancode, e->type == SDL_KEYDOWN);
                return true;
            }
            break;
        }
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP: {
            if (e->button.button <= 2) {
                pimpl->enqueue(msLeft + e->button.button, e->type == SDL_MOUSEBUTTONDOWN);
                return true;
            }
            break;
        }
        case SDL_MOUSEWHEEL: {
            unsigned id = e->wheel.y < 0 ? msWheelUp : msWheelDown;
            pimpl->enqueue(id, true);
            pimpl->enqueue(id, false);
            break;
        }
    }
    return false;
}

wchar_t Gosu::Input::idToChar(Button btn)
{
    return 0;
}

Gosu::Button Gosu::Input::charToId(wchar_t ch)
{
    return Button(Gosu::noButton);
}

bool Gosu::Input::down(Gosu::Button btn) const
{
    if (btn == noButton || btn.id() >= numButtons)
        return false;
    
    return buttonStates[btn.id()];
}

double Gosu::Input::mouseX() const
{
    return pimpl->mouseX * pimpl->mouseFactorX;
}

double Gosu::Input::mouseY() const
{
    return pimpl->mouseY * pimpl->mouseFactorY;
}

void Gosu::Input::setMousePosition(double x, double y)
{
    throw "NYI";
}

void Gosu::Input::setMouseFactors(double factorX, double factorY)
{
    pimpl->mouseFactorX = factorX;
    pimpl->mouseFactorY = factorY;
}

const Gosu::Touches& Gosu::Input::currentTouches() const
{
    static Gosu::Touches none;
    return none;
}

double Gosu::Input::accelerometerX() const
{
    return 0.0;
}

double Gosu::Input::accelerometerY() const
{
    return 0.0;
}

double Gosu::Input::accelerometerZ() const
{
    return 0.0;
}

void Gosu::Input::update()
{
    pimpl->refreshMousePosition();
    
    for (unsigned i = 0; i < pimpl->queue.size(); ++i)
    {
        Impl::WaitingButton& wb = pimpl->queue[i];
        buttonStates[wb.btn.id()] = wb.down;
        if (wb.down && onButtonDown)
            onButtonDown(wb.btn);
        else if (!wb.down && onButtonUp)
            onButtonUp(wb.btn);
    }
    pimpl->queue.clear();
    
    /*std::tr1::array<bool, gpNum> gpState = sys.poll();
    for (unsigned i = 0; i < gpNum; ++i)
    {
        if (buttonStates[i + gpRangeBegin] != gpState[i])
        {
            buttonStates[i + gpRangeBegin] = gpState[i];
            if (gpState[i] && onButtonDown)
                onButtonDown(Button(gpRangeBegin + i));
            else if (!gpState[i] && onButtonUp)
                onButtonUp(Button(gpRangeBegin + i));
        }
    }*/
}

Gosu::TextInput* Gosu::Input::textInput() const
{
    return pimpl->textInput;
}

void Gosu::Input::setTextInput(TextInput* textInput)
{
    pimpl->textInput = textInput;
}
