#include <RubyGosu/Constants.hpp>
#include <Gosu/Input.hpp>

#define DEF_KB_BTN(btn) \
    rb_define_const(btnModule, "Kb" #btn, rb_int2inum(Gosu::kb##btn))
#define DEF_MS_BTN(btn) \
    rb_define_const(btnModule, "Ms" #btn, rb_int2inum(Gosu::ms##btn))
#define DEF_GP_BTN(btn) \
    rb_define_const(btnModule, "Gp" #btn, rb_int2inum(Gosu::gp##btn))

void RubyGosu::registerConstants(VALUE module)
{
    VALUE btnModule = rb_define_module_under(module, "Button");

    DEF_KB_BTN(Escape);
    DEF_KB_BTN(F1);
    DEF_KB_BTN(F2);
    DEF_KB_BTN(F3);
    DEF_KB_BTN(F4);
    DEF_KB_BTN(F5);
    DEF_KB_BTN(F6);
    DEF_KB_BTN(F7);
    DEF_KB_BTN(F8);
    DEF_KB_BTN(F9);
    DEF_KB_BTN(F10);
    DEF_KB_BTN(F11);
    DEF_KB_BTN(F12);

    DEF_KB_BTN(0);
    DEF_KB_BTN(1);
    DEF_KB_BTN(2);
    DEF_KB_BTN(3);
    DEF_KB_BTN(4);
    DEF_KB_BTN(5);
    DEF_KB_BTN(6);
    DEF_KB_BTN(7);
    DEF_KB_BTN(8);
    DEF_KB_BTN(9);

    DEF_KB_BTN(Tab);
    DEF_KB_BTN(Return);
    DEF_KB_BTN(Space);
    DEF_KB_BTN(LeftShift);
    DEF_KB_BTN(RightShift);
    DEF_KB_BTN(LeftControl);
    DEF_KB_BTN(RightControl);
    DEF_KB_BTN(LeftAlt);
    DEF_KB_BTN(RightAlt);
    DEF_KB_BTN(LeftMeta);
    DEF_KB_BTN(RightMeta);
    DEF_KB_BTN(Backspace);

    DEF_KB_BTN(Left);
    DEF_KB_BTN(Right);
    DEF_KB_BTN(Up);
    DEF_KB_BTN(Down);

    DEF_KB_BTN(Home);
    DEF_KB_BTN(End);
    DEF_KB_BTN(Insert);
    DEF_KB_BTN(Delete);
    DEF_KB_BTN(PageUp);
    DEF_KB_BTN(PageDown);

    DEF_KB_BTN(Enter);
    DEF_KB_BTN(Numpad0);
    DEF_KB_BTN(Numpad1);
    DEF_KB_BTN(Numpad2);
    DEF_KB_BTN(Numpad3);
    DEF_KB_BTN(Numpad4);
    DEF_KB_BTN(Numpad5);
    DEF_KB_BTN(Numpad6);
    DEF_KB_BTN(Numpad7);
    DEF_KB_BTN(Numpad8);
    DEF_KB_BTN(Numpad9);
    DEF_KB_BTN(NumpadAdd);
    DEF_KB_BTN(NumpadSubtract);
    DEF_KB_BTN(NumpadMultiply);
    DEF_KB_BTN(NumpadDivide);

    DEF_MS_BTN(Left);
    DEF_MS_BTN(Right);
    DEF_MS_BTN(Middle);
    DEF_MS_BTN(WheelUp);
    DEF_MS_BTN(WheelDown);

    DEF_GP_BTN(Left);
    DEF_GP_BTN(Right);
    DEF_GP_BTN(Up);
    DEF_GP_BTN(Down);
    DEF_GP_BTN(Button0);
    DEF_GP_BTN(Button1);
    DEF_GP_BTN(Button2);
    DEF_GP_BTN(Button3);
    DEF_GP_BTN(Button4);
    DEF_GP_BTN(Button5);
    DEF_GP_BTN(Button6);
    DEF_GP_BTN(Button7);
    DEF_GP_BTN(Button8);
    DEF_GP_BTN(Button9);
    DEF_GP_BTN(Button10);
    DEF_GP_BTN(Button11);
    DEF_GP_BTN(Button12);
    DEF_GP_BTN(Button13);
    DEF_GP_BTN(Button14);
    DEF_GP_BTN(Button15);
}
