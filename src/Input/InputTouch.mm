#include <Gosu/Input.hpp>
#include <Gosu/TextInput.hpp>

#include "MacUtility.hpp"
#include "Input/AccelerometerReader.hpp"
#import <UIKit/UIKit.h>

struct Gosu::TextInput::Impl {};
Gosu::TextInput::TextInput() {}
Gosu::TextInput::~TextInput() {}
std::wstring Gosu::TextInput::text() const { return L""; }
void Gosu::TextInput::setText(const std::wstring& text) {}
unsigned Gosu::TextInput::caretPos() const { return 0; }
unsigned Gosu::TextInput::selectionStart() const { return 0; }

struct Gosu::Input::Impl
{
    UIView* view;
    float mouseX, mouseY;
    float factorX, factorY;
    float updateInterval;
    
    ObjRef<NSMutableSet> currentTouchesSet;
    std::auto_ptr<Gosu::Touches> currentTouchesVector;
    
    Touch translateTouch(UITouch* uiTouch)
    {
        CGPoint point = [uiTouch locationInView: view];
        Touch touch = { uiTouch, point.x, point.y };
        touch.x *= factorX, touch.y *= factorY;
        return touch;
    }
    
    ObjRef<AccelerometerReader> accelerometerReader;
    
    float acceleration(int index)
    {
        if (accelerometerReader.get() == 0)
            accelerometerReader.reset([[AccelerometerReader alloc] initWithUpdateInterval: updateInterval]);
        return [accelerometerReader.obj() acceleration][index];
    }
};

Gosu::Input::Input(void* view, float updateInterval)
: pimpl(new Impl)
{
    pimpl->view = (UIView*)view;
    pimpl->updateInterval = updateInterval;
    pimpl->currentTouchesSet.reset([[NSMutableSet alloc] init]);
    pimpl->mouseX = pimpl->mouseY = -1000;
    setMouseFactors(1, 1);
}

Gosu::Input::~Input()
{
}

void Gosu::Input::feedTouchEvent(int type, void* touches)
{
    NSSet* uiTouches = (NSSet*)touches;
    
    pimpl->currentTouchesVector.reset();
    std::tr1::function<void (Touch)>* f = &onTouchMoved;
    if (type == 0)
        [pimpl->currentTouchesSet.get() unionSet: uiTouches], f = &onTouchBegan;
    else if (type == 2)
        [pimpl->currentTouchesSet.get() minusSet: uiTouches], f = &onTouchEnded;
    for (UITouch* uiTouch in uiTouches)
        if (*f)
            (*f)(pimpl->translateTouch(uiTouch));
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
    return pimpl->mouseX;
}

double Gosu::Input::mouseY() const
{
    return pimpl->mouseY;
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
    if (!pimpl->currentTouchesVector.get())
    {
        pimpl->currentTouchesVector.reset(new Gosu::Touches);
        for (UITouch* uiTouch in pimpl->currentTouchesSet.obj())
            pimpl->currentTouchesVector->push_back(pimpl->translateTouch(uiTouch));
    }
    return *pimpl->currentTouchesVector;
}

double Gosu::Input::accelerometerX() const
{
    return pimpl->acceleration(0);
}

double Gosu::Input::accelerometerY() const
{
    return pimpl->acceleration(1);
}

double Gosu::Input::accelerometerZ() const
{
    return pimpl->acceleration(2);
}

void Gosu::Input::update()
{
    // Check for dead touches and remove from vector if
    // necessary

    ObjRef<NSMutableSet> deadTouches;

    for (UITouch* touch in pimpl->currentTouchesSet.obj())
    {
        UITouchPhase phase = [touch phase];
        if (phase == UITouchPhaseBegan ||
            phase == UITouchPhaseMoved ||
            phase == UITouchPhaseStationary)
            continue;
        
        // Something was deleted, we will need the set.
        if (!deadTouches.get())
            deadTouches.reset([[NSMutableSet alloc] init]);
        [deadTouches.obj() addObject:touch];
    }
    
    // Has something been deleted?
    if (deadTouches.get())
    {
        pimpl->currentTouchesVector.reset();
        [pimpl->currentTouchesSet.obj() minusSet: deadTouches.obj()];
        for (UITouch* uiTouch in deadTouches.obj())
            if (onTouchEnded)
                onTouchEnded(pimpl->translateTouch(uiTouch));
    }
}

Gosu::TextInput* Gosu::Input::textInput() const
{
    return 0;
}

void Gosu::Input::setTextInput(TextInput* input)
{
    throw "NYI";
}
