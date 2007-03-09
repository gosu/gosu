#include <RubyGosu/Sample.hpp>
#include <RubyGosu/Window.hpp>
#include <RubyGosu/DataWrapper.hpp>
#include <RubyGosu/Exceptions.hpp>
#include <Gosu/Audio.hpp>
#include <Gosu/IO.hpp>
#include <Gosu/Utility.hpp>
#include <Gosu/Window.hpp>

// TODO: Check for nullness
struct SampleData
{
    boost::shared_ptr<Gosu::Window> window;
    boost::shared_ptr<Gosu::Sample> sample;
};

void RubyGosu::registerSample(VALUE module)
{
    VALUE cSample = rb_define_class_under(module, "Sample", rb_cObject);
    defineDefaultNew<SampleData>(cSample);

    rb_define_method(cSample, "initialize", (RubyMethod)Sample_initialize, 2);
    rb_define_method(cSample, "play", (RubyMethod)Sample_play, -1);
    rb_define_method(cSample, "play_pan", (RubyMethod)Sample_play_pan, -1);
}

VALUE RubyGosu::Sample_initialize(VALUE self, VALUE windowV, VALUE filenameV)
{
    TRY_CPP

    Gosu::Buffer buf;
    Gosu::loadFile(buf, Gosu::widen(StringValueCStr(filenameV)));

    SampleData& data = get<SampleData>(self);
    data.window = getWindowRef(windowV);
    data.sample.reset(new Gosu::Sample(data.window->audio(), buf.frontReader()));

    return self;

    TRANSLATE_TO_RB
}

VALUE RubyGosu::Sample_play(int argc, VALUE *argv, VALUE self)
{
    TRY_CPP

    double vol = 1, speed = 1;
    switch (argc)
    {
    case 2:
        speed = NUM2DBL(argv[1]);
    case 1:
        vol = NUM2DBL(argv[0]);
    case 0:
        ;
    }

    get<SampleData>(self).sample->play(vol, speed);
    return Qnil;

    TRANSLATE_TO_RB
}

VALUE RubyGosu::Sample_play_pan(int argc, VALUE *argv, VALUE self)
{
    TRY_CPP

    double pan, vol = 1, speed = 1;
    switch (argc)
    {
    case 3:
        speed = NUM2DBL(argv[2]);
    case 2:
        vol = NUM2DBL(argv[1]);
    case 1:
        pan = NUM2DBL(argv[0]);
        break;
    default:
        rb_raise(rb_eArgError, "Invalid number of arguments");
    }

    get<SampleData>(self).sample->playPan(pan, vol, speed);
    return Qnil;

    TRANSLATE_TO_RB
}
