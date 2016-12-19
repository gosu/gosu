#import <Gosu/Input.hpp>
#import "AppleUtility.hpp"
#import <Gosu/TextInput.hpp>
#import <functional>
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
    UIView *view;
    float mouseX, mouseY;
    float factorX, factorY;
    float updateInterval;
    
    NSMutableSet *currentTouchesSet;
    std::unique_ptr<Gosu::Touches> currentTouchesVector;
    
    Touch translateTouch(UITouch *uiTouch)
    {
        CGPoint point = [uiTouch locationInView:view];
        
        return (Touch) {
            .id = (__bridge void *)uiTouch,
            .x = (float) point.x * factorX,
            .y = (float) point.y * factorY,
        };
    }
};

Gosu::Input::Input(void* view, float updateInterval)
: pimpl(new Impl)
{
    pimpl->view = (__bridge UIView *)view;
    pimpl->updateInterval = updateInterval;
    pimpl->currentTouchesSet = [NSMutableSet new];
    pimpl->mouseX = pimpl->mouseY = -1000;
    setMouseFactors(1, 1);
}

Gosu::Input::~Input()
{
}

void Gosu::Input::feedTouchEvent(int type, void *touches)
{
    NSSet *uiTouches = (__bridge NSSet *)touches;
    
    pimpl->currentTouchesVector.reset();
    
    std::function<void (Touch)>* callback = nullptr;
    
    if (type == 0) {
        [pimpl->currentTouchesSet unionSet:uiTouches];
        callback = &onTouchBegan;
    }
    else if (type == 1) {
        callback = &onTouchMoved;
    }
    else if (type == 2) {
        [pimpl->currentTouchesSet minusSet:uiTouches];
        callback = &onTouchEnded;
    }
    else if (type == 3) {
        [pimpl->currentTouchesSet minusSet:uiTouches];
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
    if (!pimpl->currentTouchesVector.get()) {
        pimpl->currentTouchesVector.reset(new Gosu::Touches);
        for (UITouch* uiTouch in pimpl->currentTouchesSet) {
            pimpl->currentTouchesVector->push_back(pimpl->translateTouch(uiTouch));
        }
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

    NSMutableSet *deadTouches = nil;

    for (UITouch *touch in pimpl->currentTouchesSet)
    {
        UITouchPhase phase = [touch phase];
        if (phase == UITouchPhaseBegan ||
                phase == UITouchPhaseMoved ||
                phase == UITouchPhaseStationary) {
            continue;
        }
        
        // Something was deleted, we will need the set.
        if (!deadTouches) {
            deadTouches = [NSMutableSet new];
        }
        [deadTouches addObject:touch];
    }
    
    // Has something been deleted?
    if (deadTouches)
    {
        pimpl->currentTouchesVector.reset();
        [pimpl->currentTouchesSet minusSet: deadTouches];
        
        if (onTouchEnded) {
            for (UITouch *uiTouch in deadTouches) {
                onTouchEnded(pimpl->translateTouch(uiTouch));
            }
        }
    }
}

Gosu::TextInput* Gosu::Input::textInput() const
{
    return nullptr;
}

void Gosu::Input::setTextInput(TextInput* input)
{
    throw "NYI";
}
