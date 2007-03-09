#include <RubyGosu/RubyGosu.hpp>
#include <RubyGosu/Constants.hpp>
#include <RubyGosu/Window.hpp>
#include <RubyGosu/Image.hpp>
#include <RubyGosu/Font.hpp>
#include <RubyGosu/Functions.hpp>
#include <RubyGosu/Color.hpp>
#include <RubyGosu/Sample.hpp>
#include <RubyGosu/Sockets.hpp>
#include <RubyGosu/Song.hpp>
#include <Gosu/AutoLink.hpp>

std::string RubyGosu::valueToString(VALUE val)
{
    VALUE rubyStr = rb_obj_as_string(val);
    return StringValueCStr(rubyStr);
}

extern "C" void Init_gosu()
{
    VALUE module = rb_define_module("Gosu");

    RubyGosu::registerConstants(module);
    RubyGosu::registerWindow(module);
    RubyGosu::registerImage(module);
    RubyGosu::registerFont(module);
    RubyGosu::registerFunctions(module);
    RubyGosu::registerColor(module);
    RubyGosu::registerSample(module);
    RubyGosu::registerSockets(module);
    RubyGosu::registerSong(module);
}
