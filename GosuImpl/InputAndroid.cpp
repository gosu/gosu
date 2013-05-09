#include <Gosu/Input.hpp>
#include <Gosu/TextInput.hpp>

struct Gosu::TextInput::Impl {};
Gosu::TextInput::TextInput() {}
Gosu::TextInput::~TextInput() {}
std::wstring Gosu::TextInput::text() const { return L""; }
void Gosu::TextInput::setText(const std::wstring& text) {}
unsigned Gosu::TextInput::caretPos() const { return 0; }
unsigned Gosu::TextInput::selectionStart() const { return 0; }

struct Gosu::Input::Impl
{
    float mouseX, mouseY;
    float factorX, factorY;
};

Gosu::Input::Input()
: pimpl(new Impl)
{
    pimpl->mouseX = pimpl->mouseY = -1000;
    setMouseFactors(1, 1);
}

Gosu::Input::~Input()
{
}

wchar_t Gosu::Input::idToChar(Button btn)
{
    return 0;
}

Gosu::Button Gosu::Input::charToId(wchar_t ch)
{
    return noButton;
}
        
bool Gosu::Input::down(Button btn) const
{
    return false;
}

double Gosu::Input::mouseX() const
{
    return pimpl->mouseX * pimpl->factorX;
}

double Gosu::Input::mouseY() const
{
    return pimpl->mouseY * pimpl->factorY;
}

void Gosu::Input::setMousePosition(double x, double y) {
    pimpl->mouseX = x;
    pimpl->mouseY = y;
}

void Gosu::Input::setMouseFactors(double factorX, double factorY) {
    pimpl->factorX = factorX;
    pimpl->factorY = factorY;
}

const Gosu::Touches& Gosu::Input::currentTouches() const
{
    static Gosu::Touches none;
    return none;
}

namespace Gosu {
    float accelX = 0, accelY = 0, accelZ = 0;
}

double Gosu::Input::accelerometerX() const
{
    return accelX;
}

double Gosu::Input::accelerometerY() const
{
    return accelY;
}

double Gosu::Input::accelerometerZ() const
{
    return accelZ;
}

void Gosu::Input::update()
{
}

Gosu::TextInput* Gosu::Input::textInput() const
{
    return 0;
}

void Gosu::Input::setTextInput(TextInput* input)
{
    throw "NYI";
}
