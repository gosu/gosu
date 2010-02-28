#include <Gosu/Input.hpp>
#include <Gosu/TextInput.hpp>

struct Gosu::TextInput::Impl {};
Gosu::TextInput::TextInput() {}
Gosu::TextInput::~TextInput() {}
std::wstring Gosu::TextInput::text() const { return L""; }
void Gosu::TextInput::setText(const std::wstring& text) {}
unsigned Gosu::TextInput::caretPos() const { return 0; }
unsigned Gosu::TextInput::selectionStart() const { return 0; }

struct Gosu::Input::Impl {
};

Gosu::Input::Input() {
}

Gosu::Input::~Input() {
}

wchar_t Gosu::Input::idToChar(Button btn) {
    return 0;
}

Gosu::Button Gosu::Input::charToId(wchar_t ch) {
    return noButton;
}
        
bool Gosu::Input::down(Button btn) const {
    return false;
}

double Gosu::Input::mouseX() const {
    return -1000;
}

double Gosu::Input::mouseY() const {
    return -1000;
}

void Gosu::Input::setMousePosition(double x, double y) {
}

void Gosu::Input::setMouseFactors(double factorX, double factorY) {
}

void Gosu::Input::update() {
}

Gosu::TextInput* Gosu::Input::textInput() const {
    return 0;
}

void Gosu::Input::setTextInput(TextInput* input) {
    throw "NYI";
}
