#include <Gosu/Input.hpp>
#include <Gosu/TextInput.hpp>

#include "AppleUtility.hpp"
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
    
    ObjCRef<NSMutableSet> currentTouchesSet;
    std::auto_ptr<Gosu::Touches> currentTouchesVector;
    
    Touch translateTouch(UITouch* uiTouch)
    {
        CGPoint point = [uiTouch locationInView: view];
        Touch touch = { uiTouch, (float)point.x, (float)point.y };
        touch.x *= factorX, touch.y *= factorY;
        return touch;
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
    NSSet *uiTouches = (NSSet *)touches;
    
    pimpl->currentTouchesVector.reset();
    
    std::tr1::function<void (Touch)>* callback = nullptr;
    
    if (type == 0) {
        [pimpl->currentTouchesSet.get() unionSet:uiTouches];
        callback = &onTouchBegan;
    }
    else if (type == 1) {
        callback = &onTouchMoved;
    }
    else if (type == 2) {
        [pimpl->currentTouchesSet.get() minusSet:uiTouches];
        callback = &onTouchEnded;
    }
    else if (type == 3) {
        [pimpl->currentTouchesSet.get() minusSet:uiTouches];
        callback = &onTouchCancelled;
    }
    
    if (callback && *callback) {
        for (UITouch *uiTouch in uiTouches) {
            (*callback)(pimpl->translateTouch(uiTouch));
        }
    }
}

wchar_t Gosu::Input::idToChar(Button btn)
{
    return 0;
}

Gosu::Button Gosu::Input::charToId(wchar_t ch)
{
    return noButton;
}
        
bool Gosu::Input::down(Button btn)
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

void Gosu::Input::setMouseFactors(double factorX, double factorY, double offsetX, double offsetY) {
    pimpl->factorX = factorX;
    pimpl->factorY = factorY;
    
    // TODO - use offset
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
    // Check for dead touches and remove from vector if
    // necessary

    ObjCRef<NSMutableSet> deadTouches;

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
