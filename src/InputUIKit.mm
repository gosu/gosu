#import <Gosu/Input.hpp>
#import "AppleUtility.hpp"
#import <Gosu/TextInput.hpp>
#import <functional>
#import <UIKit/UIKit.h>

struct Gosu::TextInput::Impl {};
Gosu::TextInput::TextInput() {}
Gosu::TextInput::~TextInput() {}
std::wstring Gosu::TextInput::text() const { return L""; }
void Gosu::TextInput::set_text(const std::wstring& text) {}
unsigned Gosu::TextInput::caret_pos() const { return 0; }
unsigned Gosu::TextInput::selection_start() const { return 0; }

struct Gosu::Input::Impl
{
    UIView *view;
    float mouse_x, mouse_y;
    float scale_x, scale_y;
    float update_interval;
    
    NSMutableSet *current_touches_set;
    std::unique_ptr<Gosu::Touches> current_touches_vector;
    
    Touch translate_touch(UITouch *ui_touch)
    {
        CGPoint point = [ui_touch locationInView:view];
        
        return (Touch) {
            .id = (__bridge void *)ui_touch,
            .x = (float) point.x * scale_x,
            .y = (float) point.y * scale_y,
        };
    }
};

Gosu::Input::Input(void* view, float update_interval)
: pimpl(new Impl)
{
    pimpl->view = (__bridge UIView *)view;
    pimpl->update_interval = update_interval;
    pimpl->current_touches_set = [NSMutableSet new];
    pimpl->mouse_x = pimpl->mouse_y = -1000;
    set_mouse_factors(1, 1);
}

Gosu::Input::~Input()
{
}

void Gosu::Input::feed_touch_event(int type, void *touches)
{
    NSSet *ui_touches = (__bridge NSSet *)touches;
    
    pimpl->current_touches_vector.reset();
    
    std::function<void (Touch)>* callback = nullptr;
    
    if (type == 0) {
        [pimpl->current_touches_set unionSet:ui_touches];
        callback = &on_touch_began;
    }
    else if (type == 1) {
        callback = &on_touch_moved;
    }
    else if (type == 2) {
        [pimpl->current_touches_set minusSet:ui_touches];
        callback = &on_touch_ended;
    }
    else if (type == 3) {
        [pimpl->current_touches_set minusSet:ui_touches];
        callback = &on_touch_cancelled;
    }
    
    if (callback && *callback) {
        for (UITouch *ui_touch in ui_touches) {
            (*callback)(pimpl->translate_touch(ui_touch));
        }
    }
}

wchar_t Gosu::Input::id_to_char(Button btn)
{
    return 0;
}

Gosu::Button Gosu::Input::char_to_id(wchar_t ch)
{
    return NO_BUTTON;
}
        
bool Gosu::Input::down(Button btn)
{
    return false;
}

double Gosu::Input::mouse_x() const
{
    return pimpl->mouse_x;
}

double Gosu::Input::mouse_y() const
{
    return pimpl->mouse_y;
}

void Gosu::Input::set_mouse_position(double x, double y) {
    pimpl->mouse_x = x;
    pimpl->mouse_y = y;
}

void Gosu::Input::set_mouse_factors(double scale_x, double scale_y, double offset_x, double offset_y) {
    pimpl->scale_x = scale_x;
    pimpl->scale_y = scale_y;
    
    // TODO - use offset
}

const Gosu::Touches& Gosu::Input::current_touches() const
{
    if (!pimpl->current_touches_vector.get()) {
        pimpl->current_touches_vector.reset(new Gosu::Touches);
        for (UITouch* ui_touch in pimpl->current_touches_set) {
            pimpl->current_touches_vector->push_back(pimpl->translate_touch(ui_touch));
        }
    }
    return *pimpl->current_touches_vector;
}

double Gosu::Input::accelerometer_x() const
{
    return 0;
}

double Gosu::Input::accelerometer_y() const
{
    return 0;
}

double Gosu::Input::accelerometer_z() const
{
    return 0;
}

void Gosu::Input::update()
{
    // Check for dead touches and remove from vector if
    // necessary

    NSMutableSet *dead_touches = nil;

    for (UITouch *touch in pimpl->current_touches_set)
    {
        UITouchPhase phase = [touch phase];
        if (phase == UITouchPhaseBegan ||
                phase == UITouchPhaseMoved ||
                phase == UITouchPhaseStationary) {
            continue;
        }
        
        // Something was deleted, we will need the set.
        if (!dead_touches) {
            dead_touches = [NSMutableSet new];
        }
        [dead_touches addObject:touch];
    }
    
    // Has something been deleted?
    if (dead_touches)
    {
        pimpl->current_touches_vector.reset();
        [pimpl->current_touches_set minusSet: dead_touches];
        
        if (on_touch_ended) {
            for (UITouch *ui_touch in dead_touches) {
                on_touch_ended(pimpl->translate_touch(ui_touch));
            }
        }
    }
}

Gosu::TextInput* Gosu::Input::text_input() const
{
    return nullptr;
}

void Gosu::Input::set_text_input(TextInput* input)
{
    throw "NYI";
}
